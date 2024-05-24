#include "engine/context.h"
#include "engine/runtime.h"
#include "engine/scope.h"
#include "engine/type.h"
#include "engine/type/closure.h"
#include "engine/type/exception.h"
#include "engine/value.h"
#include "util/strings.h"
#include "vm/type.h"
#include "vm/type/array.h"
#include "vm/type/boolean.h"
#include "vm/type/int16.h"
#include "vm/type/int32.h"
#include "vm/type/int64.h"
#include "vm/type/int8.h"
#include "vm/type/object.h"
#include "vm/type/string.h"
#include "vm/type/uint16.h"
#include "vm/type/uint32.h"
#include "vm/type/uint64.h"
#include "vm/type/uint8.h"
#include "vm/vm.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
char *toJSON(neo_context ctx, neo_value value) {
  char buf[1024];
  uint32_t type = neo_value_get_type_name(value);
  switch (type) {
  case NEO_TYPE_NULL:
    return NULL;
  case NEO_VM_TYPE_INT8:
    sprintf(buf, "%d", neo_value_to_int8(ctx, value));
    return strings_clone(buf);
  case NEO_VM_TYPE_INT16:
    sprintf(buf, "%d", neo_value_to_int16(ctx, value));
    return strings_clone(buf);
  case NEO_VM_TYPE_INT32:
    sprintf(buf, "%d", neo_value_to_int32(ctx, value));
    return strings_clone(buf);
  case NEO_VM_TYPE_INT64:
    sprintf(buf, "%ld", neo_value_to_int64(ctx, value));
    return strings_clone(buf);
  case NEO_VM_TYPE_UINT8:
    sprintf(buf, "%u", neo_value_to_uint8(ctx, value));
    return strings_clone(buf);
  case NEO_VM_TYPE_UINT16:
    sprintf(buf, "%d", neo_value_to_uint16(ctx, value));
    return strings_clone(buf);
  case NEO_VM_TYPE_UINT32:
    sprintf(buf, "%d", neo_value_to_uint32(ctx, value));
    return strings_clone(buf);
  case NEO_VM_TYPE_UINT64:
    sprintf(buf, "%ld", neo_value_to_uint64(ctx, value));
    return strings_clone(buf);
  case NEO_VM_TYPE_STRING: {
    const char *raw = neo_value_to_string(ctx, value);
    char *buf = (char *)malloc(strlen(raw) + 3);
    sprintf(buf, "\"%s\"", raw);
    return buf;
  }
  case NEO_VM_TYPE_BOOLEAN:
    return strings_clone(neo_value_to_boolean(ctx, value) ? "true" : "false");
  case NEO_VM_TYPE_OBJECT: {
    char *buf = malloc(sizeof(char) + 1);
    size_t size = 1;
    buf[0] = '{';
    neo_list keys = neo_object_keys(ctx, value);
    neo_list_node node = neo_list_head(keys);
    int8_t flag = 1;
    while (node != neo_list_tail(keys)) {
      const char *key = (const char *)neo_list_node_get(node);
      if (key) {
        neo_value field = neo_object_get_field(ctx, value, key);
        char *field_json = toJSON(ctx, field);
        if (field_json) {
          char *oldbuf = buf;
          size += 16 + strlen(key) + strlen(field_json);
          buf = malloc(size);
          if (!flag) {
            sprintf(buf, "%s,\"%s\":%s", oldbuf, key, field_json);
          } else {
            sprintf(buf, "%s\"%s\":%s", oldbuf, key, field_json);
            flag = FALSE;
          }
          free(oldbuf);
        }
      }
      node = neo_list_node_next(node);
    }
    size = strlen(buf);
    buf[size] = '}';
    buf[size + 1] = 0;
    free_neo_list(keys);
    return buf;
  }
  default:
    neo_context_throw_exception(ctx, "unknown value type");
  }
  return NULL;
}

neo_value co_func(neo_context ctx, size_t argc, neo_value *argv) {
  printf("%s\n", "co_func");
  return neo_context_get_null(ctx);
}

neo_value neo_main(neo_context ctx, size_t argc, neo_value *argv) {
  printf("neo_main start\n");
  neo_value co_fn = create_neo_closure(ctx, co_func, "co_func");
  neo_context_call(ctx, co_fn, 0, NULL, __FILE__, __LINE__, 0);
  return neo_context_get_null(ctx);
}

int main(int argc, char *argv[]) {
  neo_runtime rt = create_neo_runtime();

  neo_init_int8(rt);
  neo_init_int16(rt);
  neo_init_int32(rt);
  neo_init_int64(rt);

  neo_init_uint8(rt);
  neo_init_uint16(rt);
  neo_init_uint32(rt);
  neo_init_uint64(rt);

  neo_init_boolean(rt);
  neo_init_string(rt);

  neo_init_object(rt);
  neo_init_array(rt);

  neo_vm vm = create_neo_vm(rt);
  neo_context ctx = neo_vm_get_context(vm);
  neo_value neo_main_fn = create_neo_closure(ctx, neo_main, "neo_main");
  neo_context_call(ctx, neo_main_fn, 0, NULL, __FILE__, __LINE__, 1);
  free_neo_vm(vm);
  free_neo_runtime(rt);
  return 0;
}