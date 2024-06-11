#include "compiler/include/compiler.h"
#include "common/include/cstring.h"
#include "common/include/list.h"
#include "token.h"
#include "tokendef.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
struct _neo_compiler {
  neo_tokenizer tokenizer;
  neo_list tokens;
  neo_list_node position;
  char *error;
};
neo_compiler create_neo_compiler() {
  neo_compiler compiler = malloc(sizeof(struct _neo_compiler));
  compiler->tokenizer = create_neo_tokenizer();
  compiler->error = NULL;
  return compiler;
}
void free_neo_compiler(neo_compiler compiler) {
  if (compiler->error) {
    free(compiler->error);
  }
  free_neo_tokenizer(compiler->tokenizer);
  free(compiler);
}
neo_ast neo_compiler_compile(neo_compiler compiler, const cstring source,
                             const cstring filename) {
  if (!neo_tokenizer_parse(compiler->tokenizer, source, filename)) {
    compiler->error =
        cstring_clone(neo_tokenizer_get_error(compiler->tokenizer));
    return NULL;
  }
  compiler->tokens = neo_tokenizer_get_token_list(compiler->tokenizer);
  compiler->position = neo_list_node_next(neo_list_head(compiler->tokens));
  neo_list_node node = neo_list_head(compiler->tokens);
  while (node != neo_list_tail(compiler->tokens)) {
    neo_token token = neo_list_node_get(node);
    if (token) {
      const char *c = token->start;
      while (c != token->end) {
        printf("%c", *c);
        c++;
      }
      switch ((NEO_TOKEN_TYPE)token->type) {

      case NEO_TOKEN_TYPE_STRING:
        printf(" :string");
        break;
      case NEO_TOKEN_TYPE_TEMPLATE_STRING:
        printf(" :template-string");
        break;
      case NEO_TOKEN_TYPE_TEMPLATE_STRING_START:
        printf(" :template-string-start");
        break;
      case NEO_TOKEN_TYPE_TEMPLATE_STRING_PART:
        printf(" :template-string-part");
        break;
      case NEO_TOKEN_TYPE_TEMPLATE_STRING_END:
        printf(" :template-string-end");
        break;
      case NEO_TOKEN_TYPE_SYMBOL:
        printf(" :symbol");
        break;
      case NEO_TOKEN_TYPE_NUMBER:
        printf(" :number");
        break;
      case NEO_TOKEN_TYPE_WORD:
        printf(" :word");
        break;
      case NEO_TOKEN_TYPE_COMMENT:
        printf(" :comment");
        break;
      case NEO_TOKEN_TYPE_REGEX:
        printf(" :regex");
        break;
      default:
        break;
      }
      printf("\n");
    }
    node = neo_list_node_next(node);
  }
  return NULL;
}
const char *neo_compiler_get_error(neo_compiler compiler) {
  return compiler->error;
}

neo_tokenizer neo_compiler_get_tokenizer(neo_compiler compiler) {
  return compiler->tokenizer;
}