#ifndef __NEO_ENGINE_CONTEXT__
#define __NEO_ENGINE_CONTEXT__
#include "engine/scope.h"
#include "engine/type.h"
#include "runtime.h"
#include "util/list.h"
#include <setjmp.h>
typedef struct _neo_context *neo_context;

typedef void (*neo_error_callback)(neo_context ctx, neo_value error, void *_);

neo_context create_neo_context(neo_runtime rt);
void free_neo_context(neo_context ctx);

neo_runtime neo_context_get_runtime(neo_context self);

void neo_context_push_scope(neo_context self);
void neo_context_pop_scope(neo_context self);
neo_scope neo_context_get_scope(neo_context self);

neo_value neo_context_get_null(neo_context self);

neo_value neo_context_call(neo_context self, neo_value closure, int argc,
                           neo_value *argv, const char *filename, int line,
                           int column);

neo_value neo_context_get_closure(neo_context self);

neo_value neo_context_create_value(neo_context self, neo_type type, void *init);

neo_value neo_context_operator(neo_context self, uint32_t opt, int argc,
                               neo_value *argv);

neo_list neo_context_trace(neo_context self, const char *filename, int line,
                           int column);

jmp_buf *neo_context_try_start(neo_context self);

void neo_context_try_end(neo_context self);

neo_value neo_context_catch(neo_context self);

void neo_context_throw(neo_context self, neo_value exception);

void neo_context_set_error_callback(neo_context self, neo_error_callback cb,
                                    void *arg);

neo_value neo_context_co_start(neo_context ctx, neo_value func, size_t argc,
                               neo_value *argv);
void neo_context_co_yield(neo_context ctx);
neo_value neo_context_co_wait(neo_context ctx, neo_value promise);
int8_t neo_context_co_empty(neo_context ctx);
#endif