#include "atom.h"
#include "common/include/list.h"
#include "common/include/strings.h"
#include "context.h"
#include "runtime.h"
#include "type.h"
#include "typedef.h"
#include "value.h"
#include <stdlib.h>

typedef struct _neo_exception_impl *neo_exception_impl;
struct _neo_exception_impl {
  char *message;
  neo_list stack;
  neo_atom caused;
};

static void neo_init_exception(void *target, void *source, void *_) {
  neo_exception_impl dst = (neo_exception_impl)target;
  neo_exception_impl src = (neo_exception_impl)source;
  dst->message = strings_clone(src->message);
  dst->caused = src->caused;
  dst->stack = src->stack;
}
static void neo_dispose_exception(void *target, void *_) {
  neo_exception_impl dst = (neo_exception_impl)target;
  free_neo_list(dst->stack);
  free(dst->message);
}
static char *neo_exception_to_string(void *data) {
  neo_exception_impl self = (neo_exception_impl)data;
  const char *message = self->message;
  neo_list result = create_neo_list(free);
  char line[1024];
  sprintf(line, "Error: %s", message);
  neo_list_push(result, strings_clone(line));
  neo_list stack = self->stack;
  neo_list_node node = neo_list_head(stack);
  while (node != neo_list_tail(stack)) {
    char *frame = neo_list_node_get(node);
    if (frame) {
      sprintf(line, "\tat %s", frame);
      neo_list_push(result, strings_clone(line));
    }
    node = neo_list_node_next(node);
  }
  neo_exception_impl caused = neo_atom_get(self->caused);
  if (caused) {
    char *caused_message = neo_exception_to_string(caused);
    neo_list_push(result, caused_message);
  }
  char *result_str = strings_join(result, "\n");
  free_neo_list(result);
  return result_str;
}
static int8_t neo_convert_exception(void *data, uint32_t type, void *output,
                                    void *_) {
  switch (type) {
  case NEO_TYPE_BOOLEAN:
    *(int8_t *)output = 1;
    return 1;
  case NEO_TYPE_STRING:
    *(char **)output = neo_exception_to_string(data);
    return 1;
  }
  return 0;
}

void neo_exception_init(neo_runtime runtime) {
  neo_type_hook hook = {neo_init_exception, 0, neo_dispose_exception, 0,
                        neo_atom_copy_ref,  0};
  neo_type neo_exception = create_neo_type(
      NEO_TYPE_EXCEPTION, sizeof(struct _neo_exception_impl), &hook);
  neo_runtime_define_type(runtime, neo_exception);
}

neo_value create_neo_exception(neo_context ctx, const char *message,
                               neo_value caused, const char *filename, int line,
                               int column) {

  struct _neo_exception_impl impl = {
      (char *)message, neo_context_trace(ctx, filename, line, column), NULL};
  if (caused) {
    impl.caused = neo_value_get_atom(caused);
  }
  neo_type neo_exception =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_TYPE_EXCEPTION);
  if (!neo_exception) {
    return NULL;
  }
  neo_value exception = neo_context_create_value(ctx, neo_exception, &impl);
  if (impl.caused) {
    neo_atom atom = neo_value_get_atom(exception);
    neo_atom_add_ref(impl.caused, atom);
  }

  return exception;
}

const char *neo_exception_get_message(neo_value self) {
  if (!self) {
    return "unknown error";
  }
  neo_type value_type = neo_value_get_type(self);
  if (!value_type || neo_type_get_name(value_type) != NEO_TYPE_EXCEPTION) {
    return "unknown error";
  }
  neo_exception_impl impl = (neo_exception_impl)neo_value_get_data(self);
  return impl->message;
}

neo_value neo_exception_get_caused(neo_value self, neo_context ctx) {
  if (!self) {
    return NULL;
  }
  neo_type value_type = neo_value_get_type(self);
  if (!value_type || neo_type_get_name(value_type) != NEO_TYPE_EXCEPTION) {
    return NULL;
  }
  neo_exception_impl impl = (neo_exception_impl)neo_value_get_data(self);
  if (!impl->caused) {
    return NULL;
  }
  return create_neo_value(neo_context_get_scope(ctx), impl->caused);
}
neo_list neo_exception_get_stack(neo_value self) {
  if (!self) {
    return NULL;
  }
  neo_type value_type = neo_value_get_type(self);
  if (!value_type || neo_type_get_name(value_type) != NEO_TYPE_EXCEPTION) {
    return NULL;
  }
  neo_exception_impl impl = (neo_exception_impl)neo_value_get_data(self);
  return impl->stack;
}