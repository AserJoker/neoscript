#include "resolver/div.h"
#include "engine/include/type/exception.h"
#include "engine/include/type/number.h"
#include "engine/include/typedef.h"
#include "engine/include/value.h"
neo_value neo_resolver_div(neo_vm vm, neo_ast node) {

  neo_context ctx = neo_vm_get_context(vm);
  if (!node->right || !node->left) {
    neo_context_throw(ctx, create_neo_exception(ctx, "Invalid operator div",
                                                NULL, node->start.filename,
                                                node->start.line,
                                                node->start.column));
  }
  neo_value left = NULL;
  neo_value right = NULL;
  if (node->left) {
    left = neo_vm_eval(vm, node->left);
  }
  if (node->right) {
    right = neo_vm_eval(vm, node->right);
  }
  double left_val = 0;
  double right_val = 0;
  if (neo_value_convert(left, NEO_TYPE_NUMBER, &left_val) &&
      neo_value_convert(right, NEO_TYPE_NUMBER, &right_val)) {
    return create_neo_number(ctx, left_val / right_val);
  }
  char message[1024];
  sprintf(message, "Invalid operator div with type (%d and %d)",
          neo_value_get_type_name(left), neo_value_get_type_name(right));
  neo_context_throw(
      ctx, create_neo_exception(ctx, message, NULL, node->start.filename,
                                node->start.line, node->start.column));
  return NULL;
}