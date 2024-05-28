#include "ast.h"
#include "strings.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
struct _neo_ast {
  uint32_t type;
  void *data;
  union {
    neo_ast *children;
    char *s_data;
    int64_t i64_data;
    double f_data;
    char *s_symbol;
  };
};

neo_ast *neo_ast_get_children(neo_ast self) { return self->children; }

uint32_t neo_ast_get_type(neo_ast self) { return self->type; }
neo_ast create_neo_ast(uint32_t type, void *data, neo_ast *children) {
  neo_ast node = (neo_ast)malloc(sizeof(struct _neo_ast));
  memset(node, 0, sizeof(struct _neo_ast));
  node->type = type;
  node->children = children;
  node->data = data;
  return node;
}
void free_neo_ast(neo_ast ast) {
  switch (ast->type) {
  case NEO_AST_TYPE_INTEGER:
  case NEO_AST_TYPE_FLOAT:
  case NEO_AST_TYPE_EOF:
    break;
  case NEO_AST_TYPE_SYMBOL:
    free(ast->s_symbol);
    break;
  case NEO_AST_TYPE_STRING:
    free(ast->s_data);
    break;
  default:
    if (ast->children) {
      for (int i = 0;; i++) {
        if (ast->children[i]) {
          uint32_t type = ast->children[i]->type;
          free_neo_ast(ast->children[i]);
          if (type == NEO_AST_TYPE_EOF) {
            break;
          }
        }
      }
      free(ast->children);
    }
    break;
  }
  if (ast->data) {
    free(ast->data);
  }
  free(ast);
}
neo_ast create_neo_integer_ast(int64_t value) {
  neo_ast node = (neo_ast)malloc(sizeof(struct _neo_ast));
  memset(node, 0, sizeof(struct _neo_ast));
  node->type = NEO_AST_TYPE_INTEGER;
  node->i64_data = value;
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
  node->s_data = strings_clone(value);
  return node;
}
neo_ast create_neo_symbol_ast(const char *value) {
  neo_ast node = (neo_ast)malloc(sizeof(struct _neo_ast));
  memset(node, 0, sizeof(struct _neo_ast));
  node->type = NEO_AST_TYPE_SYMBOL;
  node->s_data = strings_clone(value);
  return node;
}