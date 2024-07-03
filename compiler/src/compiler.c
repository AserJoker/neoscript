#include "compiler/include/compiler.h"
#include "ast.h"
#include "common/include/cstring.h"
#include "common/include/list.h"
#include "token.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#define check_skip(symbol, type, compiler)                                     \
  do {                                                                         \
    neo_token token = neo_compiler_read_token(compiler, skips_default);        \
    if (!neo_token_is(token, symbol, type)) {                                  \
      char buf[1024];                                                          \
      sprintf(buf, "'%s' expected. at\n\t%s:%d:%d", symbol,                    \
              token->pos.filename, token->pos.line, token->pos.column);        \
      compiler->error = cstring_clone(buf);                                    \
      free_neo_ast(node);                                                      \
      return NULL;                                                             \
    }                                                                          \
    neo_compiler_next(compiler, skips_default);                                \
  } while (0)
struct _neo_compiler {
  neo_tokenizer tokenizer;
  neo_list tokens;
  neo_list_node position;
  char *error;
  int8_t is_inline;
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
static cstring symbol_0[] = {"++", "--", 0};
static cstring symbol_1[] = {"*", "/", "%", 0};
static cstring symbol_2[] = {"+", "-", 0};
static cstring symbol_3[] = {"<<", ">>", ">>>", 0};
static cstring symbol_4[] = {"<", ">", "<=", ">=", 0};
static cstring symbol_5[] = {"==", "!=", "===", "!==", 0};
static cstring symbol_6[] = {"&&", 0};
static cstring symbol_7[] = {"||", 0};
static cstring symbol_8[] = {
    "=", "+=", "-=", "*=", "/=", "%=", "<<=", ">>=", 0};
static cstring symbol_9[] = {",", 0};
static cstring *symbols[] = {symbol_0, symbol_1, symbol_2, symbol_3,
                             symbol_4, symbol_5, symbol_6, symbol_7,
                             symbol_8, symbol_9, 0};
static cstring binary_operator_keywords[] = {
    "true",     "false",  "null",       "undefined", "this",   "super", "class",
    "function", "typeof", "instanceof", "new",       "delete", "async", "await",
    "void",     0};
neo_ast neo_compiler_compile(neo_compiler compiler, const cstring source,
                             const cstring filename) {
  if (!neo_tokenizer_parse(compiler->tokenizer, source, filename)) {
    compiler->error =
        cstring_clone(neo_tokenizer_get_error(compiler->tokenizer));
    return NULL;
  }
  compiler->tokens = neo_tokenizer_get_token_list(compiler->tokenizer);
  compiler->position = neo_list_node_next(neo_list_head(compiler->tokens));
  return NULL;
}
const char *neo_compiler_get_error(neo_compiler compiler) {
  return compiler->error;
}
neo_tokenizer neo_compiler_get_tokenizer(neo_compiler compiler) {
  return compiler->tokenizer;
}
