#include "type/string.h"
#include "context.h"
#include "runtime.h"
#include "strings.h"
#include "type/exception.h"
#include "typedef.h"
#include "value.h"
#include <stdio.h>
#include <stdlib.h>
typedef struct _neo_string *neo_string;
struct _neo_string {
  char *str;
};
void neo_string_init(void *target, void *source, void *_) {
  neo_string dst = (neo_string)target;
  neo_string src = (neo_string)source;
  dst->str = strings_clone(src->str);
}
void neo_string_dispose(void *target, void *_) {
  neo_string dst = (neo_string)target;
  free(dst->str);
}
void neo_init_string(neo_runtime runtime) {
  neo_type_hook hook = {neo_string_init,   NULL, neo_string_dispose, NULL,
                        neo_atom_copy_val, NULL};
  neo_type neo_string =
      create_neo_type(NEO_VM_TYPE_STRING, sizeof(struct _neo_string), &hook);
  neo_runtime_define_type(runtime, neo_string);
}
neo_value create_neo_string(neo_context ctx, const char *value) {
  struct _neo_string str = {(char *)value};
  neo_type neo_string =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_VM_TYPE_STRING);
  if (!neo_string) {
    neo_context_throw(ctx,
                      create_neo_exception(ctx, "unsupport value type string",
                                           NULL, __FILE__, __LINE__, 1));
  }
  return neo_context_create_value(ctx, neo_string, &str);
}
const char *neo_value_to_string(neo_context ctx, neo_value value) {
  CHECK_TYPE(NEO_VM_TYPE_STRING);
  neo_string str = neo_value_get_data(value);
  return str->str;
}