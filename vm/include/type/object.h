#ifndef __NEO_VM_TYPE_OBJECT__
#define __NEO_VM_TYPE_OBJECT__
#include "runtime.h"
#include "type.h"
#include "list.h"
#include <stdint.h>

typedef struct _neo_attrubite *neo_attribute;

typedef neo_value (*neo_attribute_get)(neo_context ctx, neo_value target,
                                       void *arg);

typedef int8_t (*neo_attribute_set)(neo_context ctx, neo_value target,
                                    neo_value field, void *arg);

typedef void (*neo_attribute_delete)(neo_context ctx, neo_value target,
                                     void *arg);
struct _neo_attrubite {
  neo_attribute_get get;
  void *get_arg;
  neo_attribute_set set;
  void *set_arg;
  neo_attribute_delete del;
  void *delete_arg;
  neo_atom value;
  int8_t visible;
  int8_t readonly;
  int8_t configurable;
};

void neo_init_object(neo_runtime runtime);

neo_value create_neo_object(neo_context ctx);

void neo_object_set_attribute(neo_context ctx, neo_value value,
                              const char *name, neo_attribute field);
neo_attribute neo_object_get_attribute(neo_context ctx, neo_value value,
                                       const char *name);
void neo_object_delete_attribute(neo_context ctx, neo_value value,
                                 const char *name);

int8_t neo_object_set_field(neo_context ctx, neo_value value, const char *name,
                            neo_value field);
neo_value neo_object_get_field(neo_context ctx, neo_value value,
                               const char *name);

int8_t neo_object_delete_field(neo_context ctx, neo_value value,
                               const char *name);

neo_list neo_object_keys(neo_context ctx, neo_value value);

#endif