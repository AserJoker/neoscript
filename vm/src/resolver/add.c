#include "resolver/add.h"
#include "context.h"
#include "type.h"
#include "type/exception.h"
#include "type/float.h"
#include "type/integer.h"
#include "typedef.h"
#include "value.h"
#include "vm.h"
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

  if (left_type_name >= NEO_VM_TYPE_INTEGER &&
      left_type_name <= NEO_VM_TYPE_FLOAT &&
      right_type_name >= NEO_VM_TYPE_INTEGER &&
      right_type_name <= NEO_VM_TYPE_FLOAT) {
    if (left_type_name == NEO_VM_TYPE_INTEGER &&
        right_type_name == NEO_VM_TYPE_INTEGER) {
      int64_t left_value = neo_value_to_integer(ctx, left);
      int64_t right_value = neo_value_to_integer(ctx, right);
      return create_neo_integer(ctx, left_value + right_value);
    } else {
      double left_value = 0;
      double right_value = 0;
      if (left_type_name == NEO_VM_TYPE_INTEGER) {
        left_value = neo_value_to_integer(ctx, left);
      } else {
        left_value = neo_value_to_float(ctx, left);
      }
      if (right_type_name == NEO_VM_TYPE_INTEGER) {
        right_value = neo_value_to_integer(ctx, left);
      } else {
        right_value = neo_value_to_float(ctx, left);
      }
      return create_neo_float(ctx, left_value + right_value);
    }
  } else {
  }
  neo_context_throw_exception(ctx, "not implement");
  return NULL;
}