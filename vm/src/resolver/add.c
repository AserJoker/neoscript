#include "resolver/add.h"
#include "context.h"
#include "tostring.h"
#include "type.h"
#include "type/boolean.h"
#include "type/float.h"
#include "type/integer.h"
#include "type/object.h"
#include "type/string.h"
#include "typedef.h"
#include "value.h"
#include "vm.h"
#include <stdlib.h>
#include <string.h>
neo_value neo_resolver_add(neo_vm vm, neo_ast node) {
  neo_value left = NULL;
  neo_value right = NULL;
  neo_context ctx = neo_vm_get_context(vm);
  if (node->left) {
    left = neo_vm_eval(vm, node->left);
  }
  right = neo_vm_eval(vm, node->right);
  if (!left) {
    return right;
  }
  neo_type left_type = neo_value_get_type(left);
  neo_type right_type = neo_value_get_type(right);
  uint32_t left_type_name = neo_type_get_name(left_type);
  uint32_t right_type_name = neo_type_get_name(right_type);

  if (left_type_name >= NEO_VM_TYPE_BOOLEAN &&
      left_type_name <= NEO_VM_TYPE_FLOAT &&
      right_type_name >= NEO_VM_TYPE_BOOLEAN &&
      right_type_name <= NEO_VM_TYPE_FLOAT) {
    if (left_type_name <= NEO_VM_TYPE_INTEGER &&
        right_type_name <= NEO_VM_TYPE_INTEGER) {
      int64_t left_value = 0;
      int64_t right_value = 0;
      if (left_type_name == NEO_VM_TYPE_INTEGER) {
        left_value = neo_value_to_integer(ctx, left);
      } else if (left_type_name == NEO_VM_TYPE_BOOLEAN) {
        left_value = neo_value_to_boolean(ctx, left);
      }
      if (right_type_name == NEO_VM_TYPE_INTEGER) {
        right_value = neo_value_to_integer(ctx, right);
      } else if (right_type_name == NEO_VM_TYPE_BOOLEAN) {
        right_value = neo_value_to_float(ctx, right);
      }
      return create_neo_integer(ctx, left_value + right_value);
    } else {
      double left_value = 0;
      double right_value = 0;
      if (left_type_name == NEO_VM_TYPE_INTEGER) {
        left_value = neo_value_to_integer(ctx, left);
      } else if (left_type_name == NEO_VM_TYPE_BOOLEAN) {
        left_value = neo_value_to_boolean(ctx, left);
      } else {
        left_value = neo_value_to_float(ctx, left);
      }
      if (right_type_name == NEO_VM_TYPE_INTEGER) {
        right_value = neo_value_to_integer(ctx, right);
      } else if (right_type_name == NEO_VM_TYPE_BOOLEAN) {
        right_value = neo_value_to_float(ctx, right);
      } else {
        right_value = neo_value_to_float(ctx, right);
      }
      return create_neo_float(ctx, left_value + right_value);
    }
  } else {
    if (left_type_name == NEO_VM_TYPE_OBJECT) {
      neo_value meta = neo_object_get_field(ctx, left, "__meta__");
      neo_value result = NULL;
      if (meta && neo_value_get_type_name(meta) == NEO_VM_TYPE_OBJECT) {
        neo_value add = neo_object_get_field(ctx, meta, "__add__");
        if (add && neo_value_get_type_name(add) == NEO_TYPE_FUNCTION) {
          neo_value args[] = {left, right};
          result = neo_context_call(ctx, add, 2, args, NULL, 0, 0);
        }
        free_neo_value(add);
      }
      free_neo_value(meta);
      if (result) {
        return result;
      }
    }
  }
  char *left_str = neo_to_string(ctx, left);
  char *right_str = neo_to_string(ctx, right);
  char *buf = malloc(strlen(left_str) + strlen(right_str) + 1);
  sprintf(buf, "%s%s", left_str, right_str);
  free(left_str);
  free(right_str);
  neo_value result = create_neo_string(ctx, buf);
  free(buf);
  return result;
}