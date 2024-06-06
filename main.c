#include "engine/include/context.h"
#include "engine/include/runtime.h"
#include "engine/include/type/boolean.h"
#include "engine/include/typedef.h"
#include "engine/include/value.h"
#include "vm/include/ast/ast.h"
#include "vm/include/astdef.h"
#include "vm/include/vm.h"
#include <stdint.h>
#include <stdlib.h>
int main(int argc, char *argv[]) {
  neo_runtime rt = create_neo_runtime();
  neo_context ctx = create_neo_context(rt);
  neo_vm vm = create_neo_vm(ctx);
  neo_ast ast =
      create_neo_ast(NEO_AST_TYPE_ADD, 0, create_neo_boolean_ast(NEO_TRUE),
                     create_neo_number_ast(123));
  neo_value val = neo_vm_eval(vm, ast);
  char *buf = NULL;
  neo_value_convert(val, NEO_TYPE_STRING, &buf);
  printf("%s\n", buf);
  free(buf);
  free_neo_ast(ast);
  free_neo_vm(vm);
  free_neo_context(ctx);
  free_neo_runtime(rt);
  return 0;
}