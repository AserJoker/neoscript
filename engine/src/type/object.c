#include "type/object.h"
#include "atom.h"
#include "common/include/list.h"
#include "common/include/map.h"
#include "common/include/strings.h"
#include "context.h"
#include "runtime.h"
#include "type.h"
#include "type/boolean.h"
#include "type/string.h"
#include "typedef.h"
#include "value.h"
#include <stdlib.h>
typedef struct _neo_attribute_impl {
  int8_t readonly;
  int8_t configurable;
  int8_t enumable;
  neo_atom getter;
  neo_atom setter;
  neo_atom value;
} *neo_attribute_impl;
typedef struct _neo_object_impl {
  neo_map properties;
} *neo_object_impl;
static void neo_init_object(void *target, void *source, void *_) {
  neo_object_impl dst = (neo_object_impl)target;
  dst->properties = create_neo_map((neo_compare_fn)strings_compare, free, free);
}
static void neo_dispose_object(void *target, void *_) {
  neo_object_impl dst = (neo_object_impl)target;
  free_neo_map(dst->properties);
}
static int8_t neo_convert_object(void *data, uint32_t type, void *output,
                                 void *_) {
  switch (type) {
  case NEO_TYPE_BOOLEAN:
    *(int8_t *)output = 1;
    return 1;
  case NEO_TYPE_STRING: {
    char buf[128];
    sprintf(buf, "[Object 0x%lx]", (ptrdiff_t)data);
    *(char **)output = strings_clone(buf);
    return 1;
  }
  }
  return 0;
}

void neo_object_init(neo_runtime rt) {
  neo_type_hook hook = {neo_init_object,   0, neo_dispose_object, 0,
                        neo_atom_copy_ref, 0, neo_convert_object, 0};
  neo_type neo_object =
      create_neo_type(NEO_TYPE_OBJECT, sizeof(struct _neo_object_impl), &hook);
  neo_runtime_define_type(rt, neo_object);
}

neo_value create_neo_object(neo_context ctx) {
  neo_type neo_object =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_TYPE_OBJECT);
  return neo_context_create_value(ctx, neo_object, NULL);
}

neo_list neo_object_get_keys(neo_context ctx, neo_value value) {
  CHECK_TYPE(NEO_TYPE_OBJECT);
  neo_object_impl impl = (neo_object_impl)neo_value_get_data(value);
  neo_list keys = neo_map_keys(impl->properties);
  neo_list result = create_neo_list(NULL);
  neo_list_node node = neo_list_head(keys);
  while (node != neo_list_tail(keys)) {
    char *key = neo_list_node_get(node);
    if (key) {
      neo_attribute_impl attr = neo_map_get(impl->properties, key);
      if (attr->enumable) {
        neo_list_push(result, key);
      }
    }
    node = neo_list_node_next(node);
  }
  return result;
}

neo_value neo_object_get_field(neo_context ctx, neo_value value,
                               const char *name) {
  CHECK_TYPE(NEO_TYPE_OBJECT);
  neo_object_impl impl = (neo_object_impl)neo_value_get_data(value);
  neo_attribute attr = neo_object_get_attribute(ctx, value, name);
  if (attr) {
    if (attr->getter) {

      neo_value args[] = {value, create_neo_string(ctx, name)};
      return neo_context_call(ctx, attr->getter, 2, args, NULL, 0, 0);
    } else if (attr->value) {
      return attr->value;
    }
  }
  return neo_context_get_null(ctx);
}

void neo_object_set_field(neo_context ctx, neo_value value, const char *name,
                          neo_value field) {
  CHECK_TYPE(NEO_TYPE_OBJECT);
  neo_object_impl impl = (neo_object_impl)neo_value_get_data(value);
  neo_attribute_impl attr_impl = neo_map_get(impl->properties, (void *)name);
  neo_atom obj_atom = neo_value_get_atom(value);
  if (!attr_impl) {
    attr_impl = malloc(sizeof(struct _neo_attribute_impl));
    attr_impl->readonly = NEO_FALSE;
    attr_impl->enumable = NEO_TRUE;
    attr_impl->configurable = NEO_TRUE;
    attr_impl->getter = NULL;
    attr_impl->setter = NULL;
    attr_impl->value = neo_value_get_atom(field);
    neo_atom_add_ref(attr_impl->value, obj_atom);
    neo_map_set(impl->properties, strings_clone(name), attr_impl);
  } else {
    if (!attr_impl->readonly) {
      return;
    }
    if (attr_impl->setter) {
      neo_value setter =
          create_neo_value(neo_context_get_scope(ctx), attr_impl->setter);
      neo_value args[] = {value, create_neo_string(ctx, name), field};
      neo_context_call(ctx, setter, 3, args, NULL, 0, 0);
    }
    neo_atom atom = neo_value_get_atom(value);
    neo_atom_add_ref(atom, obj_atom);
    if (attr_impl->value) {
      neo_atom_remove_ref(attr_impl->value, obj_atom);
      attr_impl->value = atom;
    }
  }
}

