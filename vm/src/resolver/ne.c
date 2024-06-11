#include "resolver/ne.h"
#include "common/include/cstring.h"
#include "engine/include/type.h"
#include "engine/include/type/boolean.h"
#include "engine/include/type/exception.h"
#include "engine/include/type/number.h"
#include "engine/include/type/string.h"
#include "engine/include/typedef.h"
#include "engine/include/value.h"
#include "vm.h"
neo_value neo_resolver_ne(neo_vm vm, neo_ast node) {
  neo_context ctx = neo_vm_get_context(vm);
  if (!node->right || !node->left) {
    neo_context_throw(ctx, create_neo_exception(ctx, "Invalid operator ne",
                                                NULL, node->start.filename,
                                                node->start.line,
                                                node->start.column));
  }
  neo_value left = neo_vm_eval(vm, node->left);
  neo_value right = neo_vm_eval(vm, node->right);
  neo_type left_type = neo_value_get_type(left);
  neo_type right_type = neo_value_get_type(right);
  if (left_type != right_type) {
    return create_neo_boolean(ctx, NEO_TRUE);
  } else {
    enum NEO_VALUE_TYPE type_name = neo_type_get_name(left_type);
    switch (type_name) {
    case NEO_TYPE_NULL:
      return create_neo_boolean(ctx, NEO_FALSE);
    case NEO_TYPE_BOOLEAN:
      return create_neo_boolean(ctx, neo_value_get_boolean(ctx, left) !=
                                         neo_value_get_boolean(ctx, right));
    case NEO_TYPE_NUMBER:
      return create_neo_boolean(ctx, neo_value_get_number(ctx, left) !=
                                         neo_value_get_number(ctx, right));
    case NEO_TYPE_STRING:
      return create_neo_boolean(
          ctx, !cstring_compare(neo_value_get_string(ctx, left),
                                neo_value_get_string(ctx, right)));
    case NEO_TYPE_ARRAY:
    case NEO_TYPE_OBJECT:
    case NEO_TYPE_EXCEPTION:
    case NEO_TYPE_PROMISE:
    case NEO_TYPE_FUNCTION:
      return create_neo_boolean(ctx, neo_value_get_data(left) !=
                                         neo_value_get_data(right));
    default:
      break;
    }
  }

  char message[1024];
  sprintf(message, "Invalid operator ne with type (%d and %d)",
          neo_value_get_type_name(left), neo_value_get_type_name(right));
  neo_context_throw(
      ctx, create_neo_exception(ctx, message, NULL, node->start.filename,
                                node->start.line, node->start.column));
  return NULL;
}