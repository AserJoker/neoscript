#include "compiler/include/ast.h"
#include "compiler/include/astdef.h"
#include "compiler/include/compiler.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void print_ast(neo_ast ast) {
  printf("{");
  printf("\"type\":");
  switch ((enum NEO_AST_TYPE)ast->type) {
  case NEO_AST_TYPE_UNKNOWN:
    break;
  case NEO_AST_TYPE_THIS:
    printf("\"NEO_AST_TYPE_THIS\"");
    break;
  case NEO_AST_TYPE_OBJECT_DESTRUCT_FIELD:
    printf("\"NEO_AST_TYPE_OBJECT_DESTRUCT_FIELD\"");
    break;
  case NEO_AST_TYPE_ARRAY_DESTRUCT:
    printf("\"NEO_AST_TYPE_ARRAY_DESTRUCT\"");
    break;
  case NEO_AST_TYPE_OBJECT_DESTRUCT:
    printf("\"NEO_AST_TYPE_OBJECT_DESTRUCT\"");
    break;
  case NEO_AST_TYPE_OBJECT_FIELD:
    printf("\"NEO_AST_TYPE_OBJECT_FIELD\"");
    break;
  case NEO_AST_TYPE_OBJECT_FUNCTION:
    printf("\"NEO_AST_TYPE_OBJECT_FUNCTION\"");
    break;
  case NEO_AST_TYPE_OBJECT_GET:
    printf("\"NEO_AST_TYPE_OBJECT_GET\"");
    break;
  case NEO_AST_TYPE_OBJECT_SET:
    printf("\"NEO_AST_TYPE_OBJECT_SET\"");
    break;
  case NEO_AST_TYPE_OPTIONAL_MEMBER:
    printf("\"NEO_AST_TYPE_OPTIONAL_MEMBER\"");
    break;
  case NEO_AST_TYPE_OPTIONAL_CALL:
    printf("\"NEO_AST_TYPE_OPTIONAL_CALL\"");
    break;
  case NEO_AST_TYPE_TERNARY:
    printf("\"NEO_AST_TYPE_TERNARY\"");
    break;
  case NEO_AST_TYPE_TERNARY_BODY:
    printf("\"NEO_AST_TYPE_TERNARY_BODY\"");
    break;
  case NEO_AST_TYPE_ASYNC_GENERATOR_FUNCTION:
    printf("\"NEO_AST_TYPE_ASYNC_GENERATOR_FUNCTION\"");
    break;
  case NEO_AST_TYPE_GENERATOR_FUNCTION:
    printf("\"NEO_AST_TYPE_GENERATOR_FUNCTION\"");
    break;
  case NEO_AST_TYPE_ASYNC_FUNCTION:
    printf("\"NEO_AST_TYPE_ASYNC_FUNCTION\"");
    break;
  case NEO_AST_TYPE_ASYNC_LAMBDA:
    printf("\"NEO_AST_TYPE_ASYNC_LAMBDA\"");
    break;
  case NEO_AST_TYPE_BLOCK_STATEMENT:
    printf("\"NEO_AST_TYPE_BLOCK_STATEMENT\"");
    break;
  case NEO_AST_TYPE_FUNCTION_META:
    printf("\"NEO_AST_TYPE_FUNCTION_META\"");
    break;
  case NEO_AST_TYPE_ARG_DEF:
    printf("\"NEO_AST_TYPE_ARG_DEF\"");
    break;
  case NEO_AST_TYPE_LIST:
    printf("\"NEO_AST_TYPE_LIST\"");
    break;
  case NEO_AST_TYPE_NULL:
    printf("\"NEO_AST_TYPE_NULL\"");
    break;
  case NEO_AST_TYPE_BOOLEAN:
    printf("\"NEO_AST_TYPE_BOOLEAN\"");
    printf(",\"value\":%s", ast->b_data ? "true" : "false");
    break;
  case NEO_AST_TYPE_NUMBER:
    printf("\"NEO_AST_TYPE_NUMBER\"");
    printf(",\"value\":%g", ast->f_data);
    break;
  case NEO_AST_TYPE_STRING:
    printf("\"NEO_AST_TYPE_STRING\"");
    printf(",\"value\":\"%s\"", ast->s_data);
    break;
  case NEO_AST_TYPE_SYMBOL:
    printf("\"NEO_AST_TYPE_SYMBOL\"");
    printf(",\"value\":\"%s\"", ast->s_symbol);
    break;
  case NEO_AST_TYPE_REGEX:
    printf("\"NEO_AST_TYPE_REGEX\"");
    printf(",\"value\":\"%s\"", ast->s_regex);
    break;
  case NEO_AST_TYPE_TEMPLATE_STRING:
    printf("\"NEO_AST_TYPE_TEMPLATE_STRING\"");
    break;
  case NEO_AST_TYPE_FUNCTION:
    printf("\"NEO_AST_TYPE_FUNCTION\"");
    break;
  case NEO_AST_TYPE_LAMBDA:
    printf("\"NEO_AST_TYPE_LAMBDA\"");
    break;
  case NEO_AST_TYPE_CLASS:
    printf("\"NEO_AST_TYPE_CLASS\"");
    break;
  case NEO_AST_TYPE_ARRAY:
    printf("\"NEO_AST_TYPE_ARRAY\"");
    break;
  case NEO_AST_TYPE_OBJECT:
    printf("\"NEO_AST_TYPE_OBJECT\"");
    break;
  case NEO_AST_TYPE_EXPRESSION: {
    printf("\"NEO_AST_TYPE_BINARY\"");
    break;
  }
  case NEO_AST_TYPE_MEMBER:
    printf("\"NEO_AST_TYPE_MEMBER\"");
    break;
  case NEO_AST_TYPE_CALL:
    printf("\"NEO_AST_TYPE_CALL\"");
    break;
  case NEO_AST_TYPE_PROGRAM:
    printf("\"NEO_AST_TYPE_PROGRAM\"");
    break;
  case NEO_AST_TYPE_STATEMENTS:
    printf("\"NEO_AST_TYPE_STATEMENTS\"");
    break;
  case NEO_AST_TYPE_VAR:
    printf("\"NEO_AST_TYPE_VAR\"");
    break;
  case NEO_AST_TYPE_IF_STATEMENT:
    printf("\"NEO_AST_TYPE_IF_STATEMENT\"");
    break;
  case NEO_AST_TYPE_SWITCH_STATEMENT:
    printf("\"NEO_AST_TYPE_SWITCH_STATEMENT\"");
    break;
  case NEO_AST_TYPE_FOR_STATEMENT:
    printf("\"NEO_AST_TYPE_FOR_STATEMENT\"");
    break;
  case NEO_AST_TYPE_WHILE_STATEMENT:
    printf("\"NEO_AST_TYPE_WHILE_STATEMENT\"");
    break;
  case NEO_AST_TYPE_BRACKETS:
    printf("\"NEO_AST_TYPE_BRACKETS\"");
    break;
  case NEO_AST_TYPE_MAX:
    break;
  }
  if (ast->operator) {
    printf(",\"operator\":");
    char buf[8] = {0};
    strncpy(buf, ast->operator->start,
            ast->operator->end - ast->operator->start);
    printf("\"%s\"", buf);
  }
  if (ast->left) {
    printf(",\"left\":");
    print_ast(ast->left);
  }
  if (ast->right) {
    printf(",\"right\":");
    print_ast(ast->right);
  }
  printf(",\"level\":%d", ast->level);
  printf("}");
}

int main(int argc, cstring argv[]) {
  FILE *fp = fopen("./index.mjs", "r");
  char *buf = NULL;
  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  buf = malloc(size + 1);
  buf[size] = 0;
  fseek(fp, 0, SEEK_SET);
  fread(buf, size, 1, fp);
  fclose(fp);
  neo_compiler compiler = create_neo_compiler();
  neo_ast root = neo_compiler_compile(compiler, buf, "index.mjs");
  if (!root) {
    printf("%s\n", neo_compiler_get_error(compiler));
  } else {
    print_ast(root);
    printf("\n");
    free_neo_ast(root);
  }
  free(buf);
  free_neo_compiler(compiler);
  return 0;
}
