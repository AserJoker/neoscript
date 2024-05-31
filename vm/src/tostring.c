#include "tostring.h"
#include "list.h"
#include "strings.h"
#include "type/array.h"
#include "type/boolean.h"
#include "type/closure.h"
#include "type/exception.h"
#include "type/float.h"
#include "type/integer.h"
#include "type/object.h"
#include "type/promise.h"
#include "type/string.h"
#include "typedef.h"
#include "value.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
char *neo_to_string(neo_context ctx, neo_value value) {
  uint32_t type = neo_value_get_type_name(value);
  switch (type) {
  case NEO_VM_TYPE_NULL:
    return strings_clone("null");
  case NEO_VM_TYPE_EXCEPTION: {
    neo_list result = create_neo_list(free);
    uint32_t len = 0;
    char buf[1024];
    const char *msg = neo_exception_get_message(value);
    sprintf(buf, "Error:%s\n", msg);
    neo_list_push(result, strings_clone(buf));
    neo_list trace = neo_exception_get_stack(value);
    neo_list_node node = neo_list_head(trace);
    while (node != neo_list_tail(trace)) {
      char *frame = neo_list_node_get(node);
      if (frame) {
        char *buf = malloc(strlen(frame) + 64);
        sprintf(buf, "\tat%s\n", frame);
        neo_list_push(result, buf);
        len += strlen(buf);
      }
      node = neo_list_node_next(node);
    }
    free_neo_list(trace);
    neo_value caused = neo_exception_get_caused(value, ctx);
    if (caused) {
      char *caused_str = neo_to_string(ctx, caused);
      len += strlen(caused_str);
      neo_list_push(result, caused_str);
      free_neo_value(caused);
    }
    char *res = malloc(len + 64);
    res[0] = 0;
    node = neo_list_head(result);
    while (node != neo_list_tail(result)) {
      char *line = neo_list_node_get(node);
      if (line) {
        sprintf(res, "%s%s", res, line);
      }
      node = neo_list_node_next(node);
    }
    free_neo_list(result);
    return res;
  }
  case NEO_VM_TYPE_PROMISE: {
    PromiseStatus status = neo_promise_get_status(value, ctx);
    switch (status) {
    case PROMISE_FULFILLED:
      return strings_clone("[Promise fulfilled]");
    case PROMISE_PENDDING:
      return strings_clone("[Promise pendding]");
    case PROMISE_REJECTED:
      return strings_clone("[Promise rejected]");
    }
  }
  case NEO_VM_TYPE_FUNCTION: {
    const char *name = neo_closure_get_name(ctx, value);
    char *buf = malloc(strlen(name) + 64);
    sprintf(buf, "[Function %s]", name ? name : "<no name>");
    return buf;
  }
  case NEO_VM_TYPE_BOOLEAN: {
    int8_t val = neo_value_to_boolean(ctx, value);
    return val ? strings_clone("true") : strings_clone("false");
  }
  case NEO_VM_TYPE_INTEGER: {
    char *buf = malloc(128);
    sprintf(buf, "%ld", neo_value_to_integer(ctx, value));
    return buf;
  }
  case NEO_VM_TYPE_FLOAT: {
    char *buf = malloc(128);
    sprintf(buf, "%lf", neo_value_to_float(ctx, value));
    return buf;
  }
  case NEO_VM_TYPE_STRING: {
    return strings_clone(neo_value_to_string(ctx, value));
  }
  case NEO_VM_TYPE_OBJECT: {
    char *buf = NULL;
    neo_value meta = neo_object_get_field(ctx, value, "__meta__");
    if (meta) {
      if (neo_value_get_type_name(meta) == NEO_VM_TYPE_OBJECT) {
        neo_value tostring = neo_object_get_field(ctx, meta, "__tostring__");
        if (tostring) {
          if (neo_value_get_type_name(tostring) == NEO_VM_TYPE_FUNCTION) {
            buf = neo_to_string(ctx, tostring);
          }
          free_neo_value(tostring);
        }
      }
      free_neo_value(meta);
    }
    if (!buf) {
      buf = malloc(1024);
      sprintf(buf, "[Object 0x%lx]", (ptrdiff_t)neo_value_get_data(value));
      return buf;
    }
  }
  case NEO_VM_TYPE_ARRAY: {
    neo_list result = create_neo_list(free);
    uint32_t len = 0;
    uint32_t size = neo_array_get_length(ctx, value);
    for (int i = 0; i < size; i++) {
      char *str = neo_to_string(ctx, neo_array_get_index(ctx, value, i));
      neo_list_push(result, str);
      len += strlen(str);
    }
    char *buf = malloc(len + size + 64);
    buf[0] = 0;
    sprintf(buf, "[");
    neo_list_node node = neo_list_head(result);
    while (node != neo_list_tail(result)) {
      char *part = neo_list_node_get(node);
      if (part) {
        sprintf(buf, "%s,%s", buf, part);
      }
      node = neo_list_node_next(node);
    }
    sprintf(buf, "%s]", buf);
    free_neo_list(result);
    return buf;
  }
  }
  return NULL;
}