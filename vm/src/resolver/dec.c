#include "resolver/dec.h"
#include "engine/include/type.h"
#include "engine/include/type/exception.h"
#include "engine/include/type/number.h"
#include "engine/include/typedef.h"
#include "engine/include/value.h"
#include "vm.h"
neo_value neo_resolver_dec(neo_vm vm, neo_ast node) {
  neo_context ctx = neo_vm_get_context(vm);
  if (node->left) {
    neo_value left = neo_vm_eval(vm, node->left);
    double val = 0;
    if (neo_value_convert(left, NEO_TYPE_NUMBER, &val)) {
      neo_value result = create_neo_number(ctx, val);
      neo_value_set_number(ctx, left, val - 1);
      return result;
    }

    char message[1024];
    sprintf(message, "Invalid operator dec with type (%d)",
            neo_value_get_type_name(left));
    neo_context_throw(
        ctx, create_neo_exception(ctx, message, NULL, node->start.filename,
                                  node->start.line, node->start.column));
  }
  if (node->right) {
    neo_value right = neo_vm_eval(vm, node->right);
    double val = 0;
    if (neo_value_convert(right, NEO_TYPE_NUMBER, &val)) {
      neo_value_set_number(ctx, right, val - 1);
      return create_neo_number(ctx, val - 1);
    }

    char message[1024];
    sprintf(message, "Invalid operator dec with type (%d)",
            neo_value_get_type_name(right));
    neo_context_throw(
        ctx, create_neo_exception(ctx, message, NULL, node->start.filename,
                                  node->start.line, node->start.column));
  }

  neo_context_throw(ctx,
                    create_neo_exception(ctx, "Invalid operator dec", NULL,
                                         node->start.filename, node->start.line,
                                         node->start.column));
  return NULL;
}