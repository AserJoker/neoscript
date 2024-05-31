#include "context.h"
#include "atom.h"
#include "coroutine.h"
#include "list.h"
#include "runtime.h"
#include "scope.h"
#include "strings.h"
#include "type.h"
#include "type/closure.h"
#include "type/exception.h"
#include "type/null.h"
#include "type/promise.h"
#include "value.h"
#include <assert.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void neo_context_default_error_callback(neo_context ctx, neo_value error,
                                        void *_) {
  const char *message = neo_exception_get_message(error);
  neo_list stack = neo_exception_get_stack(error);
  neo_value caused = neo_exception_get_caused(error, ctx);
  fprintf(stderr, "Error: %s", message);
  if (stack) {
    neo_list_node node = neo_list_head(stack);
    while (node != neo_list_tail(stack)) {
      char *frame = (char *)neo_list_node_get(node);
      if (frame) {
        fprintf(stderr, "\n\tat %s", frame);
      }
      node = neo_list_node_next(node);
    }
    fprintf(stderr, "\n");
  }
  if (caused) {
    neo_context_default_error_callback(ctx, caused, NULL);
  }
  if (!stack && !caused) {
    fprintf(stderr, "\n");
  }
}

typedef struct _neo_try_block *neo_try_block;

struct _neo_try_block {
  jmp_buf context;
  neo_scope scope;
  neo_value error;
};

neo_try_block create_neo_try_block(neo_scope scope) {
  neo_try_block block = (neo_try_block)malloc(sizeof(struct _neo_try_block));
  memset(block->context, 0, sizeof(block->context));
  block->scope = scope;
  block->error = NULL;
  return block;
}

void free_neo_try_block(neo_try_block block) { free(block); }

typedef struct _neo_call_frame *neo_call_frame;
void neo_context_push_call_frame(neo_context self, const char *funcname,
                                 const char *filename, int line, int column);
struct _neo_call_frame {
  char *filename;
  char *funcname;
  int line;
  int column;
  neo_call_frame parent;
};

neo_call_frame create_neo_call_frame(const char *funcname) {
  neo_call_frame frame = (neo_call_frame)malloc(sizeof(struct _neo_call_frame));
  assert(frame != NULL);
  char *func_name = (char *)funcname;
  if (!func_name) {
    func_name = "[no name]";
  }
  frame->funcname = strings_clone(funcname);
  frame->filename = NULL;
  frame->line = 0;
  frame->column = 0;
  frame->parent = NULL;
  return frame;
}
void free_neo_call_frame(neo_call_frame frame) {
  if (!frame) {
    return;
  }
  if (frame->funcname) {
    free(frame->funcname);
  }
  if (frame->filename) {
    free(frame->filename);
  }
  free(frame);
}
char *neo_call_frame_to_string(neo_call_frame frame) {
  char *filename = frame->filename;
  char *funcname = frame->funcname;
  if (!filename) {
    filename = "<internel>";
  }
  if (!funcname) {
    funcname = "[no name]";
  }
  char *buf = (char *)malloc(strlen(filename) + strlen(funcname) + 128);
  if (frame->filename) {
    sprintf(buf, "%s(%s:%d:%d)", funcname, filename, frame->line,
            frame->column);
  } else {
    sprintf(buf, "%s(%s)", funcname, filename);
  }
  return buf;
}

typedef struct _neo_co_context *neo_co_context;
struct _neo_co_context {
  neo_coroutine coroutine;
  int8_t disposed;
  neo_value func;
  size_t argc;
  neo_value *argv;

  neo_scope scope;
  neo_value closure;
  neo_call_frame callstacks;
  neo_list trystacks;
  neo_atom promise;

  neo_co_context next;
  neo_co_context last;
};

neo_co_context create_neo_co_context(neo_scope parent) {
  neo_co_context co_context =
      (neo_co_context)malloc(sizeof(struct _neo_co_context));
  co_context->coroutine = NULL;
  co_context->scope = create_neo_scope(parent);
  co_context->closure = NULL;
  co_context->callstacks = create_neo_call_frame(NULL);
  co_context->trystacks = create_neo_list((neo_free_fn)free_neo_try_block);
  co_context->disposed = 0;
  co_context->argc = 0;
  co_context->argv = NULL;
  co_context->next = NULL;
  co_context->last = NULL;
  return co_context;
}
static void free_neo_co_context(neo_co_context routine) {
  neo_co_context last = routine->last;
  neo_co_context next = routine->next;
  if (last) {
    last->next = next;
  }
  if (next) {
    next->last = last;
  }
  if (routine->coroutine) {
    neo_co_destroy(routine->coroutine);
  }
  free_neo_list(routine->trystacks);
  free_neo_call_frame(routine->callstacks);
  while (neo_scope_get_parent(routine->scope) != NULL) {
    neo_scope parent = neo_scope_get_parent(routine->scope);
    free_neo_scope(routine->scope);
    routine->scope = parent;
  }
  free(routine);
}

struct _neo_context {
  neo_runtime rt;
  neo_scope global;
  neo_co_context co_context;

