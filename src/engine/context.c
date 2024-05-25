#include "engine/context.h"
#include "engine/atom.h"
#include "engine/runtime.h"
#include "engine/scope.h"
#include "engine/type.h"
#include "engine/type/closure.h"
#include "engine/type/exception.h"
#include "engine/type/null.h"
#include "engine/type/promise.h"
#include "engine/value.h"
#include "util/list.h"
#include "util/strings.h"
#include <assert.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct _neo_cpuinfo *neo_cpuinfo;
struct _neo_cpuinfo {
  uint64_t eax; // 0
  uint64_t ebx; // 8
  uint64_t ecx; // 16
  uint64_t edx; // 24
  uint64_t edi; // 32
  uint64_t esi; // 40

  uint64_t esp; // 48
  uint64_t ebp; // 56
  uint64_t eip; // 64
};

static void neo_co_switch(neo_cpuinfo src, neo_cpuinfo dst) {
  asm volatile("movq %%rax,0(%%rdi)\n\t"
               "movq %%rbx,8(%%rdi)\n\t"
               "movq %%rcx,16(%%rdi)\n\t"
               "movq %%rdx,24(%%rdi)\n\t"
               "movq %%rdi,32(%%rdi)\n\t"
               "movq %%rsi,40(%%rdi)\n\t"
               "movq %%rbp,%%rbx\n\t"
               "add $16,%%rbx\n\t"
               "movq %%rbx,48(%%rdi)\n\t"
               "movq 0(%%rbp),%%rbx\n\t"
               "movq %%rbx,56(%%rdi)\n\t"
               "movq 8(%%rbp),%%rbx\n\t"
               "movq %%rbx,64(%%rdi)\n\t"

               "movq 0(%%rsi),%%rax\n\t"
               "movq 16(%%rsi),%%rcx\n\t"
               "movq 24(%%rsi),%%rdx\n\t"
               "movq 48(%%rsi),%%rsp\n\t"
               "movq 56(%%rsi),%%rbp\n\t"
               "movq 64(%%rsi),%%rbx\n\t"
               "pushq %%rbx\n\t"
               "movq 8(%%rsi),%%rbx\n\t"
               "movq 32(%%rsi),%%rdi\n\t"
               "movq 40(%%rsi),%%rsi\n\t"
               "ret\n\t" ::);
}

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
  strcpy(buf, funcname);
  strcat(buf, "(");
  strcat(buf, filename);
  strcat(buf, ":");
  char *line = strings_from_int(frame->line);
  strcat(buf, line);
  free(line);
  strcat(buf, ":");
  char *column = strings_from_int(frame->column);
  strcat(buf, column);
  free(column);
  return buf;
}

typedef struct _neo_coroutine *neo_coroutine;
struct _neo_coroutine {
  struct _neo_cpuinfo cpuinfo;
  void *pstack;
  int8_t disposed;
  neo_value func;
  size_t argc;
  neo_value *argv;

  neo_scope scope;
  neo_value closure;
  neo_call_frame callstacks;
  neo_list trystacks;
  neo_atom promise;

  neo_coroutine next;
  neo_coroutine last;
};

neo_coroutine create_neo_coroutine() {
  neo_coroutine coroutine =
      (neo_coroutine)malloc(sizeof(struct _neo_coroutine));
  coroutine->scope = create_neo_scope(NULL);
  coroutine->closure = NULL;
  coroutine->callstacks = create_neo_call_frame(NULL);
  coroutine->trystacks = create_neo_list((neo_free_fn)free_neo_try_block);
  coroutine->disposed = 0;
  coroutine->argc = 0;
  coroutine->argv = NULL;
  coroutine->next = NULL;
  coroutine->last = NULL;
  coroutine->pstack = NULL;
  return coroutine;
}
void free_neo_coroutine(neo_coroutine routine) {
  neo_coroutine last = routine->last;
  neo_coroutine next = routine->next;
  if (last) {
    last->next = next;
  }
  if (next) {
    next->last = last;
  }
  if (routine->pstack) {
    free(routine->pstack);
  }
  free_neo_list(routine->trystacks);
  free_neo_call_frame(routine->callstacks);
  while (routine->scope) {
    neo_scope parent = neo_scope_get_parent(routine->scope);
    free_neo_scope(routine->scope);
    routine->scope = parent;
  }
  free(routine);
}

struct _neo_context {
  neo_runtime rt;

  neo_coroutine coroutine;

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
  ctx->coroutine = create_neo_coroutine();
  ctx->coroutine->next = ctx->coroutine;
  ctx->coroutine->last = ctx->coroutine;
  ctx->null = create_neo_null(ctx);
  return ctx;
}
void free_neo_context(neo_context ctx) {
  if (!ctx) {
    return;
  }
  while (ctx->coroutine->next != ctx->coroutine) {
    free_neo_coroutine(ctx->coroutine->next);
  }
  free_neo_coroutine(ctx->coroutine);
  free(ctx);
}
void neo_context_push_call_frame(neo_context self, const char *funcname,
                                 const char *filename, int line, int column) {
  neo_call_frame frame = create_neo_call_frame(funcname);
  if (self->coroutine->callstacks) {
    self->coroutine->callstacks->column = column;
    self->coroutine->callstacks->line = line;
    self->coroutine->callstacks->filename = strings_clone(filename);
  }
  frame->parent = self->coroutine->callstacks;
  self->coroutine->callstacks = frame;
}
void neo_context_pop_call_frame(neo_context self) {
  neo_call_frame frame = self->coroutine->callstacks;
  self->coroutine->callstacks = frame->parent;
  if (frame->filename) {
    free(frame->filename);
  }
  if (frame->funcname) {
    free(frame->funcname);
  }
  free(frame);
  if (self->coroutine->callstacks) {
    if (self->coroutine->callstacks->filename) {
      free(self->coroutine->callstacks->filename);
      self->coroutine->callstacks->filename = NULL;
    }
    self->coroutine->callstacks->line = 0;
    self->coroutine->callstacks->column = 0;
  }
}
neo_runtime neo_context_get_runtime(neo_context self) { return self->rt; }

