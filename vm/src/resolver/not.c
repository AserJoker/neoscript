#include "resolver/not.h"
#include "engine/include/type/exception.h"
#include "engine/include/type/number.h"
#include "engine/include/typedef.h"
#include "engine/include/value.h"
neo_value neo_resolver_not(neo_vm vm, neo_ast node) {

  neo_context ctx = neo_vm_get_context(vm);
  if (!node->right) {
    neo_context_throw(ctx, create_neo_exception(ctx, "Invalid operator not",
                                                NULL, node->start.filename,
                                                node->start.line,
                                                node->start.column));
  }
  neo_value right = NULL;
  if (node->right) {
    right = neo_vm_eval(vm, node->right);
  }
  double right_val = 0;
  if (neo_value_convert(right, NEO_TYPE_NUMBER, &right_val)) {
    int64_t right_i = right_val;
    return create_neo_number(ctx, ~right_i);
  }
  char message[1024];
  sprintf(message, "Invalid operator and with type ( %d)",
          neo_value_get_type_name(right));
  neo_context_throw(
      ctx, create_neo_exception(ctx, message, NULL, node->start.filename,
                                node->start.line, node->start.column));
  return NULL;
}