#include "operator/expr.h"
#include "context.h"
#include "operatordef.h"
#include "type.h"
#include "type/boolean.h"
#include "type/exception.h"
#include "type/int16.h"
#include "type/int32.h"
#include "type/int64.h"
#include "type/int8.h"
#include "type/uint16.h"
#include "type/uint32.h"
#include "type/uint8.h"
#include "typedef.h"
#include "value.h"
neo_type neo_opt_get_type(neo_value left, neo_value right) {
  neo_type left_type = 0;
  neo_type right_type = 0;
  if (!left) {
    left_type = neo_value_get_type(right);
  }
  if (!right) {
    right_type = neo_value_get_type(left);
  } else {
    left_type = neo_value_get_type(left);
    right_type = neo_value_get_type(right);
  }
  if (neo_type_get_name(left_type) >= 14 ||
      neo_type_get_name(right_type) >= 14) {
    return NULL;
  }
  if (left_type == 0) {
    return right_type;
  }
  if (right_type == 0) {
    return left_type;
  }
  int32_t left_name = neo_type_get_name(left_type);
  int32_t right_name = neo_type_get_name(right_type);
  return left_name >= right_name ? left_type : right_type;
}
int64_t convert_to_int64(neo_context ctx, neo_value value) {
  int32_t type = neo_value_get_type_name(value);
  switch (type) {
  case NEO_VM_TYPE_BOOLEAN:
    return neo_value_to_boolean(ctx, value);
  case NEO_VM_TYPE_INT8:
    return neo_value_to_int8(ctx, value);
  case NEO_VM_TYPE_INT16:
    return neo_value_to_int16(ctx, value);
  case NEO_VM_TYPE_INT32:
    return neo_value_to_int32(ctx, value);
  case NEO_VM_TYPE_INT64:
    return neo_value_to_int64(ctx, value);
  case NEO_VM_TYPE_UINT8:
    return neo_value_to_uint8(ctx, value);
  case NEO_VM_TYPE_UINT16:
    return neo_value_to_uint16(ctx, value);
  case NEO_VM_TYPE_UINT32:
    return neo_value_to_uint32(ctx, value);
  }
  char msg[1024];
  sprintf(msg, "cannot convert type:%d to int64", type);
  neo_context_throw_exception(ctx, msg);
  return 0;
}

neo_value convert_to_value(neo_context ctx, neo_type type, int64_t value) {
  int32_t type_name = neo_type_get_name(type);
  switch (type_name) {
  case NEO_VM_TYPE_BOOLEAN:
    return create_neo_boolean(ctx, value != 0);
  case NEO_VM_TYPE_INT8:
    return create_neo_int8(ctx, value);
  case NEO_VM_TYPE_INT16:
    return create_neo_int16(ctx, value);
  case NEO_VM_TYPE_INT32:
    return create_neo_int32(ctx, value);
  case NEO_VM_TYPE_INT64:
    return create_neo_int64(ctx, value);
  case NEO_VM_TYPE_UINT8:
    return create_neo_uint8(ctx, value);
  case NEO_VM_TYPE_UINT16:
    return create_neo_uint16(ctx, value);
  case NEO_VM_TYPE_UINT32:
    return create_neo_uint32(ctx, value);
  }
  char msg[1024];
  sprintf(msg, "cannot convert value to type:%d", type_name);
  neo_context_throw_exception(ctx, msg);
  return NULL;
}

neo_value neo_operator_expr(neo_context ctx, uint32_t opt, int argc,
                            neo_value *argv) {
  neo_value left = argv[0];
  neo_value right = argv[1];
  neo_type type = neo_opt_get_type(left, right);
  if (type) {
    if (neo_type_get_name(type) <= 8) {
      int64_t left_value = convert_to_int64(ctx, left);
      int64_t right_value = convert_to_int64(ctx, right);
      int64_t result = 0;
      switch (opt) {
      case NEO_OPERATOR_ADD:
        result = left_value + right_value;
        break;
      case NEO_OPERATOR_SUBTRACT:
        result = left_value - right_value;
        break;
      case NEO_OPERATOR_MULTIPLY:
        result = left_value * right_value;
        break;
      case NEO_OPERATOR_DIVIDE:
        result = left_value / right_value;
        break;
      case NEO_OPERATOR_MODULU:
        result = left_value % right_value;
      default:
        neo_context_throw_exception(ctx, "not implement");
      }
      return convert_to_value(ctx, type, result);
    } else {
      neo_context_throw_exception(ctx, "not implement");
    }
  }
  neo_context_throw_exception(ctx, "not implement");
  return NULL;
}