  neo_error_callback error_cb;
  void *error_cb_arg;
  neo_value null;
};

neo_context create_neo_context(neo_runtime rt) {
  neo_context ctx = (neo_context)malloc(sizeof(struct _neo_context));
  assert(ctx != NULL);
  ctx->error_cb = neo_context_default_error_callback;
  ctx->error_cb_arg = NULL;
  ctx->rt = rt;
  ctx->global = create_neo_scope(NULL);
  ctx->co_context = create_neo_co_context(ctx->global);
  ctx->co_context->next = ctx->co_context;
  ctx->co_context->last = ctx->co_context;
  ctx->null = create_neo_null(ctx);
  ctx->co_context->coroutine = neo_co_start(NULL, NULL);
  return ctx;
}
void free_neo_context(neo_context ctx) {
  if (!ctx) {
    return;
  }
  while (ctx->co_context->next != ctx->co_context) {
    free_neo_co_context(ctx->co_context->next);
  }
  free_neo_co_context(ctx->co_context);
  free_neo_scope(ctx->global);
  free(ctx);
}
void neo_context_push_call_frame(neo_context self, const char *funcname,
                                 const char *filename, int line, int column) {
  neo_call_frame frame = create_neo_call_frame(funcname);
  if (self->co_context->callstacks) {
    self->co_context->callstacks->column = column;
    self->co_context->callstacks->line = line;
    self->co_context->callstacks->filename = strings_clone(filename);
  }
  frame->parent = self->co_context->callstacks;
  self->co_context->callstacks = frame;
}
void neo_context_pop_call_frame(neo_context self) {
  neo_call_frame frame = self->co_context->callstacks;
  self->co_context->callstacks = frame->parent;
  if (frame->filename) {
    free(frame->filename);
  }
  if (frame->funcname) {
    free(frame->funcname);
  }
  free(frame);
  if (self->co_context->callstacks) {
    if (self->co_context->callstacks->filename) {
      free(self->co_context->callstacks->filename);
      self->co_context->callstacks->filename = NULL;
    }
    self->co_context->callstacks->line = 0;
    self->co_context->callstacks->column = 0;
  }
}
neo_runtime neo_context_get_runtime(neo_context self) { return self->rt; }

void neo_context_push_scope(neo_context self) {
  neo_scope scope = create_neo_scope(self->co_context->scope);
  self->co_context->scope = scope;
}
void neo_context_pop_scope(neo_context self) {
  neo_scope parent = neo_scope_get_parent(self->co_context->scope);
  free_neo_scope(self->co_context->scope);
  self->co_context->scope = parent;
}

neo_scope neo_context_get_scope(neo_context self) {
  return self->co_context->scope;
}
neo_value neo_context_create_value(neo_context self, neo_type type,
                                   void *init) {
  neo_atom atom = create_neo_atom(type, init);
  neo_value value = create_neo_value(self->co_context->scope, atom);
  return value;
}
neo_value neo_context_call(neo_context self, neo_value closure, int argc,
                           neo_value *args, const char *filename, int line,
                           int column) {
  neo_scope current = neo_context_get_scope(self);

  neo_function func = neo_closure_get_function(self, closure);
  jmp_buf *context = neo_context_try_start(self);

  neo_context_push_call_frame(self, neo_closure_get_name(self, closure),
                              filename, line, column);

  neo_value last = self->co_context->closure;
  self->co_context->closure = closure;

  neo_context_push_scope(self);

  neo_scope func_current = neo_context_get_scope(self);
  for (int i = 0; i < argc; i++) {
    args[i] = neo_scope_clone_value(func_current, args[i]);
  }
  neo_list keys = neo_closure_get_keys(self, closure);
  neo_list_node node = neo_list_head(keys);
  while (node != neo_list_tail(keys)) {
    char *name = neo_list_node_get(node);
    if (name) {
      neo_value val = neo_closure_get(self, closure, name);
      neo_scope_store_value(self->co_context->scope, name, val);
    }
    node = neo_list_node_next(node);
  }
  free_neo_list(keys);
  neo_value res = NULL;
  int is_error = 0;
  if (!setjmp(*context)) {
    res = func(self, argc, args);
    neo_context_try_end(self);
  } else {
    res = neo_context_catch(self);
    is_error = 1;
  }

  neo_value result = neo_scope_clone_value(current, res);

  neo_context_pop_scope(self);

  self->co_context->closure = closure;

  neo_context_pop_call_frame(self);
  if (is_error) {
    neo_context_throw(self, result);
  }
  return result;
}
neo_value neo_context_get_null(neo_context self) { return self->null; }


neo_list neo_context_trace(neo_context self, const char *filename, int line,
                           int column) {
  neo_list trace = create_neo_list(free);
  neo_call_frame frame = self->co_context->callstacks;
  struct _neo_call_frame tmp;
  tmp.funcname = frame->funcname;
  tmp.filename = (char *)filename;
  tmp.line = line;
  tmp.column = column;
  char *buf = neo_call_frame_to_string(&tmp);
  neo_list_push(trace, buf);
  frame = frame->parent;
  while (frame) {
    char *buf = neo_call_frame_to_string(frame);
    neo_list_push(trace, buf);
    frame = frame->parent;
  }
  return trace;
}

