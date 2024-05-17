#include "engine/type/exception.h"
#include "engine/context.h"
#include "engine/runtime.h"
#include "engine/type.h"
#include "engine/value.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
typedef struct _neo_exception_impl {
  char *message;
} neo_exception_impl;

static void init(void *buf, void *body) {
  neo_exception_impl *target = (neo_exception_impl *)buf;
  neo_exception_impl *source = (neo_exception_impl *)body;
  size_t len = strlen(source->message);
  target->message = malloc(len + 1);
  strcpy(target->message, source->message);
  target->message[len] = 0;
}
static void dispose(void *buf) {
  neo_exception_impl *source = (neo_exception_impl *)buf;
  free(source->message);
}
void neo_exception_init(neo_runtime rt) {
  neo_type_hook hook = {init, dispose};
  neo_type neo_exception = create_neo_type(
      NEO_TYPE_EXCEPTION, sizeof(struct _neo_exception_impl), &hook);
  neo_runtime_define_type(rt, neo_exception);
}
neo_value create_neo_exception(neo_context ctx, const char *message) {
  neo_exception_impl exp = {(char *)message};
  neo_type type =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_TYPE_EXCEPTION);
  return neo_context_create_value(ctx, type, &exp);
}
const char *neo_exception_get_message(neo_context ctx, neo_value exception) {
  neo_exception_impl *impl =
      (neo_exception_impl *)neo_value_get_data(exception);
  return impl->message;
}