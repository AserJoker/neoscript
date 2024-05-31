#include "ast/ast.h"
#include "astdef.h"
#include "strings.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

uint32_t neo_ast_get_type(neo_ast self) { return self->type; }
neo_ast create_neo_ast(uint32_t type, uint32_t data, neo_ast left,
                       neo_ast right) {
  neo_ast node = (neo_ast)malloc(sizeof(struct _neo_ast));
  memset(node, 0, sizeof(struct _neo_ast));
  node->type = type;
  node->left = left;
  node->right = right;
  node->data = data;
  return node;
}
void free_neo_ast(neo_ast ast) {
  switch (ast->type) {
  case NEO_AST_TYPE_INTEGER:
  case NEO_AST_TYPE_FLOAT:
  case NEO_AST_TYPE_STRING:
  case NEO_AST_TYPE_SYMBOL:
    break;
  default:
    if (ast->left) {
      free(ast->left);
    }
    if (ast->right) {
      free(ast->right);
    }
    break;
  }
  free(ast);
}
neo_ast create_neo_integer_ast(int64_t value) {
  neo_ast node = (neo_ast)malloc(sizeof(struct _neo_ast));
  memset(node, 0, sizeof(struct _neo_ast));
  node->type = NEO_AST_TYPE_INTEGER;
  node->i_data = value;
  return node;
}
neo_ast create_neo_float_ast(double value) {
  neo_ast node = (neo_ast)malloc(sizeof(struct _neo_ast));
  memset(node, 0, sizeof(struct _neo_ast));
  node->type = NEO_AST_TYPE_FLOAT;
  node->f_data = value;
  return node;
}
neo_ast create_neo_string_ast(const char *value) {
  neo_ast node = (neo_ast)malloc(sizeof(struct _neo_ast));
  memset(node, 0, sizeof(struct _neo_ast));
  node->type = NEO_AST_TYPE_STRING;
  node->s_data = value;
  return node;
}
neo_ast create_neo_symbol_ast(const char *value) {
  neo_ast node = (neo_ast)malloc(sizeof(struct _neo_ast));
  memset(node, 0, sizeof(struct _neo_ast));
  node->type = NEO_AST_TYPE_SYMBOL;
  node->s_data = strings_clone(value);
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