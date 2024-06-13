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
  case NEO_AST_TYPE_ARG:
    printf("\"NEO_AST_TYPE_ARG\"");
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
    printf(",\"operator\":");
    char buf[8] = {0};
    strncpy(buf, ast->operator->start,
            ast->operator->end - ast->operator->start);
    printf("\"%s\"", buf);
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
