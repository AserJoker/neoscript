#include "resolver/logic_not.h"
#include "engine/include/type/boolean.h"
#include "engine/include/type/exception.h"
#include "engine/include/typedef.h"
#include "engine/include/value.h"
neo_value neo_resolver_logic_not(neo_vm vm, neo_ast node) {
  neo_context ctx = neo_vm_get_context(vm);
  if (!node->right) {
    neo_context_throw(
        ctx, create_neo_exception(ctx, "Invalid operator logic_not", NULL,
                                  node->start.filename, node->start.line,
                                  node->start.column));
  }
  neo_value right = neo_vm_eval(vm, node->right);
  int8_t val = 0;
  if (neo_value_convert(right, NEO_TYPE_BOOLEAN, &val)) {
    return create_neo_boolean(ctx, !val);
  }

  neo_context_throw(ctx,
                    create_neo_exception(ctx, "Invalid operator logic_not",
                                         NULL, node->start.filename,
                                         node->start.line, node->start.column));
  return NULL;
}
