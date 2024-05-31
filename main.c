#include "ast/ast.h"
#include "astdef.h"
#include "context.h"
#include "resolver/float.h"
#include "resolver/integer.h"
#include "resolver/string.h"
#include "resolver/symbol.h"
#include "runtime.h"
#include "scope.h"
#include "type.h"
#include "type/array.h"
#include "type/boolean.h"
#include "type/custom.h"
#include "type/float.h"
#include "type/integer.h"
#include "type/object.h"
#include "type/string.h"
#include "value.h"
#include "vm.h"
#include <stdint.h>
int main(int argc, char *argv[]) {
  neo_runtime rt = create_neo_runtime();

  neo_init_boolean(rt);
  neo_init_string(rt);
  neo_init_integer(rt);
  neo_init_float(rt);
  neo_init_object(rt);
  neo_init_array(rt);
  neo_init_custom(rt);

  neo_context ctx = create_neo_context(rt);
  neo_vm vm = create_neo_vm(ctx);

  neo_vm_set_resolver(vm, NEO_AST_TYPE_INTEGER, neo_resolver_integer);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_FLOAT, neo_resolver_float);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_STRING, neo_resolver_string);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_SYMBOL, neo_resolver_symbol);

  neo_ast ast = create_neo_ast(NEO_AST_TYPE_ADD, 0, create_neo_integer_ast(2),
                               create_neo_integer_ast(4));
  neo_value value = neo_vm_eval(vm, ast);
  free_neo_ast(ast);
  free_neo_vm(vm);
  free_neo_context(ctx);
  free_neo_runtime(rt);
  return 0;
}