void neo_context_push_scope(neo_context self) {
  neo_scope scope = create_neo_scope(self->coroutine->scope);
  self->coroutine->scope = scope;
}
void neo_context_pop_scope(neo_context self) {
  neo_scope parent = neo_scope_get_parent(self->coroutine->scope);
  free_neo_scope(self->coroutine->scope);
  self->coroutine->scope = parent;
}

neo_scope neo_context_get_scope(neo_context self) {
  return self->coroutine->scope;
}
neo_value neo_context_create_value(neo_context self, neo_type type,
                                   void *init) {
  neo_atom atom = create_neo_atom(type, init);
  neo_value value = create_neo_value(self->coroutine->scope, atom);
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

  neo_value last = self->coroutine->closure;
  self->coroutine->closure = closure;

  neo_context_push_scope(self);

  neo_scope func_current = neo_context_get_scope(self);
  for (int i = 0; i < argc; i++) {
    args[i] = neo_scope_clone_value(func_current, args[i]);
  }
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

  self->coroutine->closure = closure;

  neo_context_pop_call_frame(self);
  if (is_error) {
    neo_context_throw(self, result);
  }
  return result;
}
neo_value neo_context_get_null(neo_context self) { return self->null; }

neo_value neo_context_operator(neo_context self, uint32_t opt, int argc,
                               neo_value *argv) {
  neo_operator_fn operator= neo_runtime_get_operator(self->rt, opt);
  if (operator) {
    neo_value result = operator(self, opt, argc, argv);
    return result;
  }
  neo_context_throw(self, create_neo_exception(self, "unsupport operator", NULL,
                                               __FILE__, __LINE__, 1));
  return NULL;
}
neo_list neo_context_trace(neo_context self, const char *filename, int line,
                           int column) {
  neo_list trace = create_neo_list(free);
  neo_call_frame frame = self->coroutine->callstacks;
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
  return self->coroutine->closure;
}

jmp_buf *neo_context_try_start(neo_context self) {
  neo_try_block block = create_neo_try_block(self->coroutine->scope);
  neo_list_push(self->coroutine->trystacks, block);
  return &block->context;
}

void neo_context_try_end(neo_context self) {
  neo_try_block block = neo_list_pop(self->coroutine->trystacks);
  if (block) {
    free_neo_try_block(block);
  }
}

neo_value neo_context_catch(neo_context self) {
  neo_try_block block = neo_list_pop(self->coroutine->trystacks);
  neo_value error = block->error;
  free(block);
  return error;
}

void neo_context_throw(neo_context self, neo_value exception) {
  neo_try_block block = (neo_try_block)neo_list_node_get(
      neo_list_node_last(neo_list_tail(self->coroutine->trystacks)));
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
  neo_coroutine routine = ctx->coroutine;
  jmp_buf *context = neo_context_try_start(ctx);
  neo_value promise = create_neo_value(routine->scope, routine->promise);
  neo_context_push_scope(ctx);
  if (!setjmp(*context)) {
    neo_function func = neo_closure_get_function(ctx, routine->func);
    neo_value value = func(ctx, routine->argc, routine->argv);
    neo_promise_ready(promise, ctx, value);
    neo_context_try_end(ctx);
  } else {
    neo_value error = neo_context_catch(ctx);
    neo_promise_error(promise, ctx, error);
  }
  neo_context_pop_scope(ctx);
  ctx->coroutine->disposed = 1;
  neo_context_co_yield(ctx);
}
neo_value neo_context_co_start(neo_context ctx, neo_value func, size_t argc,
                               neo_value *argv) {
  neo_value promise = create_neo_promise(ctx);
  neo_coroutine coroutine = create_neo_coroutine();
  coroutine->pstack = malloc(4096);
  coroutine->argc = argc;
  coroutine->argv = argv;
  coroutine->cpuinfo.esp = (ptrdiff_t)(coroutine->pstack) + 4096;
  coroutine->cpuinfo.ebp = coroutine->cpuinfo.esp;
  coroutine->cpuinfo.eip = (ptrdiff_t)neo_co_schedule;
  coroutine->cpuinfo.edi = (ptrdiff_t)ctx;
  coroutine->func = neo_scope_clone_value(coroutine->scope, func);
  for (int i = 0; i < argc; i++) {
    coroutine->argv[i] =
        neo_scope_clone_value(coroutine->scope, coroutine->argv[i]);
  }
  coroutine->promise = neo_value_get_atom(promise);
  coroutine->next = ctx->coroutine;
  coroutine->last = ctx->coroutine->last;
  ctx->coroutine->last->next = coroutine;
  ctx->coroutine->last = coroutine;
  return promise;
}
void neo_context_co_yield(neo_context ctx) {
  neo_coroutine current = ctx->coroutine;
  ctx->coroutine = ctx->coroutine->next;
  while (ctx->coroutine->disposed) {
    neo_coroutine next = ctx->coroutine->next;
    free_neo_coroutine(ctx->coroutine);
    ctx->coroutine = next;
  }
  if (current->next != current) {
    neo_co_switch(&current->cpuinfo, &ctx->coroutine->cpuinfo);
  }
}
int8_t neo_context_co_empty(neo_context ctx) {
  return ctx->coroutine->next == ctx->coroutine;
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