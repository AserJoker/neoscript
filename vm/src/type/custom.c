#include "type/custom.h"
#include "atom.h"
#include "context.h"
#include "runtime.h"
#include "type.h"
#include "type/exception.h"
#include "typedef.h"
#include "value.h"
typedef struct _neo_custom *neo_custom;
struct _neo_custom {
  void *data;
  void (*autofree)(void *);
};
void neo_custom_init(void *target, void *source, void *_) {
  neo_custom dst = (neo_custom)target;
  neo_custom src = (neo_custom)source;
  dst->data = src->data;
  dst->autofree = src->autofree;
}
void neo_custom_dispose(void *target, void *_) {
  neo_custom dst = (neo_custom)target;
  if (dst->autofree) {
    dst->autofree(dst->data);
  }
}
void neo_init_custom(neo_runtime runtime) {
  neo_type_hook hook = {neo_custom_init,   0, neo_custom_dispose, 0,
                        neo_atom_copy_ref, 0};
  neo_type type =
      create_neo_type(NEO_VM_TYPE_CUSTOM, sizeof(struct _neo_custom), &hook);
  neo_runtime_define_type(runtime, type);
}
neo_value neo_create_custom(neo_context ctx, void *data,
                            void (*auto_free)(void *)) {
  neo_type type =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_VM_TYPE_CUSTOM);
  if (!type) {
    neo_context_throw_exception(ctx, "unsupport value type custom");
  }
  struct _neo_custom custom = {data, auto_free};
  return neo_context_create_value(ctx, type, &custom);
}
void *neo_value_to_custom(neo_context ctx, neo_value value) {
  CHECK_TYPE(NEO_VM_TYPE_CUSTOM);
  neo_custom custom = (neo_custom)neo_value_get_data(value);
  return custom->data;
}