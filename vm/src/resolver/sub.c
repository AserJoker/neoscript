#include "resolver/sub.h"
#include "engine/include/type.h"
#include "engine/include/type/exception.h"
#include "engine/include/type/number.h"
#include "engine/include/typedef.h"
#include "engine/include/value.h"
#include "vm.h"
neo_value neo_resolver_sub(neo_vm vm, neo_ast node) {
  neo_context ctx = neo_vm_get_context(vm);
  neo_value left = NULL;
  if (!node->right) {
    neo_context_throw(
        ctx, create_neo_exception(ctx, "Invalid operator sub", NULL,
                                  node->start.filename, node->start.line,
                                  node->start.column));
  }
  if (node->left) {
    left = neo_vm_eval(vm, node->left);
  }
  neo_value right = neo_vm_eval(vm, node->right);
  if (!left) {
    double val = 0;
    if (neo_value_convert(right, NEO_TYPE_NUMBER, &val)) {
      return create_neo_number(ctx, -val);
    }

    char message[1024];
    sprintf(message, "Invalid operator sub with type (%d)",
            neo_value_get_type_name(right));
    neo_context_throw(
        ctx, create_neo_exception(ctx, message, NULL, node->start.filename,
                                  node->start.line, node->start.column));
  }
  double left_val = 0;
  double right_val = 0;
  if (neo_value_convert(left, NEO_TYPE_NUMBER, &left_val) &&
      neo_value_convert(right, NEO_TYPE_NUMBER, &right_val)) {
    return create_neo_number(ctx, left_val - right_val);
  }
  char message[1024];
  sprintf(message, "Invalid operator subtract with type (%d and %d)",
          neo_value_get_type_name(left), neo_value_get_type_name(right));
  neo_context_throw(
      ctx, create_neo_exception(ctx, message, NULL, node->start.filename,
                                node->start.line, node->start.column));
  return NULL;
}