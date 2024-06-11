#ifndef __NEO_ENGINE_TYPE_OBJECT__
#define __NEO_ENGINE_TYPE_OBJECT__
#include "common/include/list.h"
#include "engine/include/runtime.h"
#include "engine/include/type.h"
#include "common/include/cstring.h"

typedef struct _neo_attribute {
  int8_t readonly;
  int8_t configurable;
  int8_t enumable;
  neo_value getter;
  neo_value setter;
  neo_value value;
} *neo_attribute;

void neo_object_init(neo_runtime rt);

neo_value create_neo_object(neo_context ctx);

neo_list neo_object_get_keys(neo_context ctx, neo_value value);

neo_value neo_object_get_field(neo_context ctx, neo_value value,
                               const cstring name);

void neo_object_set_field(neo_context ctx, neo_value value, const cstring name,
                          neo_value field);

void neo_object_delete_field(neo_context ctx, neo_value value,
                             const cstring name);

void neo_object_set_attribute(neo_context ctx, neo_value value,
                              const cstring name, neo_attribute attr);
void neo_object_delete_attribute(neo_context ctx, neo_value value,
                                 const cstring name, neo_attribute attr);
neo_attribute neo_object_get_attribute(neo_context ctx, neo_value value,
                                       const cstring name);

#endif