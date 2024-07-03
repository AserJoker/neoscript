#include "ast.h"

neo_ast_meta make_neo_ast(enum NEO_AST_TYPE type, neo_token start,
                          neo_token end) {
  neo_ast_meta meta = {type, start, end};
  return meta;
}