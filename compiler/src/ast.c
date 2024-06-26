#include "ast.h"
#include "astdef.h"
#include "common/include/cstring.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

uint32_t neo_ast_get_type(neo_ast self) { return self->type; }
neo_ast create_neo_ast(uint32_t type, neo_token operator, neo_ast left,
                       neo_ast right) {
  neo_ast node = (neo_ast)malloc(sizeof(struct _neo_ast));
  memset(node, 0, sizeof(struct _neo_ast));
  node->type = type;
  node->left = left;
  node->right = right;
  node->operator= operator;
  return node;
}
void free_neo_ast(neo_ast ast) {
  switch (ast->type) {
  case NEO_AST_TYPE_NUMBER:
    break;
  case NEO_AST_TYPE_STRING:
  case NEO_AST_TYPE_SYMBOL:
  case NEO_AST_TYPE_REGEX:
    free(ast->s_data);
    break;
  default:
    if (ast->left) {
      free_neo_ast(ast->left);
    }
    if (ast->right) {
      free_neo_ast(ast->right);
    }
    break;
  }
  free(ast);
}

neo_ast create_neo_boolean_ast(int8_t value) {
  neo_ast node = (neo_ast)malloc(sizeof(struct _neo_ast));
  memset(node, 0, sizeof(struct _neo_ast));
  node->type = NEO_AST_TYPE_BOOLEAN;
  node->b_data = value;
  node->level = -1;
  return node;
}
neo_ast create_neo_number_ast(double value) {
  neo_ast node = (neo_ast)malloc(sizeof(struct _neo_ast));
  memset(node, 0, sizeof(struct _neo_ast));
  node->type = NEO_AST_TYPE_NUMBER;
  node->f_data = value;
  node->level = -1;
  return node;
}
neo_ast create_neo_string_ast(const cstring value) {
  neo_ast node = (neo_ast)malloc(sizeof(struct _neo_ast));
  memset(node, 0, sizeof(struct _neo_ast));
  node->type = NEO_AST_TYPE_STRING;
  node->s_data = cstring_clone(value);
  node->level = -1;
  return node;
}
neo_ast create_neo_symbol_ast(const cstring value) {
  neo_ast node = (neo_ast)malloc(sizeof(struct _neo_ast));
  memset(node, 0, sizeof(struct _neo_ast));
  node->type = NEO_AST_TYPE_SYMBOL;
  node->s_symbol = cstring_clone(value);
  node->level = -1;
  return node;
}
neo_ast create_neo_regex_ast(const cstring value) {
  neo_ast node = (neo_ast)malloc(sizeof(struct _neo_ast));
  memset(node, 0, sizeof(struct _neo_ast));
  node->type = NEO_AST_TYPE_REGEX;
  node->s_regex = cstring_clone(value);
  node->level = -1;
  return node;
}
neo_ast create_neo_null_ast() {
  neo_ast node = (neo_ast)malloc(sizeof(struct _neo_ast));
  memset(node, 0, sizeof(struct _neo_ast));
  node->type = NEO_AST_TYPE_NULL;
  node->level = -1;
  return node;
}
neo_ast create_neo_undefined_ast() {
  neo_ast node = (neo_ast)malloc(sizeof(struct _neo_ast));
  memset(node, 0, sizeof(struct _neo_ast));
  node->type = NEO_AST_TYPE_UNDEFINED;
  node->level = -1;
  return node;
}
neo_ast neo_ast_transform(neo_ast source,
                          neo_ast (*transformer)(neo_ast source)) {
  if (source->type <= 4) {
    return transformer(source);
  }
  source->left = neo_ast_transform(source->left, transformer);
  source->right = neo_ast_transform(source->right, transformer);
  return neo_ast_transform(source, transformer);
}