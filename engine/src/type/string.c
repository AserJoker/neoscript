#include "type/string.h"
#include "atom.h"
#include "common/include/strings.h"
#include "context.h"
#include "runtime.h"
#include "type.h"
#include "typedef.h"
#include "value.h"
#include <stdlib.h>
typedef struct _neo_string_impl *neo_string_impl;
struct _neo_string_impl {
  char *data;
};
static void neo_init_string(void *target, void *source, void *_) {
  neo_string_impl dst = (neo_string_impl)target;
  neo_string_impl src = (neo_string_impl)source;
  dst->data = strings_clone(src->data);
}
static void neo_dispose_string(void *target, void *_) {
  neo_string_impl dst = (neo_string_impl)target;
  free(dst->data);
}
void neo_string_init(neo_runtime runtime) {
  neo_type_hook hook = {neo_init_string,   0, neo_dispose_string, 0,
                        neo_atom_copy_val, 0};
  neo_type neo_string =
      create_neo_type(NEO_TYPE_STRING, sizeof(struct _neo_string_impl), &hook);
  neo_runtime_define_type(runtime, neo_string);
}
const char *neo_value_get_string(neo_context ctx, neo_value value) {
  CHECK_TYPE(NEO_TYPE_STRING);
  return ((neo_string_impl)neo_value_get_data(value))->data;
}
void neo_value_set_string(neo_context ctx, neo_value value, const char *val) {
  CHECK_TYPE(NEO_TYPE_STRING);
  neo_string_impl impl = (neo_string_impl)neo_value_get_data(value);
  if (impl->data != val) {
    free(impl->data);
    impl->data = strings_clone(val);
  }
}

neo_value create_neo_string(neo_context ctx, const char *val) {
  neo_type type =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_TYPE_STRING);
  struct _neo_string_impl impl = {(char *)val};
  return neo_context_create_value(ctx, type, &impl);
}