#include "resolver/call.h"
#include "compiler/include/astdef.h"
#include "engine/include/context.h"
#include "engine/include/type.h"
#include "vm.h"
#include <setjmp.h>
#include <stdlib.h>
neo_value neo_resolver_call(neo_vm vm, neo_ast node) {
  neo_value *args = NULL;
  uint32_t argc = 0;
  uint32_t index = 0;
  if (node->left->type == NEO_AST_TYPE_MEMBER) {
    argc++;
    index++;
  }
  neo_context ctx = neo_vm_get_context(vm);
  neo_ast arg_node = node->right;
  if (arg_node) {
    while (arg_node->type == NEO_AST_TYPE_COMMA) {
      argc++;
      arg_node = arg_node->right;
    }
    argc++;
  }
  args = malloc(sizeof(neo_value) * argc);
  arg_node = node->right;
  while (arg_node->type == NEO_AST_TYPE_COMMA) {
    args[index++] = neo_vm_eval(vm, arg_node->left);
    arg_node = arg_node->right;
  }
  args[index] = neo_vm_eval(vm, arg_node);
  neo_value func = NULL;
  if (node->left->type == NEO_AST_TYPE_MEMBER) {
    args[0] = neo_vm_eval(vm, node->left->left);
  }
  func = neo_vm_eval(vm, node->left);
  neo_value result = NULL;
  jmp_buf *try_block = neo_context_try_start(ctx);
  if (setjmp(*try_block)) {
    result = neo_context_call(ctx, func, argc, args, node->start.filename,
                              node->start.line, node->start.column);
    neo_context_try_end(ctx);
  } else {
    neo_value e = neo_context_catch(ctx);
    free(args);
    neo_context_throw(ctx, e);
  }
  free(args);
  return result;
}