void neo_object_delete_field(neo_context ctx, neo_value value,
                             const char *name) {
  CHECK_TYPE(NEO_TYPE_OBJECT);
  neo_object_impl impl = (neo_object_impl)neo_value_get_data(value);
  neo_attribute_impl attr_impl = neo_map_get(impl->properties, (void *)name);
  if (!attr_impl) {
    return;
  }
  if (attr_impl->readonly) {
    return;
  }
  neo_atom obj_atom = neo_value_get_atom(value);
  if (attr_impl->getter) {
    neo_atom_remove_ref(attr_impl->getter, obj_atom);
  }
  if (attr_impl->setter) {
    neo_atom_remove_ref(attr_impl->setter, obj_atom);
  }
  if (attr_impl->value) {
    neo_atom_remove_ref(attr_impl->value, obj_atom);
  }
  neo_map_delete(impl->properties, (void *)name);
}

void neo_object_set_attribute(neo_context ctx, neo_value value,
                              const char *name, neo_attribute attr) {
  CHECK_TYPE(NEO_TYPE_OBJECT);
  neo_object_impl impl = (neo_object_impl)neo_value_get_data(value);
  neo_attribute_impl attr_impl = neo_map_get(impl->properties, (void *)name);
  if (!attr_impl) {
    return;
  }
  if (!attr->configurable) {
    return;
  }
  attr_impl->configurable = attr->configurable;
  attr_impl->enumable = attr->enumable;
  attr_impl->readonly = attr->readonly;
  neo_atom obj_atom = neo_value_get_atom(value);
  if (attr->getter) {
    neo_atom getter = neo_value_get_atom(attr->getter);
    if (getter != attr_impl->getter) {
      if (attr_impl->getter) {
        neo_atom_remove_ref(attr_impl->getter, obj_atom);
      }
      attr_impl->getter = getter;
      neo_atom_add_ref(attr_impl->getter, obj_atom);
    }
    if (attr->setter) {
      neo_atom setter = neo_value_get_atom(attr->setter);
      if (setter != attr_impl->setter) {
        if (attr_impl->setter) {
          neo_atom_remove_ref(attr_impl->setter, obj_atom);
        }
        attr_impl->setter = setter;
        neo_atom_add_ref(attr_impl->setter, obj_atom);
      }
    } else if (attr_impl->setter) {
      neo_atom_remove_ref(attr_impl->setter, obj_atom);
      attr_impl->setter = NULL;
    }
  } else {
    if (attr_impl->getter) {
      neo_atom_remove_ref(attr_impl->getter, obj_atom);
      attr_impl->getter = NULL;
    }
    if (attr->value) {
      neo_atom atom = neo_value_get_atom(attr->value);
      if (attr_impl->value != atom) {
        if (attr_impl->value) {
          neo_atom_remove_ref(attr_impl->value, obj_atom);
        }
        attr_impl->value = atom;
        neo_atom_add_ref(attr_impl->value, obj_atom);
      }
    } else {
      if (attr_impl->value) {
        neo_atom_remove_ref(attr_impl->value, obj_atom);
      }
      attr->value = NULL;
    }
  }
}

void neo_object_delete_attribute(neo_context ctx, neo_value value,
                                 const char *name, neo_attribute attr) {
  CHECK_TYPE(NEO_TYPE_OBJECT);
  neo_object_impl impl = (neo_object_impl)neo_value_get_data(value);
  neo_attribute_impl attr_impl = neo_map_get(impl->properties, (void *)name);
  if (!attr_impl) {
    return;
  }
  neo_atom obj_atom = neo_value_get_atom(value);
  if (attr_impl->getter) {
    neo_atom_remove_ref(attr_impl->getter, obj_atom);
  }
  if (attr_impl->setter) {
    neo_atom_remove_ref(attr_impl->setter, obj_atom);
  }
  if (attr_impl->value) {
    neo_atom_remove_ref(attr_impl->value, obj_atom);
  }
  neo_map_delete(impl->properties, (void *)name);
}

neo_attribute neo_object_get_attribute(neo_context ctx, neo_value value,
                                       const char *name) {

  CHECK_TYPE(NEO_TYPE_OBJECT);
  neo_object_impl impl = (neo_object_impl)neo_value_get_data(value);
  neo_attribute_impl attr_impl = neo_map_get(impl->properties, (void *)name);
  if (!attr_impl) {
    return NULL;
  }
  neo_attribute attr = malloc(sizeof(struct _neo_attribute));
  attr->configurable = attr_impl->configurable;
  attr->enumable = attr_impl->enumable;
  attr->readonly = attr_impl->readonly;
  if (attr_impl->getter) {
    attr->getter =
        create_neo_value(neo_context_get_scope(ctx), attr_impl->getter);
  } else {
    attr->getter = NULL;
  }
  if (attr_impl->setter) {
    attr->setter =
        create_neo_value(neo_context_get_scope(ctx), attr_impl->setter);

  } else {
    attr->setter = NULL;
  }
  if (attr_impl->value) {
    attr->value =
        create_neo_value(neo_context_get_scope(ctx), attr_impl->value);
  } else {
    attr->value = NULL;
  }
  return attr;
}