neo_value neo_context_get_closure(neo_context self) {
  return self->co_context->closure;
}

jmp_buf *neo_context_try_start(neo_context self) {
  neo_try_block block = create_neo_try_block(self->co_context->scope);
  neo_list_push(self->co_context->trystacks, block);
  return &block->context;
}

void neo_context_try_end(neo_context self) {
  neo_try_block block = neo_list_pop(self->co_context->trystacks);
  if (block) {
    free_neo_try_block(block);
  }
}
neo_value neo_context_catch(neo_context self) {
  neo_try_block block = neo_list_pop(self->co_context->trystacks);
  neo_value error = block->error;
  free(block);
  return error;
}

void neo_context_throw(neo_context self, neo_value exception) {
  neo_try_block block = (neo_try_block)neo_list_node_get(
      neo_list_node_last(neo_list_tail(self->co_context->trystacks)));
  if (block) {
    block->error = neo_scope_clone_value(block->scope, exception);
    longjmp(block->context, 1);
  } else if (self->error_cb) {
    self->error_cb(self, exception, self->error_cb_arg);
  }
}

void neo_context_set_error_callback(neo_context self, neo_error_callback cb,
                                    void *arg) {
  self->error_cb = cb;
  self->error_cb_arg = arg;
}
static void neo_co_schedule(neo_context ctx) {
  neo_co_context routine = ctx->co_context;
  jmp_buf *context = neo_context_try_start(ctx);
  neo_value promise = create_neo_value(routine->scope, routine->promise);
  neo_context_push_scope(ctx);
  neo_context_push_call_frame(ctx, "<coroutine>", NULL, 0, 0);
  neo_context_push_call_frame(ctx, neo_closure_get_name(ctx, routine->func),
                              NULL, 0, 0);
  neo_list keys = neo_closure_get_keys(ctx, routine->func);
  neo_list_node node = neo_list_head(keys);
  while (node != neo_list_tail(keys)) {
    char *name = neo_list_node_get(node);
    if (name) {
      neo_value val = neo_closure_get(ctx, routine->func, name);
      neo_scope_store_value(ctx->co_context->scope, name, val);
    }
    node = neo_list_node_next(node);
  }
  free_neo_list(keys);
  if (!setjmp(*context)) {
    neo_function func = neo_closure_get_function(ctx, routine->func);
    neo_value value = func(ctx, routine->argc, routine->argv);
    neo_promise_ready(promise, ctx, value);
    neo_context_try_end(ctx);
  } else {
    neo_value error = neo_context_catch(ctx);
    neo_promise_error(promise, ctx, error);
  }
  neo_context_pop_call_frame(ctx);
  neo_context_pop_call_frame(ctx);
  neo_context_pop_scope(ctx);
  ctx->co_context->disposed = 1;
  neo_context_co_yield(ctx);
}

neo_value neo_context_co_start(neo_context ctx, neo_value func, size_t argc,
                               neo_value *argv) {
  neo_value promise = create_neo_promise(ctx);
  neo_co_context coroutine = create_neo_co_context(ctx->global);
  coroutine->argc = argc;
  coroutine->argv = argv;
  coroutine->coroutine = neo_co_start((void (*)(void *))neo_co_schedule, ctx);
  coroutine->func = neo_scope_clone_value(coroutine->scope, func);
  for (int i = 0; i < argc; i++) {
    coroutine->argv[i] =
        neo_scope_clone_value(coroutine->scope, coroutine->argv[i]);
  }
  coroutine->promise = neo_value_get_atom(promise);
  coroutine->next = ctx->co_context;
  coroutine->last = ctx->co_context->last;
  ctx->co_context->last->next = coroutine;
  ctx->co_context->last = coroutine;
  return promise;
}
void neo_context_co_yield(neo_context ctx) {
  neo_co_context current = ctx->co_context;
  ctx->co_context = ctx->co_context->next;
  while (ctx->co_context->disposed) {
    neo_co_context next = ctx->co_context->next;
    free_neo_co_context(ctx->co_context);
    ctx->co_context = next;
  }
  if (current->next != current) {
    neo_co_yield(current->coroutine, ctx->co_context->coroutine);
  }
}
int8_t neo_context_co_empty(neo_context ctx) {
  return ctx->co_context->next == ctx->co_context;
}
neo_value neo_context_co_wait(neo_context ctx, neo_value promise) {
  while (neo_promise_get_status(promise, ctx) == PROMISE_PENDDING) {
    neo_context_co_yield(ctx);
  }
  if (neo_promise_get_status(promise, ctx) == PROMISE_FULFILLED) {
    return neo_promise_get_value(promise, ctx);
  }
  if (neo_promise_get_status(promise, ctx) == PROMISE_REJECTED) {
    neo_value error = neo_promise_get_value(promise, ctx);
    neo_context_throw(ctx, error);
  }
  return NULL;
}