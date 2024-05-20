#include "engine/context.h"
#include "engine/atom.h"
#include "engine/closure.h"
#include "engine/runtime.h"
#include "engine/scope.h"
#include "engine/type.h"
#include "engine/type/null.h"
#include "engine/value.h"
#include "util/list.h"
#include "util/strings.h"
#include <assert.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  char *buf = (char *)malloc(strlen(filename) + strlen(funcname) + 64);
  sprintf(buf, "%s(%s:%d:%d)", funcname, filename, frame->line, frame->column);
  return buf;
}
struct _neo_context {
  neo_runtime rt;
  neo_scope scope;
  neo_closure closure;
  neo_call_frame callstacks;
  neo_value null;
  neo_list trystacks;

  neo_error_callback error_cb;
  void *error_cb_arg;
};

neo_context create_neo_context(neo_runtime rt) {
  neo_context ctx = (neo_context)malloc(sizeof(struct _neo_context));
  assert(ctx != NULL);
  ctx->error_cb = NULL;
  ctx->error_cb_arg = NULL;
  ctx->rt = rt;
  ctx->scope = create_neo_scope(NULL);
  ctx->closure = NULL;
  ctx->callstacks = create_neo_call_frame(NULL);
  ctx->trystacks = create_neo_list((neo_free_fn)free_neo_try_block);
  ctx->null = create_neo_null(ctx);

  return ctx;
}
void free_neo_context(neo_context ctx) {
  if (!ctx) {
    return;
  }
  free_neo_list(ctx->trystacks);
  free_neo_call_frame(ctx->callstacks);
  while (ctx->scope) {
    neo_context_pop_scope(ctx);
  }
  free(ctx);
}
void neo_context_push_call_frame(neo_context self, const char *funcname,
                                 const char *filename, int line, int column) {
  neo_call_frame frame = create_neo_call_frame(funcname);
  if (self->callstacks) {
    self->callstacks->column = column;
    self->callstacks->line = line;
    self->callstacks->filename = strings_clone(filename);
  }
  frame->parent = self->callstacks;
  self->callstacks = frame;
}
void neo_context_pop_call_frame(neo_context self) {
  neo_call_frame frame = self->callstacks;
  self->callstacks = frame->parent;
  if (frame->filename) {
    free(frame->filename);
  }
  if (frame->funcname) {
    free(frame->funcname);
  }
  free(frame);
  if (self->callstacks) {
    if (self->callstacks->filename) {
      free(self->callstacks->filename);
      self->callstacks->filename = NULL;
    }
    self->callstacks->line = 0;
    self->callstacks->column = 0;
  }
}
neo_runtime neo_context_get_runtime(neo_context self) { return self->rt; }

void neo_context_push_scope(neo_context self) {
  neo_scope scope = create_neo_scope(self->scope);
  self->scope = scope;
}
void neo_context_pop_scope(neo_context self) {
  neo_scope parent = neo_scope_get_parent(self->scope);
  free_neo_scope(self->scope);
  self->scope = parent;
}

neo_scope neo_context_get_scope(neo_context self) { return self->scope; }
neo_value neo_context_create_value(neo_context self, neo_type type,
                                   void *init) {
  neo_atom atom = create_neo_atom(type, init);
  neo_value value = create_neo_value(self->scope, atom);
  return value;
}
neo_value neo_context_call(neo_context self, neo_closure closure,
                           neo_value *args, int argc, const char *filename,
                           int line, int column) {
  neo_scope current = neo_context_get_scope(self);

  neo_function func = neo_closure_get_function(closure);
  jmp_buf *context = neo_context_try_start(self);

  neo_context_push_call_frame(self, neo_closure_get_name(closure), filename,
                              line, column);

  neo_closure last = self->closure;
  self->closure = closure;

  neo_context_push_scope(self);

  neo_value *args_next = (neo_value *)malloc(sizeof(neo_value *) * argc);

  neo_scope func_current = neo_context_get_scope(self);
  for (int i = 0; i < argc; i++) {
    args_next[i] = neo_scope_clone_value(func_current, args[i]);
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
  free(args_next);

  neo_value result = neo_scope_clone_value(current, res);

  neo_context_pop_scope(self);

  self->closure = closure;

  neo_context_pop_call_frame(self);
  if (is_error) {
    neo_context_throw(self, result);
  }
  return result;
}
neo_value neo_context_get_null(neo_context self) { return self->null; }

neo_value neo_context_get_closure_value(neo_context self, int index) {
  return neo_closure_get(self, self->closure, index);
}
neo_value neo_context_operator(neo_context self, uint32_t opt, int argc,
                               neo_value *argv) {
  neo_operator_fn operator= neo_runtime_get_operator(self->rt, opt);
  if (operator) {
    return operator(self, opt, argc, argv);
  }
  return NULL;
}
neo_list neo_context_trace(neo_context self, const char *filename, int line,
                           int column) {
  neo_list trace = create_neo_list(free);
  neo_call_frame frame = self->callstacks;
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

jmp_buf *neo_context_try_start(neo_context self) {
  neo_try_block block = create_neo_try_block(self->scope);
  neo_list_push(self->trystacks, block);
  return &block->context;
}

void neo_context_try_end(neo_context self) {
  neo_try_block block = neo_list_pop(self->trystacks);
  if (block) {
    free_neo_try_block(block);
  }
}

neo_value neo_context_catch(neo_context self) {
  neo_try_block block = neo_list_pop(self->trystacks);
  neo_value error = block->error;
  free(block);
  return error;
}

void neo_context_throw(neo_context self, neo_value exception) {
  neo_try_block block = (neo_try_block)neo_list_node_get(
      neo_list_node_last(neo_list_tail(self->trystacks)));
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