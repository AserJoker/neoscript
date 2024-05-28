#include "type/object.h"
#include "atom.h"
#include "context.h"
#include "list.h"
#include "map.h"
#include "runtime.h"
#include "strings.h"
#include "type/boolean.h"
#include "type/exception.h"
#include "typedef.h"
#include "value.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

neo_attribute create_neo_attribute() {
  neo_attribute attr = (neo_attribute)malloc(sizeof(struct _neo_attrubite));
  memset(attr, 0, sizeof(struct _neo_attrubite));
  attr->visible = NEO_TRUE;
  attr->readonly = NEO_TRUE;
  attr->configurable = NEO_TRUE;
  return attr;
}
void free_neo_attribute(neo_attribute attr) { free(attr); }

typedef struct _neo_object_impl {
  neo_map attributes;
} *neo_object_impl;

int8_t neo_key_cmp(char *key1, char *key2) { return strcmp(key1, key2) == 0; }

void neo_object_init(void *target, void *_, void *__) {
  neo_object_impl dst = (neo_object_impl)target;
  dst->attributes = create_neo_map((neo_compare_fn)neo_key_cmp, free,
                                   (neo_free_fn)free_neo_attribute);
}

void neo_object_dispose(void *target, void *_) {
  neo_object_impl dst = (neo_object_impl)target;
  free_neo_map(dst->attributes);
}

void neo_init_object(neo_runtime runtime) {
  neo_type_hook hook = {neo_object_init,   0, neo_object_dispose, 0,
                        neo_atom_copy_ref, 0};
  neo_type neo_object = create_neo_type(NEO_VM_TYPE_OBJECT,
                                        sizeof(struct _neo_object_impl), &hook);
  neo_runtime_define_type(runtime, neo_object);
}

neo_value create_neo_object(neo_context ctx) {
  neo_type neo_object =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_VM_TYPE_OBJECT);
  if (!neo_object) {

    neo_context_throw(ctx,
                      create_neo_exception(ctx, "unsupport value type string",
                                           NULL, __FILE__, __LINE__, 1));
  }
  return neo_context_create_value(ctx, neo_object, NULL);
}

void neo_object_set_attribute(neo_context ctx, neo_value value,
                              const char *name, neo_attribute field) {
  CHECK_TYPE(NEO_VM_TYPE_OBJECT);
  neo_object_impl obj = (neo_object_impl)neo_value_get_data(value);
  neo_attribute attr = neo_object_get_attribute(ctx, value, name);
  if (!attr) {
    attr = create_neo_attribute();
    neo_map_set(obj->attributes, strings_clone(name), attr);
  } else if (attr->configurable == NEO_TRUE) {
    char msg[1024];
    sprintf(msg, "cannot reconfigure attribute %s", name);
    neo_context_throw(
        ctx, create_neo_exception(ctx, msg, NULL, __FILE__, __LINE__, 1));
  }
  attr->del = field->del;
  attr->delete_arg = field->delete_arg;
  attr->get = field->get;
  attr->get_arg = field->get_arg;
  attr->set = field->set;
  attr->set_arg = field->set_arg;
  attr->readonly = field->readonly;
  attr->visible = field->visible;
  attr->configurable = field->configurable;
  if (attr->value != field->value) {
    neo_atom atom = neo_value_get_atom(value);
    if (field->value) {
      neo_atom_add_ref(field->value, atom);
    }
    if (attr->value) {
      neo_atom_remove_ref(attr->value, atom);
    }
    attr->value = field->value;
  }
}
neo_attribute neo_object_get_attribute(neo_context ctx, neo_value value,
                                       const char *name) {
  CHECK_TYPE(NEO_VM_TYPE_OBJECT);
  neo_object_impl obj = (neo_object_impl)neo_value_get_data(value);
  neo_attribute attr =
      (neo_attribute)neo_map_get(obj->attributes, (char *)name);
  return attr;
}
void neo_object_delete_attribute(neo_context ctx, neo_value value,
                                 const char *name) {
  CHECK_TYPE(NEO_VM_TYPE_OBJECT);
  neo_object_impl obj = (neo_object_impl)neo_value_get_data(value);
  neo_attribute attr = neo_map_get(obj->attributes, (char *)name);
  if (attr) {
    if (attr->value) {
      neo_atom atom = neo_value_get_atom(value);
      neo_atom_remove_ref(attr->value, atom);
    }
    neo_map_delete(obj->attributes, (char *)name);
  }
}

int8_t neo_object_set_field(neo_context ctx, neo_value value, const char *name,
                            neo_value field) {
  neo_attribute attr = neo_object_get_attribute(ctx, value, name);
  if (!attr) {
    struct _neo_attrubite attr_impl = {};
    memset(&attr_impl, 0, sizeof(attr_impl));
    attr_impl.configurable = NEO_TRUE;
    attr_impl.readonly = NEO_TRUE;
    attr_impl.visible = NEO_TRUE;
    neo_object_set_attribute(ctx, value, name, &attr_impl);
    attr = neo_object_get_attribute(ctx, value, name);
  }
  if (attr->readonly) {
    return 0;
  }
  if (attr->set) {
    return attr->set(ctx, value, field, attr->set_arg);
  } else {
    neo_atom field_atom = neo_value_get_atom(field);
    neo_atom atom = neo_value_get_atom(value);
    if (attr->value != field_atom) {
      if (attr->value) {
        neo_atom_remove_ref(attr->value, atom);
      }
      neo_atom_add_ref(field_atom, atom);
      attr->value = field_atom;
    }
  }
  return 1;
}
neo_value neo_object_get_field(neo_context ctx, neo_value value,
                               const char *name) {
  neo_attribute attr = neo_object_get_attribute(ctx, value, name);
  if (!attr) {
    return neo_context_get_null(ctx);
  }
  if (attr->get) {
    return attr->get(ctx, value, attr->get_arg);
  }
  if (attr->value) {
    return create_neo_value(neo_context_get_scope(ctx), attr->value);
  }
  return neo_context_get_null(ctx);
}
int8_t neo_object_delete_field(neo_context ctx, neo_value value,
                               const char *name) {
  neo_attribute attr = neo_object_get_attribute(ctx, value, name);
  if (!attr) {
    return 0;
  }
  if (attr->readonly) {
    return 0;
  }
  if (attr->del) {
    attr->del(ctx, value, attr->delete_arg);
    neo_object_delete_attribute(ctx, value, name);
    return 1;
  } else if (attr->value) {
    neo_atom atom = neo_value_get_atom(value);
    neo_atom_remove_ref(attr->value, atom);
    neo_object_delete_attribute(ctx, value, name);
    return 1;
  }
  return 0;
}

neo_list neo_object_keys(neo_context ctx, neo_value value) {
  CHECK_TYPE(NEO_VM_TYPE_OBJECT);
  neo_object_impl obj = (neo_object_impl)neo_value_get_data(value);
  neo_list keys = create_neo_list(NULL);
  neo_list ownkeys = neo_map_keys(obj->attributes);
  neo_list_node node = neo_list_head(ownkeys);
  while (node != neo_list_tail(ownkeys)) {
    char *key = (char *)neo_list_node_get(node);
    if (key) {
      neo_attribute attr = neo_map_get(obj->attributes, key);
      if (attr->visible) {
        neo_list_push(keys, key);
      }
    }
    node = neo_list_node_next(node);
  }
  return keys;
}