#include "compiler/include/compiler.h"
#include "ast.h"
#include "astdef.h"
#include "common/include/cstring.h"
#include "common/include/list.h"
#include "token.h"
#include "tokendef.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
static neo_ast neo_compiler_read_expression(neo_compiler compiler);
static neo_ast neo_compiler_read_statement(neo_compiler compiler);
static neo_ast neo_compiler_read_statements(neo_compiler compiler);
static neo_ast neo_compiler_read_block_statement(neo_compiler compiler);
static neo_ast neo_compiler_read_lambda(neo_compiler compiler);
static neo_ast neo_compiler_read_array_destruct(neo_compiler compiler);
static neo_ast neo_compiler_read_object_destruct(neo_compiler compiler);
static neo_ast neo_compiler_read_rest(neo_compiler compiler);
static int8_t neo_compiler_expression_is_completed(neo_ast node) {
  while (node && node->type == NEO_AST_TYPE_EXPRESSION) {
    node = node->right;
  }
  if (node) {
    return 1;
  }
  return 0;
}
static int8_t neo_compiler_expression_append(neo_ast *root, neo_ast node) {
  if (!*root) {
    *root = node;
    return 1;
  }
  if (node->type == NEO_AST_TYPE_EXPRESSION) {
    neo_ast tmp = *root;
    if (tmp->level > node->level) {
      while (tmp->right && tmp->right->level > node->level) {
        tmp = tmp->right;
      }
      node->left = tmp->right;
      tmp->right = node;
    } else {
      node->left = *root;
      *root = node;
    }
    return 1;
  } else if (node->type == NEO_AST_TYPE_CALL ||
             node->type == NEO_AST_TYPE_MEMBER ||
             node->type == NEO_AST_TYPE_OPTIONAL_CALL ||
             node->type == NEO_AST_TYPE_OPTIONAL_MEMBER) {
    neo_ast tmp = *root;
    if (tmp->level == -1) {
      node->left = tmp;
      *root = node;
    } else {
      while (tmp->right->level != -1) {
        tmp = tmp->right;
      }
      node->left = tmp->right;
      tmp->right = node;
    }
    return 1;
  } else if (node->type == NEO_AST_TYPE_TERNARY) {
    neo_ast tmp = *root;
    if (tmp->level < 9) {
      node->left = tmp;
      *root = node;
    } else {
      while (tmp->right->level >= 9) {
        tmp = tmp->right;
      }
      node->left = tmp->right;
      tmp->right = node;
    }
    return 1;
  } else {
    neo_ast tmp = *root;
    while (tmp->right) {
      tmp = tmp->right;
    }
    if (tmp->type == NEO_AST_TYPE_EXPRESSION) {
      tmp->right = node;
      return 1;
    }
  }
  return 0;
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
static uint32_t skips_default[] = {NEO_TOKEN_TYPE_COMMENT, NEO_TOKEN_TYPE_END,
                                   0};
static uint32_t skips_inline[] = {NEO_TOKEN_TYPE_COMMENT, 0};

static void neo_compiler_skip(neo_compiler compiler, const uint32_t skips[]) {
  while (compiler->position != neo_list_tail(compiler->tokens)) {
    neo_token token = neo_list_node_get(compiler->position);
    int8_t flag = 0;
    for (int index = 0; skips[index] != 0; index++) {
      if (token->type == skips[index]) {
        flag = 1;
        break;
      }
    }
    if (flag) {
      compiler->position = neo_list_node_next(compiler->position);
    } else {
      break;
    }
  }
}
static void neo_compiler_next(neo_compiler compiler, const uint32_t skips[]) {
  neo_compiler_skip(compiler, skips);
  compiler->position = neo_list_node_next(compiler->position);
}
static neo_token neo_compiler_read_token(neo_compiler compiler,
                                         const uint32_t skips[]) {
  neo_compiler_skip(compiler, skips);
  return neo_list_node_get(compiler->position);
}
static neo_ast neo_compiler_read_number(neo_compiler compiler) {
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  char *buf = malloc(token->end - token->start + 1);
  memset(buf, 0, token->end - token->start + 1);
  strncpy(buf, token->start, token->end - token->start);
  neo_ast node = create_neo_number_ast(atof(buf));
  free(buf);
  neo_compiler_next(compiler, skips_default);
  return node;
}
static neo_ast neo_compiler_read_string(neo_compiler compiler) {
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  char *buf = malloc(token->end - token->start + 1);
  memset(buf, 0, token->end - token->start + 1);
  int index = 0;
  for (const char *c = token->start + 1; c != token->end - 1; c++) {
    if (*c == '\\') {
      c++;
      buf[index] = *c - 'a' + '\a';
    } else {
      buf[index] = *c;
    }
    index++;
  }
  buf[index] = 0;
  neo_ast node = create_neo_string_ast(buf);
  free(buf);
  neo_compiler_next(compiler, skips_default);
  return node;
}
static neo_ast neo_compiler_read_regex(neo_compiler compiler) {
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  char *buf = malloc(token->end - token->start + 1);
  memset(buf, 0, token->end - token->start + 1);
  strncpy(buf, token->start, token->end - token->start);
  neo_ast node = create_neo_regex_ast(buf);
  free(buf);
  neo_compiler_next(compiler, skips_default);
  return node;
}
static neo_ast neo_compiler_read_word(neo_compiler compiler) {
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  char *buf = malloc(token->end - token->start + 1);
  memset(buf, 0, token->end - token->start + 1);
  strncpy(buf, token->start, token->end - token->start);
  neo_ast node = create_neo_symbol_ast(buf);
  free(buf);
  neo_compiler_next(compiler, skips_default);
  return node;
}
static neo_ast neo_compiler_read_brackets(neo_compiler compiler) {
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  neo_list_node pair = compiler->position;
  token = neo_list_node_get(pair);
  uint32_t flag = 0;
  while (token->type != NEO_TOKEN_TYPE_EOF) {
    if (neo_token_is(token, "(", NEO_TOKEN_TYPE_SYMBOL)) {
      flag++;
    }
    if (neo_token_is(token, ")", NEO_TOKEN_TYPE_SYMBOL)) {
      flag--;
      if (!flag) {
        break;
      }
    }
    pair = neo_list_node_next(pair);
    token = neo_list_node_get(pair);
  }
  if (token->type == NEO_TOKEN_TYPE_EOF) {
    char buf[1024];
    sprintf(buf, "')' expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    return NULL;
  }
  neo_list_node current = compiler->position;
  compiler->position = pair;
  neo_compiler_next(compiler, skips_inline);
  neo_token next_token = neo_compiler_read_token(compiler, skips_inline);
  compiler->position = current;
  if (neo_token_is(next_token, "=>", NEO_TOKEN_TYPE_SYMBOL)) {
    return neo_compiler_read_lambda(compiler);
  }
  neo_compiler_next(compiler, skips_default);
  int8_t is_inline = compiler->is_inline;
  compiler->is_inline = 0;
  neo_ast content = neo_compiler_read_expression(compiler);
  compiler->is_inline = is_inline;
  if (!content) {
    return NULL;
  }
  token = neo_compiler_read_token(compiler, skips_default);
  if (!neo_token_is(token, ")", NEO_TOKEN_TYPE_SYMBOL)) {
    char buf[1024];
    sprintf(buf, "')' expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    free_neo_ast(content);
    return NULL;
  }
  neo_ast node = create_neo_ast(NEO_AST_TYPE_BRACKETS, 0, content, 0);
  node->level = -1;
  neo_compiler_next(compiler, skips_default);
  return node;
}
static neo_ast neo_compiler_read_symbol(neo_compiler compiler) {
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  char buf[8] = {0};
  strncpy(buf, token->start, token->end - token->start);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_EXPRESSION, token, NULL, NULL);
  neo_compiler_next(compiler, skips_default);
  return node;
}
static neo_ast neo_compiler_read_function_call(neo_compiler compiler) {
  neo_compiler_next(compiler, skips_default);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_CALL, 0, 0, NULL);
  neo_ast arg = NULL;
  neo_ast args = node;
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  if (token->type == NEO_TOKEN_TYPE_EOF) {
    char buf[1024];
    sprintf(buf, "')' expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    free_neo_ast(node);
    return NULL;
  }
  if (!neo_token_is(token, ")", NEO_TOKEN_TYPE_SYMBOL)) {
    for (;;) {
      int8_t is_inline = compiler->is_inline;
      compiler->is_inline = 1;
      arg = neo_compiler_read_expression(compiler);
      compiler->is_inline = is_inline;
      if (!arg) {
        free_neo_ast(node);
        return NULL;
      }
      token = neo_compiler_read_token(compiler, skips_default);
      if (token && *token->start == ',') {
        neo_ast next = create_neo_ast(NEO_AST_TYPE_PAIR, 0, arg, 0);
        args->right = next;
        args = next;
        neo_compiler_next(compiler, skips_default);
      } else if (token && *token->start == ')') {
        args->right = arg;
        break;
      } else {
        char buf[1024];
        sprintf(buf, "')' expected. at\n\t%s:%d:%d", token->pos.filename,
                token->pos.line, token->pos.column);
        compiler->error = cstring_clone(buf);
        free_neo_ast(node);
        return NULL;
      }
    }
  }
  neo_compiler_next(compiler, skips_default);
  node->level = -1;
  return node;
}
static neo_ast neo_compiler_read_array(neo_compiler compiler) {
  neo_ast node = create_neo_ast(NEO_AST_TYPE_ARRAY, 0, 0, 0);
  neo_compiler_next(compiler, skips_default);
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  if (token->type == NEO_TOKEN_TYPE_EOF) {
    char buf[1024];
    sprintf(buf, "']' expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    free_neo_ast(node);
    return NULL;
  }
  if (neo_token_is(token, "]", NEO_TOKEN_TYPE_SYMBOL)) {
    neo_compiler_next(compiler, skips_default);
    return node;
  }
  neo_ast items = node;
  neo_ast item = NULL;
  for (;;) {
    token = neo_compiler_read_token(compiler, skips_default);
    if (neo_token_is(token, "...", NEO_TOKEN_TYPE_SYMBOL)) {
      item = neo_compiler_read_rest(compiler);
    } else {
      int8_t is_inline = compiler->is_inline;
      compiler->is_inline = 1;
      item = neo_compiler_read_expression(compiler);
      compiler->is_inline = is_inline;
    }
    if (!item) {
      free_neo_ast(node);
      return NULL;
    }
    token = neo_compiler_read_token(compiler, skips_default);
    if (neo_token_is(token, ",", NEO_TOKEN_TYPE_SYMBOL)) {
      neo_ast next = create_neo_ast(NEO_AST_TYPE_PAIR, 0, item, 0);
      items->right = next;
      items = next;
      neo_compiler_next(compiler, skips_default);
    } else if (neo_token_is(token, "]", NEO_TOKEN_TYPE_SYMBOL)) {
      items->right = item;
      break;
    } else {
      char buf[1024];
      sprintf(buf, "']' expected. at\n\t%s:%d:%d", token->pos.filename,
              token->pos.line, token->pos.column);
      compiler->error = cstring_clone(buf);
      free_neo_ast(node);
      return NULL;
    }
  }
  neo_compiler_next(compiler, skips_default);
  node->level = -1;
  return node;
}
static neo_ast neo_compiler_read_array_member(neo_compiler compiler) {
  neo_compiler_next(compiler, skips_default);
  neo_ast node = NULL;
  int8_t is_inline = compiler->is_inline;
  compiler->is_inline = 0;
  neo_ast field = neo_compiler_read_expression(compiler);
  compiler->is_inline = is_inline;
  if (!field) {
    return NULL;
  }
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  if (!neo_token_is(token, "]", NEO_TOKEN_TYPE_SYMBOL)) {
    char buf[1024];
    sprintf(buf, "']' expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    free_neo_ast(field);
    return NULL;
  }
  neo_compiler_next(compiler, skips_default);
  node = create_neo_ast(NEO_AST_TYPE_MEMBER, 0, 0, field);
  node->level = -1;
  return node;
}
static neo_ast neo_compiler_read_new(neo_compiler compiler) {
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_EXPRESSION, token, 0, 0);
  node->level = 0;
  neo_compiler_next(compiler, skips_default);
  return node;
}
static neo_ast neo_compiler_read_await(neo_compiler compiler) {
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_EXPRESSION, token, 0, 0);
  node->level = 0;
  neo_compiler_next(compiler, skips_default);
  return node;
}
static neo_ast neo_compiler_read_yield(neo_compiler compiler) {
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_EXPRESSION, token, 0, 0);
  node->level = 0;
  neo_compiler_next(compiler, skips_inline);
  neo_token next = neo_compiler_read_token(compiler, skips_inline);
  if (next->type == NEO_TOKEN_TYPE_EOF || next->type == NEO_TOKEN_TYPE_END ||
      neo_token_is(next, ";", NEO_TOKEN_TYPE_SYMBOL)) {
    node->level = -1;
    node->right = create_neo_undefined_ast();
  }
  return node;
}
static neo_ast neo_compiler_read_void(neo_compiler compiler) {
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_EXPRESSION, token, 0, 0);
  node->level = 0;
  neo_compiler_next(compiler, skips_default);
  return node;
}
static neo_ast neo_compiler_read_delete(neo_compiler compiler) {
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_EXPRESSION, token, 0, 0);
  node->level = 0;
  neo_compiler_next(compiler, skips_default);
  return node;
}
static neo_ast neo_compiler_read_typeof(neo_compiler compiler) {
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_EXPRESSION, token, 0, 0);
  node->level = 0;
  neo_compiler_next(compiler, skips_default);
  return node;
}
static neo_ast neo_compiler_read_instanceof(neo_compiler compiler) {
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_EXPRESSION, token, 0, 0);
  node->level = 0;
  neo_compiler_next(compiler, skips_default);
  return node;
}
static neo_ast neo_compiler_read_member(neo_compiler compiler) {
  neo_compiler_next(compiler, skips_default);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_MEMBER, 0, 0, 0);
  node->right = neo_compiler_read_word(compiler);
  node->level = -1;
  return node;
}
static neo_ast neo_compiler_read_rest(neo_compiler compiler) {
  neo_ast node = create_neo_ast(NEO_AST_TYPE_REST, 0, 0, 0);
  neo_compiler_next(compiler, skips_default);
  int8_t is_inline = compiler->is_inline;
  compiler->is_inline = 1;
  node->right = neo_compiler_read_expression(compiler);
  compiler->is_inline = is_inline;
  if (!node->right) {
    free_neo_ast(node);
    return NULL;
  }
  return node;
}
static neo_ast neo_compiler_read_arg_def(neo_compiler compiler) {
  neo_ast node = create_neo_ast(NEO_AST_TYPE_ARG_DEF, 0, 0, 0);
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  if (neo_token_is(token, "[", NEO_TOKEN_TYPE_SYMBOL)) {
    node->left = neo_compiler_read_array_destruct(compiler);
  } else if (neo_token_is(token, "{", NEO_TOKEN_TYPE_SYMBOL)) {
    node->left = neo_compiler_read_object_destruct(compiler);
  } else if (neo_token_is(token, "...", NEO_TOKEN_TYPE_SYMBOL)) {
    node->left = neo_compiler_read_rest(compiler);
  } else {
    node->left = neo_compiler_read_word(compiler);
  }
  if (!node->left) {
    free_neo_ast(node);
    return NULL;
  }
  token = neo_compiler_read_token(compiler, skips_default);
  if (neo_token_is(token, "=", NEO_TOKEN_TYPE_SYMBOL)) {
    neo_compiler_next(compiler, skips_default);
    int8_t is_inline = compiler->is_inline;
    compiler->is_inline = 1;
    node->right = neo_compiler_read_expression(compiler);
    compiler->is_inline = 0;
    if (!node->right) {
      free_neo_ast(node);
      return NULL;
    }
  }
  return node;
}
static neo_ast neo_compiler_read_function_meta(neo_compiler compiler) {
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  neo_ast meta = create_neo_ast(NEO_AST_TYPE_FUNCTION_META, 0, 0, 0);
  if (token->type == NEO_TOKEN_TYPE_WORD) {
    meta->left = neo_compiler_read_word(compiler);
    token = neo_compiler_read_token(compiler, skips_default);
  }
  if (!neo_token_is(token, "(", NEO_TOKEN_TYPE_SYMBOL)) {
    char buf[1024];
    sprintf(buf, "'(' expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    free_neo_ast(meta);
    return NULL;
  }
  neo_compiler_next(compiler, skips_default);
  token = neo_compiler_read_token(compiler, skips_default);
  if (!neo_token_is(token, ")", NEO_TOKEN_TYPE_SYMBOL)) {
    neo_ast args = meta;
    neo_ast arg = NULL;
    for (;;) {
      arg = neo_compiler_read_arg_def(compiler);
      if (!arg) {
        free_neo_ast(meta);
        return NULL;
      }
      token = neo_compiler_read_token(compiler, skips_default);
      if (neo_token_is(token, ",", NEO_TOKEN_TYPE_SYMBOL)) {
        neo_ast next = create_neo_ast(NEO_AST_TYPE_PAIR, 0, arg, 0);
        args->right = next;
        args = next;
        neo_compiler_next(compiler, skips_default);
      } else if (neo_token_is(token, ")", NEO_TOKEN_TYPE_SYMBOL)) {
        args->right = arg;
        break;
      } else {
        if (arg) {
          free_neo_ast(arg);
        }
        char buf[1024];
        sprintf(buf, "'(' expected. at\n\t%s:%d:%d", token->pos.filename,
                token->pos.line, token->pos.column);
        compiler->error = cstring_clone(buf);
        free_neo_ast(meta);
        return NULL;
      }
    }
  }
  if (!neo_token_is(token, ")", NEO_TOKEN_TYPE_SYMBOL)) {
    char buf[1024];
    sprintf(buf, "'(' expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    free_neo_ast(meta);
    return NULL;
  }
  neo_compiler_next(compiler, skips_default);
  return meta;
}
static neo_ast neo_compiler_read_function_def(neo_compiler compiler) {
  neo_compiler_next(compiler, skips_default);
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  if (token->type == NEO_TOKEN_TYPE_EOF) {
    char buf[1024];
    sprintf(buf, "Identifier expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    return NULL;
  }
  neo_ast node = create_neo_ast(NEO_AST_TYPE_FUNCTION, 0, 0, 0);
  node->level = -1;
  if (neo_token_is(token, "*", NEO_TOKEN_TYPE_SYMBOL)) {
    node->type = NEO_AST_TYPE_GENERATOR_FUNCTION;
    neo_compiler_next(compiler, skips_default);
  }
  neo_ast meta = neo_compiler_read_function_meta(compiler);
  if (!meta) {
    free_neo_ast(node);
    return meta;
  }
  node->left = meta;
  token = neo_compiler_read_token(compiler, skips_default);
  if (neo_token_is(token, ";", NEO_TOKEN_TYPE_SYMBOL)) {
    neo_compiler_next(compiler, skips_default);
    node->type = NEO_AST_TYPE_FUNCTION_DECLARATION;
    return node;
  }
  neo_ast body = neo_compiler_read_block_statement(compiler);
  if (!body) {
    free_neo_ast(node);
    return NULL;
  }
  node->right = body;
  return node;
}
static neo_ast neo_compiler_read_lambda(neo_compiler compiler) {
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_LAMBDA, 0, 0, 0);
  node->level = -1;
  if (neo_token_is(token, "(", NEO_TOKEN_TYPE_SYMBOL)) {
    neo_ast meta = neo_compiler_read_function_meta(compiler);
    if (!meta) {
      free_neo_ast(node);
      return NULL;
    }
    node->left = meta;
    token = neo_compiler_read_token(compiler, skips_default);
  } else if (token->type == NEO_TOKEN_TYPE_WORD) {
    neo_ast meta = create_neo_ast(NEO_AST_TYPE_FUNCTION_META, 0, 0, 0);
    node->left = meta;
    meta->right = create_neo_ast(NEO_AST_TYPE_ARG_DEF, 0, 0, 0);
    meta->right->left = neo_compiler_read_word(compiler);
    token = neo_compiler_read_token(compiler, skips_default);
  } else {
    char buf[1024];
    sprintf(buf, "Unexpect token. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    free_neo_ast(node);
    return NULL;
  }
  if (!neo_token_is(token, "=>", NEO_TOKEN_TYPE_SYMBOL)) {
    char buf[1024];
    sprintf(buf, "'=>' expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    free_neo_ast(node);
    return NULL;
  }
  neo_compiler_next(compiler, skips_default);
  token = neo_compiler_read_token(compiler, skips_default);
  neo_ast body = NULL;
  if (neo_token_is(token, "{", NEO_TOKEN_TYPE_SYMBOL)) {
    body = neo_compiler_read_block_statement(compiler);
  } else {
    int8_t is_inline = compiler->is_inline;
    compiler->is_inline = 1;
    body = neo_compiler_read_expression(compiler);
    compiler->is_inline = is_inline;
  }
  if (!body) {
    free_neo_ast(node);
    return NULL;
  }
  node->right = body;
  return node;
}
static neo_ast neo_compiler_read_async(neo_compiler compiler) {
  neo_compiler_next(compiler, skips_default);
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  if (neo_token_is(token, "function", NEO_TOKEN_TYPE_KEYWORD)) {
    neo_ast func = neo_compiler_read_function_def(compiler);
    if (!func) {
      return NULL;
    }
    if (func->type == NEO_AST_TYPE_FUNCTION) {
      func->type = NEO_AST_TYPE_ASYNC_FUNCTION;
    } else {
      func->type = NEO_AST_TYPE_ASYNC_GENERATOR_FUNCTION;
    }
    return func;
  } else {
    neo_ast func = neo_compiler_read_lambda(compiler);
    if (!func) {
      return NULL;
    }
    func->type = NEO_AST_TYPE_ASYNC_LAMBDA;
    return func;
  }
}
static neo_ast neo_compiler_read_ternary(neo_compiler compiler) {
  neo_compiler_next(compiler, skips_default);
  int8_t is_inline = compiler->is_inline;
  compiler->is_inline = 1;
  neo_ast truth = neo_compiler_read_expression(compiler);
  if (!truth) {
    return NULL;
  }
  compiler->is_inline = 1;
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  if (!neo_token_is(token, ":", NEO_TOKEN_TYPE_SYMBOL)) {
    char buf[1024];
    sprintf(buf, "':' expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    free_neo_ast(truth);
    return NULL;
  }
  neo_compiler_next(compiler, skips_default);
  neo_ast falsy = neo_compiler_read_expression(compiler);
  if (!falsy) {
    free_neo_ast(truth);
    return NULL;
  }
  compiler->is_inline = is_inline;
  neo_ast node = create_neo_ast(NEO_AST_TYPE_TERNARY, 0, 0, 0);
  neo_ast body = create_neo_ast(NEO_AST_TYPE_TERNARY_BODY, 0, truth, falsy);
  node->right = body;
  node->level = -1;
  return node;
}
static neo_ast neo_compiler_read_optional(neo_compiler compiler) {
  neo_compiler_next(compiler, skips_default);
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  if (neo_token_is(token, "[", NEO_TOKEN_TYPE_SYMBOL)) {
    neo_ast node = neo_compiler_read_array_member(compiler);
    node->type = NEO_AST_TYPE_OPTIONAL_MEMBER;
    return node;
  } else if (neo_token_is(token, "(", NEO_TOKEN_TYPE_SYMBOL)) {
    neo_ast node = neo_compiler_read_function_call(compiler);
    node->type = NEO_AST_TYPE_OPTIONAL_CALL;
    return node;
  } else if (token->type == NEO_TOKEN_TYPE_WORD) {
    neo_ast node = create_neo_ast(NEO_AST_TYPE_OPTIONAL_MEMBER, 0, 0, 0);
    node->right = neo_compiler_read_word(compiler);
    node->level = -1;
    return node;
  } else {
    char buf[1024];
    sprintf(buf, "Identifier expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    return NULL;
  }
}
static neo_ast neo_compiler_read_array_destruct_field(neo_compiler compiler) {
  neo_ast node = create_neo_ast(NEO_AST_TYPE_ARRAY_DESTRUCT_FIELD, 0, 0, 0);
  node->left = neo_compiler_read_word(compiler);
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  if (neo_token_is(token, "=", NEO_TOKEN_TYPE_SYMBOL)) {
    neo_compiler_next(compiler, skips_default);
    int8_t is_inline = compiler->is_inline;
    compiler->is_inline = 1;
    node->right = neo_compiler_read_expression(compiler);
    compiler->is_inline = is_inline;
    if (!node->right) {
      free_neo_ast(node);
      return NULL;
    }
  }
  return node;
}
static neo_ast neo_compiler_read_array_destruct(neo_compiler compiler) {
  neo_compiler_next(compiler, skips_default);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_ARRAY_DESTRUCT, 0, 0, 0);
  node->level = -1;
  neo_ast list = node;
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  neo_ast item;
  for (;;) {
    item = neo_compiler_read_array_destruct_field(compiler);
    if (!item) {
      free_neo_ast(node);
      return NULL;
    }
    token = neo_compiler_read_token(compiler, skips_default);
    if (neo_token_is(token, ",", NEO_TOKEN_TYPE_SYMBOL)) {
      neo_ast items = create_neo_ast(NEO_AST_TYPE_PAIR, 0, item, 0);
      list->right = items;
      list = items;
      neo_compiler_next(compiler, skips_default);
    } else if (neo_token_is(token, "]", NEO_TOKEN_TYPE_SYMBOL)) {
      list->right = item;
      break;
    } else {
      char buf[1024];
      sprintf(buf, "Identifier expected. at\n\t%s:%d:%d", token->pos.filename,
              token->pos.line, token->pos.column);
      compiler->error = cstring_clone(buf);
      free_neo_ast(node);
      return NULL;
    }
  }
  neo_compiler_next(compiler, skips_default);
  return node;
}
static neo_ast neo_compiler_read_object_destruct_field(neo_compiler compiler) {
  neo_ast node = create_neo_ast(NEO_AST_TYPE_OBJECT_DESTRUCT_FIELD, 0, 0, 0);
  neo_ast name =
      create_neo_ast(NEO_AST_TYPE_OBJECT_DESTRUCT_FIELD_NAME, 0, 0, 0);
  node->left = name;
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  if (token->type == NEO_TOKEN_TYPE_STRING) {
    name->left = neo_compiler_read_string(compiler);
  } else if (token->type == NEO_TOKEN_TYPE_WORD) {
    name->left = neo_compiler_read_word(compiler);
  } else {
    char buf[1024];
    sprintf(buf, "Identifier expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    free_neo_ast(node);
    return NULL;
  }
  token = neo_compiler_read_token(compiler, skips_default);
  if (neo_token_is(token, ":", NEO_TOKEN_TYPE_SYMBOL)) {
    neo_compiler_next(compiler, skips_default);
    name->right = neo_compiler_read_word(compiler);
    token = neo_compiler_read_token(compiler, skips_default);
  }
  if (neo_token_is(token, "=", NEO_TOKEN_TYPE_SYMBOL)) {
    neo_compiler_next(compiler, skips_default);
    int8_t is_inline = compiler->is_inline;
    compiler->is_inline = 1;
    node->right = neo_compiler_read_expression(compiler);
    compiler->is_inline = is_inline;
    if (!node->right) {
      free_neo_ast(node);
      return NULL;
    }
  }
  return node;
}
static neo_ast neo_compiler_read_object_destruct(neo_compiler compiler) {
  neo_compiler_next(compiler, skips_default);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_OBJECT_DESTRUCT, 0, 0, 0);
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  neo_ast list = node;
  if (!neo_token_is(token, "}", NEO_TOKEN_TYPE_SYMBOL)) {
    neo_ast item = NULL;
    for (;;) {
      item = neo_compiler_read_object_destruct_field(compiler);
      if (!item) {
        free_neo_ast(node);
        return NULL;
      }
      token = neo_compiler_read_token(compiler, skips_default);
      if (neo_token_is(token, ",", NEO_TOKEN_TYPE_SYMBOL)) {
        neo_ast items = create_neo_ast(NEO_AST_TYPE_PAIR, 0, item, 0);
        list->right = items;
        list = items;
        neo_compiler_next(compiler, skips_default);
      } else if (neo_token_is(token, "}", NEO_TOKEN_TYPE_SYMBOL)) {
        list->right = item;
        break;
      } else {
        free_neo_ast(item);
        char buf[1024];
        sprintf(buf, "Identifier expected. at\n\t%s:%d:%d", token->pos.filename,
                token->pos.line, token->pos.column);
        compiler->error = cstring_clone(buf);
        free_neo_ast(node);
        return NULL;
      }
    }
  }
  neo_compiler_next(compiler, skips_default);
  return node;
}
static neo_ast neo_compiler_read_object_function(neo_compiler compiler) {
  neo_ast func_def = create_neo_ast(NEO_AST_TYPE_OBJECT_FUNCTION, 0, 0, 0);
  neo_ast meta = neo_compiler_read_function_meta(compiler);
  if (!meta) {
    free_neo_ast(func_def);
    return NULL;
  }
  func_def->left = meta;
  neo_ast body = neo_compiler_read_block_statement(compiler);
  if (!body) {
    free_neo_ast(func_def);
    return NULL;
  }
  func_def->right = body;
  return func_def;
}
static neo_ast neo_compiler_read_object_field(neo_compiler compiler) {
  neo_ast node = create_neo_ast(NEO_AST_TYPE_OBJECT_FIELD, 0, 0, 0);
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  if (token->type == NEO_TOKEN_TYPE_STRING) {
    node->left = neo_compiler_read_string(compiler);
    node->type = NEO_AST_TYPE_OBJECT_COMPUTED_FIELD;
  } else if (token->type == NEO_TOKEN_TYPE_WORD) {
    node->left = neo_compiler_read_word(compiler);
  } else if (neo_token_is(token, "[", NEO_TOKEN_TYPE_SYMBOL)) {
    node->type = NEO_AST_TYPE_OBJECT_COMPUTED_FIELD;
    neo_compiler_next(compiler, skips_default);
    int8_t is_inline = compiler->is_inline;
    compiler->is_inline = 1;
    node->left = neo_compiler_read_expression(compiler);
    compiler->is_inline = is_inline;
    token = neo_compiler_read_token(compiler, skips_default);
    if (!neo_token_is(token, "]", NEO_TOKEN_TYPE_SYMBOL)) {
      char buf[1024];
      sprintf(buf, "']' expected. at\n\t%s:%d:%d", token->pos.filename,
              token->pos.line, token->pos.column);
      compiler->error = cstring_clone(buf);
      free_neo_ast(node);
      return NULL;
    } else {
      neo_compiler_next(compiler, skips_default);
    }
  } else if (neo_token_is(token, "get", NEO_TOKEN_TYPE_KEYWORD)) {
    node->type = NEO_AST_TYPE_OBJECT_GET;
    neo_compiler_next(compiler, skips_default);
    token = neo_compiler_read_token(compiler, skips_default);
    if (token->type != NEO_TOKEN_TYPE_WORD) {
      char buf[1024];
      sprintf(buf, "Identifier expected. at\n\t%s:%d:%d", token->pos.filename,
              token->pos.line, token->pos.column);
      compiler->error = cstring_clone(buf);
      free_neo_ast(node);
      return NULL;
    }
    node->left = neo_compiler_read_word(compiler);
    node->right = neo_compiler_read_object_function(compiler);
    if (!node->right) {
      free_neo_ast(node);
      return NULL;
    }
  } else if (neo_token_is(token, "set", NEO_TOKEN_TYPE_KEYWORD)) {
    node->type = NEO_AST_TYPE_OBJECT_SET;
    neo_compiler_next(compiler, skips_default);
    token = neo_compiler_read_token(compiler, skips_default);
    if (token->type != NEO_TOKEN_TYPE_WORD) {
      char buf[1024];
      sprintf(buf, "Identifier expected. at\n\t%s:%d:%d", token->pos.filename,
              token->pos.line, token->pos.column);
      compiler->error = cstring_clone(buf);
      free_neo_ast(node);
      return NULL;
    }
    node->left = neo_compiler_read_word(compiler);
    node->right = neo_compiler_read_object_function(compiler);
    if (!node->right) {
      free_neo_ast(node);
      return NULL;
    }
  } else if (neo_token_is(token, "...", NEO_TOKEN_TYPE_SYMBOL)) {
    node->right = neo_compiler_read_rest(compiler);
    if (!node->right) {
      free_neo_ast(node);
      return NULL;
    }
  } else {
    char buf[1024];
    sprintf(buf, "Identifier expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    free_neo_ast(node);
    return NULL;
  }
  token = neo_compiler_read_token(compiler, skips_default);
  if (neo_token_is(token, ",", NEO_TOKEN_TYPE_SYMBOL) ||
      neo_token_is(token, "}", NEO_TOKEN_TYPE_SYMBOL)) {
    return node;
  } else if (neo_token_is(token, "(", NEO_TOKEN_TYPE_SYMBOL)) {
    neo_ast func_def = neo_compiler_read_object_function(compiler);
    if (!func_def) {
      free_neo_ast(node);
      return NULL;
    }
    node->right = func_def;
  } else if (neo_token_is(token, ":", NEO_TOKEN_TYPE_SYMBOL)) {
    neo_compiler_next(compiler, skips_default);
    int8_t is_inline = compiler->is_inline;
    compiler->is_inline = 1;
    node->right = neo_compiler_read_expression(compiler);
    compiler->is_inline = is_inline;
  } else {
    char buf[1024];
    sprintf(buf, "Identifier expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    free_neo_ast(node);
    return NULL;
  }
  return node;
}
static neo_ast neo_compiler_read_object(neo_compiler compiler) {
  neo_ast node = create_neo_ast(NEO_AST_TYPE_OBJECT, 0, 0, 0);
  neo_compiler_next(compiler, skips_default);
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  neo_ast items = node;
  if (!neo_token_is(token, "}", NEO_TOKEN_TYPE_SYMBOL)) {
    neo_ast item = NULL;
    for (;;) {
      item = neo_compiler_read_object_field(compiler);
      if (!item) {
        free_neo_ast(node);
        return NULL;
      }
      token = neo_compiler_read_token(compiler, skips_default);
      if (neo_token_is(token, ",", NEO_TOKEN_TYPE_SYMBOL)) {
        neo_ast list = create_neo_ast(NEO_AST_TYPE_PAIR, 0, item, 0);
        items->right = list;
        items = list;
        neo_compiler_next(compiler, skips_default);
      } else if (neo_token_is(token, "}", NEO_TOKEN_TYPE_SYMBOL)) {
        items->right = item;
        break;
      } else {
        free_neo_ast(item);
        char buf[1024];
        sprintf(buf, "'}' expected. at\n\t%s:%d:%d", token->pos.filename,
                token->pos.line, token->pos.column);
        compiler->error = cstring_clone(buf);
        free_neo_ast(node);
        return NULL;
      }
    }
  }
  if (!neo_token_is(token, "}", NEO_TOKEN_TYPE_SYMBOL)) {
    char buf[1024];
    sprintf(buf, "'}' expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    free_neo_ast(node);
    return NULL;
  }
  neo_compiler_next(compiler, skips_default);
  return node;
}
static neo_ast neo_compiler_read_this(neo_compiler compiler) {
  neo_compiler_next(compiler, skips_default);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_THIS, 0, 0, 0);
  node->level = -1;
  return node;
}
static neo_ast neo_compiler_read_super(neo_compiler compiler) {
  neo_compiler_next(compiler, skips_default);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_SUPER, 0, 0, 0);
  node->level = -1;
  return node;
}
static neo_ast neo_compiler_read_decorator(neo_compiler compiler) {
  neo_ast node = create_neo_ast(NEO_AST_TYPE_DECORATOR, 0, 0, 0);
  neo_compiler_next(compiler, skips_default);
  neo_ast word = neo_compiler_read_word(compiler);
  node->left = word;
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  if (neo_token_is(token, "(", NEO_TOKEN_TYPE_SYMBOL)) {
    neo_ast call = neo_compiler_read_function_call(compiler);
    if (!call) {
      free_neo_ast(node);
      return NULL;
    }
    call->left = word;
    node->left = call;
  }
  node->level = -1;
  return node;
}
static neo_ast neo_compiler_read_class_meta(neo_compiler compiler) {
  neo_ast node = create_neo_ast(NEO_AST_TYPE_CLASS_META, 0, 0, 0);
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  if (token->type == NEO_TOKEN_TYPE_WORD) {
    node->left = neo_compiler_read_word(compiler);
  }
  token = neo_compiler_read_token(compiler, skips_default);
  if (neo_token_is(token, "extends", NEO_TOKEN_TYPE_KEYWORD)) {
    neo_compiler_next(compiler, skips_default);
    token = neo_compiler_read_token(compiler, skips_default);
    if (token->type == NEO_TOKEN_TYPE_WORD) {
      node->right = neo_compiler_read_word(compiler);
    } else {
      char buf[1024];
      sprintf(buf, "extends' list cannot be empty. at\n\t%s:%d:%d",
              token->pos.filename, token->pos.line, token->pos.column);
      compiler->error = cstring_clone(buf);
      free_neo_ast(node);
      return NULL;
    }
  }
  return node;
}
static neo_ast neo_compiler_read_class_function(neo_compiler compiler) {
  neo_ast func_def = create_neo_ast(NEO_AST_TYPE_CLASS_FUNCTION, 0, 0, 0);
  neo_ast meta = neo_compiler_read_function_meta(compiler);
  if (!meta) {
    free_neo_ast(func_def);
    return NULL;
  }
  func_def->left = meta;
  neo_ast body = neo_compiler_read_block_statement(compiler);
  if (!body) {
    free_neo_ast(func_def);
    return NULL;
  }
  func_def->right = body;
  return func_def;
}
static neo_ast neo_compiler_read_class_field(neo_compiler compiler) {
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  if (neo_token_is(token, "@", NEO_TOKEN_TYPE_SYMBOL)) {
    neo_ast node = neo_compiler_read_decorator(compiler);
    if (!node) {
      return NULL;
    }
    node->right = neo_compiler_read_class_field(compiler);
    if (!node->right) {
      free_neo_ast(node);
      return NULL;
    }
    return node;
  }
  if (neo_token_is(token, "static", NEO_TOKEN_TYPE_KEYWORD)) {
    neo_compiler_next(compiler, skips_default);
    neo_token token = neo_compiler_read_token(compiler, skips_default);
    if (neo_token_is(token, "{", NEO_TOKEN_TYPE_SYMBOL)) {
      neo_ast node = neo_compiler_read_block_statement(compiler);
      node->type = NEO_AST_TYPE_CLASS_STATIC_BLOCK;
      return node;
    } else {
      neo_ast node = create_neo_ast(NEO_AST_TYPE_CLASS_STATIC, 0, 0, 0);
      node->right = neo_compiler_read_class_field(compiler);
      if (!node->right) {
        free_neo_ast(node);
        return NULL;
      }
      return node;
    }
  }
  neo_ast node = create_neo_ast(NEO_AST_TYPE_CLASS_FIELD, 0, 0, 0);
  if (neo_token_is(token, "get", NEO_TOKEN_TYPE_KEYWORD)) {
    node->type = NEO_AST_TYPE_CLASS_GET;
    neo_compiler_next(compiler, skips_default);
    node->left = neo_compiler_read_word(compiler);
    node->right = neo_compiler_read_class_function(compiler);
    if (!node->right) {
      free_neo_ast(node);
      return NULL;
    }
    return node;
  }
  if (neo_token_is(token, "set", NEO_TOKEN_TYPE_KEYWORD)) {
    node->type = NEO_AST_TYPE_CLASS_SET;
    neo_compiler_next(compiler, skips_default);
    node->left = neo_compiler_read_word(compiler);
    node->right = neo_compiler_read_class_function(compiler);
    if (!node->right) {
      free_neo_ast(node);
      return NULL;
    }
    return node;
  }
  if (neo_token_is(token, "[", NEO_TOKEN_TYPE_SYMBOL)) {
    neo_compiler_next(compiler, skips_default);
    node->type = NEO_AST_TYPE_CLASS_COMPUTED_FIELD;
    int8_t is_inline = compiler->is_inline;
    compiler->is_inline = 1;
    node->left = neo_compiler_read_expression(compiler);
    compiler->is_inline = is_inline;
    if (!node->left) {
      free_neo_ast(node);
      return NULL;
    }
    token = neo_compiler_read_token(compiler, skips_default);
    if (!neo_token_is(token, "]", NEO_TOKEN_TYPE_SYMBOL)) {
      char buf[1024];
      sprintf(buf, "']' expected. at\n\t%s:%d:%d", token->pos.filename,
              token->pos.line, token->pos.column);
      compiler->error = cstring_clone(buf);
      free_neo_ast(node);
      return NULL;
    }
    neo_compiler_next(compiler, skips_default);
  } else if (token->type == NEO_TOKEN_TYPE_WORD) {
    node->left = neo_compiler_read_word(compiler);
  } else if (token->type == NEO_TOKEN_TYPE_STRING) {
    node->type = NEO_AST_TYPE_CLASS_COMPUTED_FIELD;
    node->left = neo_compiler_read_string(compiler);
  }
  token = neo_compiler_read_token(compiler, skips_default);
  if (neo_token_is(token, "=", NEO_TOKEN_TYPE_SYMBOL)) {
    neo_compiler_next(compiler, skips_default);
    int8_t is_inline = compiler->is_inline;
    compiler->is_inline = 1;
    node->right = neo_compiler_read_expression(compiler);
    compiler->is_inline = is_inline;
    if (!node->right) {
      free_neo_ast(node);
      return NULL;
    }
  } else if (neo_token_is(token, "(", NEO_TOKEN_TYPE_SYMBOL)) {
    node->right = neo_compiler_read_class_function(compiler);
    if (!node->right) {
      free_neo_ast(node);
      return NULL;
    }
  }
  return node;
}
static neo_ast neo_compiler_read_class(neo_compiler compiler) {
  neo_ast node = create_neo_ast(NEO_AST_TYPE_CLASS, 0, 0, 0);
  node->level = -1;
  neo_compiler_next(compiler, skips_default);
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  neo_ast meta = neo_compiler_read_class_meta(compiler);
  if (!meta) {
    free_neo_ast(node);
    return NULL;
  }
  node->left = meta;
  token = neo_compiler_read_token(compiler, skips_default);
  if (!neo_token_is(token, "{", NEO_TOKEN_TYPE_SYMBOL)) {
    char buf[1024];
    sprintf(buf, "'{' expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    free_neo_ast(node);
    return NULL;
  }
  neo_compiler_next(compiler, skips_default);
  token = neo_compiler_read_token(compiler, skips_default);
  neo_ast items = node;
  if (!neo_token_is(token, "}", NEO_TOKEN_TYPE_SYMBOL)) {
    neo_ast field = NULL;
    for (;;) {
      neo_ast field = neo_compiler_read_class_field(compiler);
      if (!field) {
        free_neo_ast(node);
        return NULL;
      }
      token = neo_compiler_read_token(compiler, skips_default);
      if (neo_token_is(token, "}", NEO_TOKEN_TYPE_SYMBOL)) {
        items->right = field;
        break;
      } else {
        neo_ast list = create_neo_ast(NEO_AST_TYPE_PAIR, 0, 0, 0);
        list->left = field;
        items->right = list;
        items = list;
      }
    }
  }
  if (!neo_token_is(token, "}", NEO_TOKEN_TYPE_SYMBOL)) {
    char buf[1024];
    sprintf(buf, "'}' expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    free_neo_ast(node);
    return NULL;
  }
  neo_compiler_next(compiler, skips_default);
  return node;
}
static neo_ast neo_compiler_read_import(neo_compiler compiler) {
  neo_compiler_next(compiler, skips_default);
  neo_ast node = neo_compiler_read_function_call(compiler);
  if (!node) {
    return NULL;
  }
  node->type = NEO_AST_TYPE_IMPORT_CALL;
  return node;
}
static neo_ast neo_compiler_read_expression(neo_compiler compiler) {
  neo_ast root = NULL;
  int8_t is_completed = 0;
  while (compiler->position != neo_list_tail(compiler->tokens)) {
    neo_ast node = NULL;
    neo_token token = neo_compiler_read_token(compiler, skips_inline);
    neo_list_node current = compiler->position;
    if (!is_completed) {
      switch (token->type) {
      case NEO_TOKEN_TYPE_COMMENT:
        neo_compiler_skip(compiler, skips_default);
        break;
      case NEO_TOKEN_TYPE_END:
        neo_compiler_skip(compiler, skips_default);
        break;
      case NEO_TOKEN_TYPE_NUMBER: {
        node = neo_compiler_read_number(compiler);
        break;
      }
      case NEO_TOKEN_TYPE_STRING: {
        node = neo_compiler_read_string(compiler);
        break;
      }
      case NEO_TOKEN_TYPE_REGEX: {
        neo_ast node = neo_compiler_read_regex(compiler);
        break;
      }
      case NEO_TOKEN_TYPE_KEYWORD:
        if (neo_token_is(token, "true", NEO_TOKEN_TYPE_KEYWORD)) {
          node = create_neo_boolean_ast(1);
          neo_compiler_next(compiler, skips_default);
        } else if (neo_token_is(token, "false", NEO_TOKEN_TYPE_KEYWORD)) {
          node = create_neo_boolean_ast(0);
          neo_compiler_next(compiler, skips_default);
        } else if (neo_token_is(token, "null", NEO_TOKEN_TYPE_KEYWORD)) {
          node = create_neo_null_ast();
          neo_compiler_next(compiler, skips_default);
        } else if (neo_token_is(token, "undefined", NEO_TOKEN_TYPE_KEYWORD)) {
          node = create_neo_undefined_ast();
          neo_compiler_next(compiler, skips_default);
        } else if (neo_token_is(token, "function", NEO_TOKEN_TYPE_KEYWORD)) {
          node = neo_compiler_read_function_def(compiler);
        } else if (neo_token_is(token, "class", NEO_TOKEN_TYPE_KEYWORD)) {
          node = neo_compiler_read_class(compiler);
        } else if (neo_token_is(token, "typeof", NEO_TOKEN_TYPE_KEYWORD)) {
          node = neo_compiler_read_typeof(compiler);
        } else if (neo_token_is(token, "instanceof", NEO_TOKEN_TYPE_KEYWORD)) {
          node = neo_compiler_read_instanceof(compiler);
        } else if (neo_token_is(token, "new", NEO_TOKEN_TYPE_KEYWORD)) {
          node = neo_compiler_read_new(compiler);
        } else if (neo_token_is(token, "delete", NEO_TOKEN_TYPE_KEYWORD)) {
          node = neo_compiler_read_delete(compiler);
        } else if (neo_token_is(token, "async", NEO_TOKEN_TYPE_KEYWORD)) {
          node = neo_compiler_read_async(compiler);
        } else if (neo_token_is(token, "await", NEO_TOKEN_TYPE_KEYWORD)) {
          node = neo_compiler_read_await(compiler);
        } else if (neo_token_is(token, "yield", NEO_TOKEN_TYPE_KEYWORD)) {
          node = neo_compiler_read_yield(compiler);
        } else if (neo_token_is(token, "this", NEO_TOKEN_TYPE_KEYWORD)) {
          node = neo_compiler_read_this(compiler);
        } else if (neo_token_is(token, "super", NEO_TOKEN_TYPE_KEYWORD)) {
          node = neo_compiler_read_super(compiler);
        } else if (neo_token_is(token, "void", NEO_TOKEN_TYPE_KEYWORD)) {
          node = neo_compiler_read_void(compiler);
        } else if (neo_token_is(token, "import", NEO_TOKEN_TYPE_KEYWORD)) {
          node = neo_compiler_read_import(compiler);
        } else {
          char buf[1024];
          sprintf(buf, "Identifier expected. at\n\t%s:%d:%d",
                  token->pos.filename, token->pos.line, token->pos.column);
          compiler->error = cstring_clone(buf);
          node = NULL;
        }
        break;
      case NEO_TOKEN_TYPE_WORD: {
        neo_list_node current = compiler->position;
        neo_compiler_next(compiler, skips_inline);
        neo_token next = neo_compiler_read_token(compiler, skips_inline);
        compiler->position = current;
        if (neo_token_is(next, "=>", NEO_TOKEN_TYPE_SYMBOL)) {
          node = neo_compiler_read_lambda(compiler);
        } else {
          node = neo_compiler_read_word(compiler);
        }
        break;
      }
      case NEO_TOKEN_TYPE_SYMBOL: {
        if (neo_token_is(token, "{", NEO_TOKEN_TYPE_SYMBOL)) {
          node = neo_compiler_read_object(compiler);
        } else if (neo_token_is(token, "[", NEO_TOKEN_TYPE_SYMBOL)) {
          node = neo_compiler_read_array(compiler);
        } else if (neo_token_is(token, "(", NEO_TOKEN_TYPE_SYMBOL)) {
          node = neo_compiler_read_brackets(compiler);
        } else if (neo_token_is(token, ";", NEO_TOKEN_TYPE_SYMBOL)) {
          if (!root) {
            return create_neo_ast(NEO_AST_TYPE_EMPTY, 0, 0, 0);
          } else {
            char buf[1024];
            sprintf(buf, "Unexpected keyword or identifier. at\n\t%s:%d:%d",
                    token->pos.filename, token->pos.line, token->pos.column);
            compiler->error = cstring_clone(buf);
            node = NULL;
            break;
          }
        } else if (neo_token_is(token, "@", NEO_TOKEN_TYPE_SYMBOL)) {
          node = neo_compiler_read_decorator(compiler);
          token = neo_compiler_read_token(compiler, skips_default);
          if (node) {
            node->right = neo_compiler_read_class(compiler);
            if (!node->right) {
              free_neo_ast(node);
              node = NULL;
            }
          }
        } else {
          int level = 0;
          if ((neo_token_is(token, "+", NEO_TOKEN_TYPE_SYMBOL) ||
               neo_token_is(token, "-", NEO_TOKEN_TYPE_SYMBOL))) {
            level = 0;
          } else if (neo_token_is(token, "!", NEO_TOKEN_TYPE_SYMBOL)) {
            level = 0;
          } else if (neo_token_is(token, "]", NEO_TOKEN_TYPE_SYMBOL) ||
                     neo_token_is(token, "}", NEO_TOKEN_TYPE_SYMBOL) ||
                     neo_token_is(token, ")", NEO_TOKEN_TYPE_SYMBOL) ||
                     neo_token_is(token, ":", NEO_TOKEN_TYPE_SYMBOL)) {
            return root;
          } else {
            char buf[1024];
            sprintf(buf, "Declaration or statement expected. at\n\t%s:%d:%d",
                    token->pos.filename, token->pos.line, token->pos.column);
            compiler->error = cstring_clone(buf);
            return NULL;
          }
          node = neo_compiler_read_symbol(compiler);
          node->level = level;
        }
        break;
      }
      default:
        return root;
      }
    } else {
      switch (token->type) {
      case NEO_TOKEN_TYPE_END: {
        neo_list_node current = compiler->position;
        neo_token token = neo_compiler_read_token(compiler, skips_default);
        if (token->type != NEO_TOKEN_TYPE_SYMBOL ||
            neo_token_is(token, "!", NEO_TOKEN_TYPE_SYMBOL)) {
          compiler->position = current;
          return root;
        }
      } break;
      case NEO_TOKEN_TYPE_EOF:
        return root;
      case NEO_TOKEN_TYPE_KEYWORD: {
        if (neo_token_is(token, "in", NEO_TOKEN_TYPE_KEYWORD)) {
          node = create_neo_ast(NEO_AST_TYPE_EXPRESSION, token, 0, 0);
          node->level = 0;
          neo_compiler_next(compiler, skips_default);
        } else {
          int index = 0;
          for (; binary_operator_keywords[index] != 0; index++) {
            if (neo_token_is(token, binary_operator_keywords[index],
                             NEO_TOKEN_TYPE_KEYWORD)) {
              char buf[1024];
              sprintf(buf, "Unexpected keyword or identifier. at\n\t%s:%d:%d",
                      token->pos.filename, token->pos.line, token->pos.column);
              compiler->error = cstring_clone(buf);
              node = NULL;
              break;
            }
          }
          if (!binary_operator_keywords[index]) {
            return root;
          }
        }
      } break;
      case NEO_TOKEN_TYPE_SYMBOL: {
        if (neo_token_is(token, "[", NEO_TOKEN_TYPE_SYMBOL)) {
          node = neo_compiler_read_array_member(compiler);
        } else if (neo_token_is(token, "(", NEO_TOKEN_TYPE_SYMBOL)) {
          node = neo_compiler_read_function_call(compiler);
        } else if (neo_token_is(token, "?.", NEO_TOKEN_TYPE_SYMBOL)) {
          node = neo_compiler_read_optional(compiler);
        } else if (neo_token_is(token, ".", NEO_TOKEN_TYPE_SYMBOL)) {
          node = neo_compiler_read_member(compiler);
        } else if (neo_token_is(token, "?", NEO_TOKEN_TYPE_SYMBOL)) {
          node = neo_compiler_read_ternary(compiler);
        } else if (neo_token_is(token, ";", NEO_TOKEN_TYPE_SYMBOL)) {
          return root;
        } else {
          if (neo_token_is(token, ",", NEO_TOKEN_TYPE_SYMBOL) &&
              compiler->is_inline) {
            return root;
          }
          int level = 0;
          if (neo_token_is(token, "++", NEO_TOKEN_TYPE_SYMBOL) ||
              neo_token_is(token, "--", NEO_TOKEN_TYPE_SYMBOL)) {
            level = -1;
          } else {
            for (; symbols[level] != 0; level++) {
              int index = 0;
              for (; symbols[level][index] != 0; index++) {
                if (neo_token_is(token, symbols[level][index],
                                 NEO_TOKEN_TYPE_SYMBOL)) {
                  break;
                }
              }
              if (symbols[level][index] != 0) {
                break;
              }
            }
            if (!symbols[level]) {
              if (!root) {
                char buf[1024];
                sprintf(
                    buf, "Declaration or statement expected. at\n\t%s:%d:%d",
                    token->pos.filename, token->pos.line, token->pos.column);
                compiler->error = cstring_clone(buf);
                return NULL;
              }
              return root;
            }
          }
          node = neo_compiler_read_symbol(compiler);
          node->level = level;
        }
        break;
      }
      default: {
        char buf[1024];
        sprintf(buf, "Unexpected keyword or identifier. at\n\t%s:%d:%d",
                token->pos.filename, token->pos.line, token->pos.column);
        compiler->error = cstring_clone(buf);
        node = NULL;
      }
      }
    }
    if (node) {
      if (!neo_compiler_expression_append(&root, node)) {
        free_neo_ast(node);
        compiler->position = current;
        break;
      }
      is_completed = neo_compiler_expression_is_completed(root);
    } else {
      if (compiler->error) {
        if (root) {
          free_neo_ast(root);
        }
        return NULL;
      }
    }
  }
  return root;
}
static neo_ast neo_compiler_read_expression_statement(neo_compiler compiler) {
  neo_ast node = neo_compiler_read_expression(compiler);
  while (compiler->position != neo_list_tail(compiler->tokens)) {
    neo_token token = neo_compiler_read_token(compiler, skips_default);
    if (neo_token_is(token, ";", NEO_TOKEN_TYPE_SYMBOL)) {
      neo_compiler_next(compiler, skips_default);
    } else {
      break;
    }
  }
  return node;
}
static neo_ast neo_compiler_read_block_statement(neo_compiler compiler) {
  neo_compiler_next(compiler, skips_default);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_BLOCK_STATEMENT, 0, 0, 0);
  neo_ast statements = node;
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  if (*token->start != '}') {
    for (;;) {
      neo_ast item = neo_compiler_read_statement(compiler);
      if (!item) {
        free_neo_ast(node);
        return NULL;
      }
      token = neo_compiler_read_token(compiler, skips_default);
      if (token->type == NEO_TOKEN_TYPE_EOF) {
        break;
      }
      if (*token->start == '}') {
        statements->right = item;
        break;
      } else {
        neo_ast nodes = create_neo_ast(NEO_AST_TYPE_STATEMENTS, 0, 0, 0);
        nodes->left = item;
        statements->right = nodes;
        statements = nodes;
      }
    }
  }
  if (*token->start != '}') {
    char buf[1024];
    sprintf(buf, "'}' expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    free_neo_ast(node);
    return NULL;
  }
  neo_compiler_next(compiler, skips_default);
  return node;
}
static neo_ast neo_compiler_read_var(neo_compiler compiler) {
  neo_ast node = create_neo_ast(NEO_AST_TYPE_VAR, 0, 0, 0);
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  if (neo_token_is(token, "{", NEO_TOKEN_TYPE_SYMBOL)) {
    node->left = neo_compiler_read_object_destruct(compiler);
  } else if (neo_token_is(token, "[", NEO_TOKEN_TYPE_SYMBOL)) {
    node->left = neo_compiler_read_array_destruct(compiler);
  } else if (token->type == NEO_TOKEN_TYPE_WORD) {
    node->left = neo_compiler_read_word(compiler);
  } else {
    char buf[1024];
    sprintf(buf, "Variable declaration expected. at\n\t%s:%d:%d",
            token->pos.filename, token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    free_neo_ast(node);
    return NULL;
  }
  neo_list_node current = compiler->position;
  token = neo_compiler_read_token(compiler, skips_default);
  if (neo_token_is(token, "=", NEO_TOKEN_TYPE_SYMBOL)) {
    neo_compiler_next(compiler, skips_default);
    int8_t is_inline = compiler->is_inline;
    compiler->is_inline = 1;
    node->right = neo_compiler_read_expression(compiler);
    compiler->is_inline = is_inline;
    if (!node->right) {
      free_neo_ast(node);
      return NULL;
    }
  } else {
    compiler->position = current;
  }
  return node;
}
static neo_ast neo_compiler_read_var_statement(neo_compiler compiler) {
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  neo_compiler_next(compiler, skips_default);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_VAR_STATEMENT, token, 0, 0);
  neo_ast item = NULL;
  neo_ast items = node;
  for (;;) {
    item = neo_compiler_read_var(compiler);
    if (!item) {
      free_neo_ast(node);
      return NULL;
    }
    neo_list_node current = compiler->position;
    token = neo_compiler_read_token(compiler, skips_default);
    if (neo_token_is(token, ",", NEO_TOKEN_TYPE_SYMBOL)) {
      neo_ast pair = create_neo_ast(NEO_AST_TYPE_PAIR, 0, item, 0);
      items->right = pair;
      items = pair;
      neo_compiler_next(compiler, skips_default);
    } else if (neo_token_is(token, ";", NEO_TOKEN_TYPE_SYMBOL)) {
      items->right = item;
      neo_compiler_next(compiler, skips_default);
      break;
    } else if (token->type == NEO_TOKEN_TYPE_EOF) {
      items->right = item;
      neo_compiler_next(compiler, skips_default);
      break;
    } else {
      compiler->position = current;
      token = neo_compiler_read_token(compiler, skips_inline);
      if (token->type == NEO_TOKEN_TYPE_END) {
        items->right = item;
        neo_compiler_next(compiler, skips_inline);
        break;
      }
    }
  }
  return node;
}
static neo_ast neo_compiler_read_return_statement(neo_compiler compiler) {
  neo_compiler_next(compiler, skips_default);
  neo_token token = neo_compiler_read_token(compiler, skips_inline);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_RETURN, 0, 0, 0);
  if (token->type != NEO_TOKEN_TYPE_END && token->type != NEO_TOKEN_TYPE_EOF) {
    node->right = neo_compiler_read_expression_statement(compiler);
    if (!node->right) {
      free_neo_ast(node);
      return NULL;
    }
  }
  return node;
}
static neo_ast neo_compiler_read_throw_statement(neo_compiler compiler) {
  neo_compiler_next(compiler, skips_default);
  neo_token token = neo_compiler_read_token(compiler, skips_inline);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_THROW, 0, 0, 0);
  if (token->type != NEO_TOKEN_TYPE_END && token->type != NEO_TOKEN_TYPE_EOF) {
    node->right = neo_compiler_read_expression_statement(compiler);
    if (!node->right) {
      free_neo_ast(node);
      return NULL;
    }
  }
  return node;
}
static neo_ast neo_compiler_read_while_statement(neo_compiler compiler) {
  neo_compiler_next(compiler, skips_default);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_WHILE_STATEMENT, 0, 0, 0);
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  if (!neo_token_is(token, "(", NEO_TOKEN_TYPE_SYMBOL)) {
    char buf[1024];
    sprintf(buf, "'(' expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    free_neo_ast(node);
    return NULL;
  }
  neo_compiler_next(compiler, skips_default);
  int8_t is_inline = compiler->is_inline;
  compiler->is_inline = 1;
  node->left = neo_compiler_read_expression(compiler);
  compiler->is_inline = is_inline;
  if (!node->left) {
    free_neo_ast(node);
    return NULL;
  }
  token = neo_compiler_read_token(compiler, skips_default);
  if (!neo_token_is(token, ")", NEO_TOKEN_TYPE_SYMBOL)) {
    char buf[1024];
    sprintf(buf, "')' expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    free_neo_ast(node);
    return NULL;
  }
  neo_compiler_next(compiler, skips_default);
  node->right = neo_compiler_read_statement(compiler);
  if (!node->right) {
    free_neo_ast(node);
    return NULL;
  }
  return node;
}
static neo_ast neo_compiler_read_if_statement(neo_compiler compiler) {
  neo_ast node = create_neo_ast(NEO_AST_TYPE_IF_STATEMENT, 0, 0, 0);
  neo_compiler_next(compiler, skips_default);
  check_skip("(", NEO_TOKEN_TYPE_SYMBOL, compiler);
  node->left = neo_compiler_read_expression(compiler);
  if (!node->left) {
    free_neo_ast(node);
    return NULL;
  }
  check_skip(")", NEO_TOKEN_TYPE_SYMBOL, compiler);
  node->right = create_neo_ast(NEO_AST_TYPE_PAIR, 0, 0, 0);
  neo_ast body = node->right;
  body->left = neo_compiler_read_statement(compiler);
  if (!body->left) {
    free_neo_ast(node);
    return NULL;
  }
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  if (neo_token_is(token, "else", NEO_TOKEN_TYPE_KEYWORD)) {
    body->right = neo_compiler_read_statement(compiler);
    if (!body->right) {
      free_neo_ast(node);
      return NULL;
    }
  }
  return node;
}
static neo_ast neo_compiler_read_for_var(neo_compiler compiler) {
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  neo_compiler_next(compiler, skips_default);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_VAR_STATEMENT, token, 0, 0);
  neo_ast item = NULL;
  neo_ast items = node;
  for (;;) {
    item = neo_compiler_read_var(compiler);
    if (!item) {
      free_neo_ast(node);
      return NULL;
    }
    neo_list_node current = compiler->position;
    token = neo_compiler_read_token(compiler, skips_default);
    if (neo_token_is(token, "in", NEO_TOKEN_TYPE_KEYWORD) ||
        neo_token_is(token, "of", NEO_TOKEN_TYPE_KEYWORD)) {
      neo_ast var_node = create_neo_ast(NEO_AST_TYPE_EXPRESSION, token, 0, 0);
      node->left = item;
      var_node->left = node;
      neo_compiler_next(compiler, skips_default);
      int8_t is_inline = compiler->is_inline;
      compiler->is_inline = 1;
      var_node->right = neo_compiler_read_expression(compiler);
      compiler->is_inline = is_inline;
      if (!var_node->right) {
        free_neo_ast(node);
        return NULL;
      }
      return var_node;
    } else if (neo_token_is(token, ",", NEO_TOKEN_TYPE_SYMBOL)) {
      neo_ast pair = create_neo_ast(NEO_AST_TYPE_PAIR, 0, item, 0);
      items->right = pair;
      items = pair;
      neo_compiler_next(compiler, skips_default);
    } else if (neo_token_is(token, ";", NEO_TOKEN_TYPE_SYMBOL)) {
      items->right = item;
      neo_compiler_next(compiler, skips_default);
      break;
    } else if (token->type == NEO_TOKEN_TYPE_EOF) {
      items->right = item;
      neo_compiler_next(compiler, skips_default);
      break;
    } else {
      compiler->position = current;
      token = neo_compiler_read_token(compiler, skips_inline);
      if (token->type == NEO_TOKEN_TYPE_END) {
        items->right = item;
        neo_compiler_next(compiler, skips_inline);
        break;
      }
    }
  }
  return node;
}
static neo_ast neo_compiler_read_for_statement(neo_compiler compiler) {
  neo_ast node = create_neo_ast(NEO_AST_TYPE_FOR_STATEMENT, 0, 0, 0);
  node->left = create_neo_ast(NEO_AST_TYPE_FOR_META, 0, 0, 0);
  neo_ast meta = node->left;
  meta->right = create_neo_ast(NEO_AST_TYPE_PAIR, 0, 0, 0);
  neo_ast hook = meta->right;
  neo_compiler_next(compiler, skips_default);
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  if (!neo_token_is(token, "(", NEO_TOKEN_TYPE_SYMBOL)) {
    char buf[1024];
    sprintf(buf, "'(' expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    free_neo_ast(node);
    return NULL;
  }
  neo_compiler_next(compiler, skips_default);
  token = neo_compiler_read_token(compiler, skips_default);
  if (neo_token_is(token, "const", NEO_TOKEN_TYPE_KEYWORD) ||
      neo_token_is(token, "let", NEO_TOKEN_TYPE_KEYWORD) ||
      neo_token_is(token, "var", NEO_TOKEN_TYPE_KEYWORD)) {
    neo_ast init = neo_compiler_read_for_var(compiler);
    if (!init) {
      free_neo_ast(node);
      return NULL;
    }
    if (neo_token_is(init->operator, "in", NEO_TOKEN_TYPE_KEYWORD)) {
      node->type = NEO_AST_TYPE_FOR_IN_STATEMENT;
      free_neo_ast(meta);
      node->left = init;
    } else if (neo_token_is(init->operator, "of", NEO_TOKEN_TYPE_KEYWORD)) {
      node->type = NEO_AST_TYPE_FOR_OF_STATEMENT;
      free_neo_ast(meta);
      node->left = init;
    } else {
      hook->left = init;
    }
  } else {
    int8_t is_inline = compiler->is_inline;
    compiler->is_inline = 1;
    hook->left = neo_compiler_read_expression(compiler);
    compiler->is_inline = is_inline;
    if (!hook->left) {
      free_neo_ast(node);
      return NULL;
    }
    token = neo_compiler_read_token(compiler, skips_default);
    if (!neo_token_is(token, ";", NEO_TOKEN_TYPE_SYMBOL)) {
      char buf[1024];
      sprintf(buf, "';' expected. at\n\t%s:%d:%d", token->pos.filename,
              token->pos.line, token->pos.column);
      compiler->error = cstring_clone(buf);
      free_neo_ast(node);
      return NULL;
    }
    neo_compiler_next(compiler, skips_default);
    is_inline = compiler->is_inline;
    compiler->is_inline = 1;
    meta->left = neo_compiler_read_expression(compiler);
    compiler->is_inline = is_inline;
    if (!meta->left) {
      free_neo_ast(node);
      return NULL;
    }
    token = neo_compiler_read_token(compiler, skips_default);
    if (!neo_token_is(token, ";", NEO_TOKEN_TYPE_SYMBOL)) {
      char buf[1024];
      sprintf(buf, "';' expected. at\n\t%s:%d:%d", token->pos.filename,
              token->pos.line, token->pos.column);
      compiler->error = cstring_clone(buf);
      free_neo_ast(node);
      return NULL;
    }
    neo_compiler_next(compiler, skips_default);

    is_inline = compiler->is_inline;
    compiler->is_inline = 1;
    hook->right = neo_compiler_read_expression(compiler);
    compiler->is_inline = is_inline;
    if (!hook->right) {
      free_neo_ast(node);
      return NULL;
    }
  }
  token = neo_compiler_read_token(compiler, skips_default);
  if (!neo_token_is(token, ")", NEO_TOKEN_TYPE_SYMBOL)) {
    char buf[1024];
    sprintf(buf, "')' expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    free_neo_ast(node);
    return NULL;
  }
  neo_compiler_next(compiler, skips_default);
  node->right = neo_compiler_read_statement(compiler);
  if (!node->right) {
    free_neo_ast(node->right);
    return NULL;
  }
  return node;
}
static neo_ast neo_compiler_read_switch_case_body(neo_compiler compiler) {
  neo_ast node = NULL;
  neo_ast items = node;
  neo_ast item = NULL;
  for (;;) {
    neo_token token = neo_compiler_read_token(compiler, skips_default);
    if (neo_token_is(token, "case", NEO_TOKEN_TYPE_KEYWORD) ||
        neo_token_is(token, "default", NEO_TOKEN_TYPE_KEYWORD) ||
        neo_token_is(token, "}", NEO_TOKEN_TYPE_SYMBOL)) {
      if (item) {
        if (!node) {
          node = item;
        } else {
          if (!items) {
            items = create_neo_ast(NEO_AST_TYPE_PAIR, 0, 0, 0);
            items->left = node;
            node = items;
          }
          items->right = item;
        }
      }
      break;
    }
    if (item) {
      if (!node) {
        node = item;
        items = NULL;
      } else {
        if (!items) {
          items = create_neo_ast(NEO_AST_TYPE_PAIR, 0, 0, 0);
          items->left = node;
          node = items;
        }
        neo_ast list = create_neo_ast(NEO_AST_TYPE_PAIR, 0, 0, 0);
        list->left = item;
        items->right = list;
        items = list;
      }
    }
    item = neo_compiler_read_statement(compiler);
    if (!item) {
      if (node) {
        free_neo_ast(node);
        return NULL;
      }
    }
  }
  return node;
}
static neo_ast neo_compiler_read_switch_statement(neo_compiler compiler) {
  neo_ast node = create_neo_ast(NEO_AST_TYPE_SWITCH_STATEMENT, 0, 0, 0);
  neo_compiler_next(compiler, skips_default);
  check_skip("(", NEO_TOKEN_TYPE_SYMBOL, compiler);
  node->left = neo_compiler_read_expression(compiler);
  check_skip(")", NEO_TOKEN_TYPE_SYMBOL, compiler);
  check_skip("{", NEO_TOKEN_TYPE_SYMBOL, compiler);
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  if (!neo_token_is(token, "}", NEO_TOKEN_TYPE_SYMBOL)) {
    neo_ast items = node;
    neo_ast item = NULL;
    for (;;) {
      token = neo_compiler_read_token(compiler, skips_default);
      if (neo_token_is(token, "case", NEO_TOKEN_TYPE_KEYWORD)) {
        if (item) {
          neo_ast list = create_neo_ast(NEO_AST_TYPE_PAIR, 0, item, 0);
          items->right = list;
          items = list;
        }
        item = create_neo_ast(NEO_AST_TYPE_SWITCH_CASE, 0, 0, 0);
        neo_compiler_next(compiler, skips_default);
        int8_t is_inline = compiler->is_inline;
        compiler->is_inline = 1;
        item->left = neo_compiler_read_expression(compiler);
        compiler->is_inline = is_inline;
        if (!item->left) {
          free_neo_ast(item);
          free_neo_ast(node);
          return NULL;
        }
        check_skip(":", NEO_TOKEN_TYPE_SYMBOL, compiler);
        item->right = neo_compiler_read_switch_case_body(compiler);
        if (compiler->error) {
          free_neo_ast(item);
          free_neo_ast(node);
          return NULL;
        }
      } else if (neo_token_is(token, "default", NEO_TOKEN_TYPE_KEYWORD)) {
        if (item) {
          neo_ast list = create_neo_ast(NEO_AST_TYPE_PAIR, 0, item, 0);
          items->right = list;
          items = list;
        }
        neo_compiler_next(compiler, skips_default);
        check_skip(":", NEO_TOKEN_TYPE_SYMBOL, compiler);
        item->right = neo_compiler_read_switch_case_body(compiler);
      } else if (neo_token_is(token, "}", NEO_TOKEN_TYPE_SYMBOL)) {
        items->right = item;
        neo_compiler_next(compiler, skips_default);
        break;
      } else {
        char buf[1024];
        sprintf(buf, "Identifier expected. at\n\t%s:%d:%d", token->pos.filename,
                token->pos.line, token->pos.column);
        compiler->error = cstring_clone(buf);
        if (item) {
          free_neo_ast(item);
        }
        free_neo_ast(node);
        return NULL;
      }
    }
  }
  return node;
}
static neo_ast neo_compiler_read_try_statement(neo_compiler compiler) {
  neo_ast node = create_neo_ast(NEO_AST_TYPE_TRY_STATEMENT, 0, 0, 0);
  node->right = create_neo_ast(NEO_AST_TYPE_PAIR, 0, 0, 0);
  neo_compiler_next(compiler, skips_default);
  node->left = neo_compiler_read_block_statement(compiler);
  if (!node->left) {
    free_neo_ast(node);
    return NULL;
  }
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  check_skip("catch", NEO_TOKEN_TYPE_KEYWORD, compiler);
  node->right = create_neo_ast(NEO_AST_TYPE_PAIR, 0, 0, 0);
  node->right->left = create_neo_ast(NEO_AST_TYPE_CATCH_STATEMENT, 0, 0, 0);
  neo_ast catch_node = node->right->left;
  token = neo_compiler_read_token(compiler, skips_default);
  if (neo_token_is(token, "(", NEO_TOKEN_TYPE_SYMBOL)) {
    neo_compiler_next(compiler, skips_default);
    catch_node->left = neo_compiler_read_word(compiler);
    token = neo_compiler_read_token(compiler, skips_default);
    check_skip(")", NEO_TOKEN_TYPE_SYMBOL, compiler);
  }
  catch_node->right = neo_compiler_read_block_statement(compiler);
  if (!catch_node->right) {
    free_neo_ast(node);
    return NULL;
  }
  token = neo_compiler_read_token(compiler, skips_default);
  if (neo_token_is(token, "finally", NEO_TOKEN_TYPE_KEYWORD)) {
    neo_compiler_next(compiler, skips_default);
    node->right->right = neo_compiler_read_block_statement(compiler);
    if (!node->right->right) {
      free_neo_ast(node);
      return NULL;
    }
  }
  return node;
}
static neo_ast neo_compiler_read_import_statement(neo_compiler compiler) {
  return NULL;
}
static neo_ast neo_compiler_read_export_statement(neo_compiler compiler) {
  neo_ast node = create_neo_ast(NEO_AST_TYPE_EXPORT, 0, 0, 0);
  neo_compiler_next(compiler, skips_default);
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  if (neo_token_is(token, "default", NEO_TOKEN_TYPE_KEYWORD)) {
    neo_compiler_next(compiler, skips_default);
    node->type = NEO_AST_TYPE_EXPORT_DEFAULT;
    node->right = neo_compiler_read_expression(compiler);
  } else if (neo_token_is(token, "const", NEO_TOKEN_TYPE_KEYWORD) ||
             neo_token_is(token, "var", NEO_TOKEN_TYPE_KEYWORD) ||
             neo_token_is(token, "let", NEO_TOKEN_TYPE_KEYWORD)) {
    node->right = neo_compiler_read_var_statement(compiler);
  } else {
    node->right = neo_compiler_read_expression(compiler);
  }
  if (!node->right) {
    free_neo_ast(node);
    return NULL;
  }
  return node;
}
static neo_ast neo_compiler_read_with_statement(neo_compiler compiler) {
  neo_ast node = create_neo_ast(NEO_AST_TYPE_WITH_STATEMENT, 0, 0, 0);
  neo_compiler_next(compiler, skips_default);
  node->right = neo_compiler_read_block_statement(compiler);
  if (!node->right) {
    free_neo_ast(node);
  }
  return node;
}
static neo_ast neo_compiler_read_break_statement(neo_compiler compiler) {
  neo_compiler_next(compiler, skips_default);
  neo_token token = neo_compiler_read_token(compiler, skips_inline);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_BREAK, 0, 0, 0);
  if (token->type != NEO_TOKEN_TYPE_END && token->type != NEO_TOKEN_TYPE_EOF &&
      !neo_token_is(token, ";", NEO_TOKEN_TYPE_SYMBOL)) {
    node->right = neo_compiler_read_word(compiler);
    if (!node->right) {
      free_neo_ast(node);
      return NULL;
    }
  }
  return node;
}
static neo_ast neo_compiler_read_continue_statement(neo_compiler compiler) {
  neo_compiler_next(compiler, skips_default);
  neo_token token = neo_compiler_read_token(compiler, skips_inline);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_CONTINUE, 0, 0, 0);
  if (token->type != NEO_TOKEN_TYPE_END && token->type != NEO_TOKEN_TYPE_EOF &&
      !neo_token_is(token, ";", NEO_TOKEN_TYPE_SYMBOL)) {
    node->right = neo_compiler_read_word(compiler);
    if (!node->right) {
      free_neo_ast(node);
      return NULL;
    }
  }
  return node;
}
static neo_ast neo_compiler_read_label_statement(neo_compiler compiler) {
  neo_ast node = create_neo_ast(NEO_AST_TYPE_LABEL, 0, 0, 0);
  node->left = neo_compiler_read_word(compiler);
  check_skip(":", NEO_TOKEN_TYPE_SYMBOL, compiler);
  node->right = neo_compiler_read_statement(compiler);
  if (!node->right) {
    free_neo_ast(node);
    return NULL;
  }
  return node;
}
static neo_ast neo_compiler_read_statement(neo_compiler compiler) {
  neo_token token = neo_compiler_read_token(compiler, skips_default);
  if (neo_token_is(token, "{", NEO_TOKEN_TYPE_SYMBOL)) {
    return neo_compiler_read_block_statement(compiler);
  } else if (neo_token_is(token, "const", NEO_TOKEN_TYPE_KEYWORD) ||
             neo_token_is(token, "var", NEO_TOKEN_TYPE_KEYWORD) ||
             neo_token_is(token, "let", NEO_TOKEN_TYPE_KEYWORD)) {
    return neo_compiler_read_var_statement(compiler);
  } else if (neo_token_is(token, "return", NEO_TOKEN_TYPE_KEYWORD)) {
    return neo_compiler_read_return_statement(compiler);
  } else if (neo_token_is(token, "throw", NEO_TOKEN_TYPE_KEYWORD)) {
    return neo_compiler_read_throw_statement(compiler);
  } else if (neo_token_is(token, "while", NEO_TOKEN_TYPE_KEYWORD)) {
    return neo_compiler_read_while_statement(compiler);
  } else if (neo_token_is(token, "if", NEO_TOKEN_TYPE_KEYWORD)) {
    return neo_compiler_read_if_statement(compiler);
  } else if (neo_token_is(token, "switch", NEO_TOKEN_TYPE_KEYWORD)) {
    return neo_compiler_read_switch_statement(compiler);
  } else if (neo_token_is(token, "try", NEO_TOKEN_TYPE_KEYWORD)) {
    return neo_compiler_read_try_statement(compiler);
  } else if (neo_token_is(token, "for", NEO_TOKEN_TYPE_KEYWORD)) {
    return neo_compiler_read_for_statement(compiler);
  } else if (neo_token_is(token, "import", NEO_TOKEN_TYPE_KEYWORD)) {
    return neo_compiler_read_import_statement(compiler);
  } else if (neo_token_is(token, "export", NEO_TOKEN_TYPE_KEYWORD)) {
    return neo_compiler_read_export_statement(compiler);
  } else if (neo_token_is(token, "break", NEO_TOKEN_TYPE_KEYWORD)) {
    return neo_compiler_read_break_statement(compiler);
  } else if (neo_token_is(token, "continue", NEO_TOKEN_TYPE_KEYWORD)) {
    return neo_compiler_read_continue_statement(compiler);
  } else if (neo_token_is(token, "with", NEO_TOKEN_TYPE_KEYWORD)) {
    return neo_compiler_read_with_statement(compiler);
  }
  if (token->type == NEO_TOKEN_TYPE_WORD) {
    neo_list_node current = compiler->position;
    neo_compiler_next(compiler, skips_inline);
    token = neo_compiler_read_token(compiler, skips_inline);
    if (neo_token_is(token, ":", NEO_TOKEN_TYPE_SYMBOL)) {
      compiler->position = current;
      return neo_compiler_read_label_statement(compiler);
    }
    compiler->position = current;
  }
  int8_t is_inline = compiler->is_inline;
  compiler->is_inline = 0;
  neo_ast node = neo_compiler_read_expression_statement(compiler);
  compiler->is_inline = is_inline;
  return node;
}
static neo_ast neo_compiler_read_statements(neo_compiler compiler) {
  if (compiler->position == neo_list_tail(compiler->tokens)) {
    return NULL;
  }
  neo_ast statement = neo_compiler_read_statement(compiler);
  if (!statement) {
    return NULL;
  }
  neo_ast statements = neo_compiler_read_statements(compiler);
  if (compiler->error) {
    free_neo_ast(statement);
    return NULL;
  }
  if (!statements) {
    return statement;
  }
  return create_neo_ast(NEO_AST_TYPE_STATEMENTS, NULL, statement, statements);
}
static neo_ast neo_compiler_read_program(neo_compiler compiler) {
  neo_ast meta = NULL;
  // TODO: read program meta
  neo_ast statements = neo_compiler_read_statements(compiler);
  if (!statements) {
    return NULL;
  }
  return create_neo_ast(NEO_AST_TYPE_PROGRAM, NULL, meta, statements);
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
  return neo_compiler_read_program(compiler);
}
const char *neo_compiler_get_error(neo_compiler compiler) {
  return compiler->error;
}
neo_tokenizer neo_compiler_get_tokenizer(neo_compiler compiler) {
  return compiler->tokenizer;
}
