#include "context.h"
#include "runtime.h"
#include "scope.h"
#include "strings.h"
#include "type.h"
#include "type/array.h"
#include "type/boolean.h"
#include "type/closure.h"
#include "type/custom.h"
#include "type/exception.h"
#include "type/int16.h"
#include "type/int32.h"
#include "type/int64.h"
#include "type/int8.h"
#include "type/object.h"
#include "type/string.h"
#include "type/uint16.h"
#include "type/uint32.h"
#include "type/uint64.h"
#include "type/uint8.h"
#include "typedef.h"
#include "value.h"
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
            flag = NEO_FALSE;
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
  neo_context_co_yield(ctx);
  neo_context_throw_exception(ctx, "demo error");
  return create_neo_int32(ctx, 123);
}

neo_value neo_main(neo_context ctx, size_t argc, neo_value *argv) {
  printf("neo_main start\n");
  neo_value co_fn = create_neo_closure(ctx, co_func, "co_func");
  neo_value promise = neo_context_co_start(ctx, co_fn, 0, NULL);
  while (!neo_context_co_empty(ctx)) {
    printf("neo_main\n");
    neo_context_co_yield(ctx);
  }
  neo_value result = neo_context_co_wait(ctx, promise);
  printf("%d\n", neo_value_to_int32(ctx, result));
  return neo_context_get_null(ctx);
}

int main(int argc, char *argv[]) {
  uint8_t *bytecodes = NULL;

  FILE *fp = fopen("./test.bin", "rb");
  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  bytecodes = malloc(size);
  fseek(fp, 0, SEEK_SET);
  fread(bytecodes, size, 1, fp);
  fclose(fp);
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

  neo_init_custom(rt);

  free_neo_runtime(rt);
  free(bytecodes);
  return 0;
}