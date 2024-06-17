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
             node->type == NEO_AST_TYPE_MEMBER) {
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

static void neo_compiler_next(neo_compiler compiler) {
  compiler->position = neo_list_node_next(compiler->position);
  while (compiler->position != neo_list_tail(compiler->tokens)) {
    neo_token token = neo_list_node_get(compiler->position);
    if (token->type == NEO_TOKEN_TYPE_END) {
      compiler->position = neo_list_node_next(compiler->position);
    } else {
      break;
    }
  }
}
static neo_token neo_compiler_next_token(neo_compiler compiler) {
  neo_list_node current = compiler->position;
  current = neo_list_node_next(current);
  neo_token token = neo_list_node_get(current);
  while (token->type == NEO_TOKEN_TYPE_END) {
    current = neo_list_node_next(current);
    token = neo_list_node_get(current);
  }
  return token;
}
static cstring symbol_0[] = {"++", "--", "!", 0};
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
static neo_ast neo_compiler_read_number(neo_compiler compiler) {
  neo_token token = neo_list_node_get(compiler->position);
  char *buf = malloc(token->end - token->start + 1);
  memset(buf, 0, token->end - token->start + 1);
  strncpy(buf, token->start, token->end - token->start);
  neo_ast node = create_neo_number_ast(atof(buf));
  free(buf);
  neo_compiler_next(compiler);
  return node;
}
static neo_ast neo_compiler_read_string(neo_compiler compiler) {
  neo_token token = neo_list_node_get(compiler->position);
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
  neo_compiler_next(compiler);
  return node;
}
static neo_ast neo_compiler_read_regex(neo_compiler compiler) {
  neo_token token = neo_list_node_get(compiler->position);
  char *buf = malloc(token->end - token->start + 1);
  memset(buf, 0, token->end - token->start + 1);
  strncpy(buf, token->start, token->end - token->start);
  neo_ast node = create_neo_regex_ast(buf);
  free(buf);
  neo_compiler_next(compiler);
  return node;
}
static neo_ast neo_compiler_read_word(neo_compiler compiler) {
  neo_token token = neo_list_node_get(compiler->position);
  char *buf = malloc(token->end - token->start + 1);
  memset(buf, 0, token->end - token->start + 1);
  strncpy(buf, token->start, token->end - token->start);
  neo_ast node = create_neo_symbol_ast(buf);
  free(buf);
  neo_compiler_next(compiler);
  return node;
}
static neo_ast neo_compiler_read_brackets(neo_compiler compiler) {
  neo_token token = neo_list_node_get(compiler->position);
  neo_list_node pair = neo_list_node_next(compiler->position);
  neo_list_node current = compiler->position;
  while (pair != neo_list_tail(compiler->tokens)) {
    neo_token token = neo_list_node_get(pair);
    if (token->type == NEO_TOKEN_TYPE_SYMBOL && *token->start == ')') {
      break;
    }
    pair = neo_list_node_next(pair);
  }
  if (pair == neo_list_tail(compiler->tokens)) {
    token = neo_list_node_get(pair);
    char buf[1024];
    sprintf(buf, "')' expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    compiler->position = current;
    return NULL;
  }
  neo_token next_token =
      neo_list_node_get(neo_list_node_next(compiler->position));
  if (next_token->type == NEO_TOKEN_TYPE_SYMBOL &&
      neo_token_is(next_token, "=>")) {
    return neo_compiler_read_lambda(compiler);
  }
  neo_compiler_next(compiler);
  int8_t is_inline = compiler->is_inline;
  compiler->is_inline = 0;
  neo_ast content = neo_compiler_read_expression(compiler);
  compiler->is_inline = is_inline;
  if (!content) {
    return NULL;
  }
  token = neo_list_node_get(compiler->position);
  if (token->type == NEO_TOKEN_TYPE_EOF || *token->start != ')') {
    char buf[1024];
    sprintf(buf, "')' expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    free_neo_ast(content);
    compiler->position = current;
    return NULL;
  }
  neo_ast node = create_neo_ast(NEO_AST_TYPE_BRACKETS, 0, content, 0);
  node->level = -1;
  neo_compiler_next(compiler);
  return node;
}
static neo_ast neo_compiler_read_symbol(neo_compiler compiler) {
  neo_token token = neo_list_node_get(compiler->position);
  char buf[8] = {0};
  strncpy(buf, token->start, token->end - token->start);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_EXPRESSION, token, NULL, NULL);
  neo_compiler_next(compiler);
  return node;
}
static neo_ast neo_compiler_read_function_call(neo_compiler compiler) {
  neo_list_node current = compiler->position;
  neo_compiler_next(compiler);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_CALL, 0, 0, NULL);
  neo_ast arg = NULL;
  neo_ast args = node;
  neo_token token = neo_list_node_get(compiler->position);
  if (token->type == NEO_TOKEN_TYPE_EOF) {
    char buf[1024];
    sprintf(buf, "')' expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    free_neo_ast(node);
    compiler->position = current;
    return NULL;
  }
  if (*token->start != ')') {
    for (;;) {
      int8_t is_inline = compiler->is_inline;
      compiler->is_inline = 1;
      arg = neo_compiler_read_expression(compiler);
      compiler->is_inline = is_inline;
      if (!arg) {
        free_neo_ast(node);
        compiler->position = current;
        return NULL;
      }
      token = neo_list_node_get(compiler->position);
      if (token && *token->start == ',') {
        neo_ast next = create_neo_ast(NEO_AST_TYPE_LIST, 0, arg, 0);
        args->right = next;
        args = next;
        neo_compiler_next(compiler);
      } else if (token && *token->start == ')') {
        args->right = arg;
        break;
      } else {
        char buf[1024];
        sprintf(buf, "')' expected. at\n\t%s:%d:%d", token->pos.filename,
                token->pos.line, token->pos.column);
        compiler->error = cstring_clone(buf);
        free_neo_ast(node);
        compiler->position = current;
        return NULL;
      }
    }
  }
  neo_compiler_next(compiler);
  node->level = -1;
  return node;
}
static neo_ast neo_compiler_read_array(neo_compiler compiler) {
  neo_ast node = create_neo_ast(NEO_AST_TYPE_ARRAY, 0, 0, 0);
  neo_list_node current = compiler->position;
  neo_compiler_next(compiler);
  neo_token token = neo_list_node_get(compiler->position);
  if (token->type == NEO_TOKEN_TYPE_EOF) {
    char buf[1024];
    sprintf(buf, "']' expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    free_neo_ast(node);
    compiler->position = current;
    return NULL;
  }
  if (*token->start == ']') {
    neo_compiler_next(compiler);
    return node;
  }
  neo_ast items = node;
  neo_ast item = NULL;
  for (;;) {
    int8_t is_inline = compiler->is_inline;
    compiler->is_inline = 1;
    item = neo_compiler_read_expression(compiler);
    compiler->is_inline = is_inline;
    if (!item) {
      free_neo_ast(node);
      compiler->position = current;
      return NULL;
    }
    token = neo_list_node_get(compiler->position);
    if (token && *token->start == ',') {
      neo_ast next = create_neo_ast(NEO_AST_TYPE_LIST, 0, item, 0);
      items->right = next;
      items = next;
      neo_compiler_next(compiler);
    } else if (token && *token->start == ']') {
      items->right = item;
      break;
    } else {
      char buf[1024];
      sprintf(buf, "']' expected. at\n\t%s:%d:%d", token->pos.filename,
              token->pos.line, token->pos.column);
      compiler->error = cstring_clone(buf);
      free_neo_ast(node);
      compiler->position = current;
      return NULL;
    }
  }
  neo_compiler_next(compiler);
  node->level = -1;
  return node;
}
static neo_ast neo_compiler_read_array_member(neo_compiler compiler) {
  neo_list_node current = compiler->position;
  neo_compiler_next(compiler);
  neo_ast node = NULL;
  int8_t is_inline = compiler->is_inline;
  compiler->is_inline = 0;
  neo_ast field = neo_compiler_read_expression(compiler);
  compiler->is_inline = is_inline;
  if (!field) {
    compiler->position = current;
    return NULL;
  }
  neo_token token = neo_list_node_get(compiler->position);
  if (token->type == NEO_TOKEN_TYPE_EOF || *token->start != ']') {
    char buf[1024];
    sprintf(buf, "']' expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    compiler->position = current;
    free_neo_ast(field);
    return NULL;
  }
  neo_compiler_next(compiler);
  node = create_neo_ast(NEO_AST_TYPE_MEMBER, 0, 0, field);
  node->level = -1;
  return node;
}
static neo_ast neo_compiler_read_new(neo_compiler compiler) {
  neo_token token = neo_list_node_get(compiler->position);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_EXPRESSION, token, 0, 0);
  node->level = 0;
  neo_compiler_next(compiler);
  return node;
}
static neo_ast neo_compiler_read_await(neo_compiler compiler) {
  neo_token token = neo_list_node_get(compiler->position);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_EXPRESSION, token, 0, 0);
  node->level = 0;
  neo_compiler_next(compiler);
  return node;
}
static neo_ast neo_compiler_read_delete(neo_compiler compiler) {
  neo_token token = neo_list_node_get(compiler->position);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_EXPRESSION, token, 0, 0);
  node->level = 0;
  neo_compiler_next(compiler);
  return node;
}
static neo_ast neo_compiler_read_typeof(neo_compiler compiler) {
  neo_token token = neo_list_node_get(compiler->position);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_EXPRESSION, token, 0, 0);
  node->level = 0;
  neo_compiler_next(compiler);
  return node;
}
static neo_ast neo_compiler_read_instanceof(neo_compiler compiler) {
  neo_token token = neo_list_node_get(compiler->position);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_EXPRESSION, token, 0, 0);
  node->level = 0;
  neo_compiler_next(compiler);
  return node;
}
static neo_ast neo_compiler_read_member(neo_compiler compiler) {
  neo_compiler_next(compiler);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_MEMBER, 0, 0, 0);
  node->right = neo_compiler_read_word(compiler);
  node->level = -1;
  return node;
}
static neo_ast neo_compiler_read_arg_def(neo_compiler compiler) {
  neo_ast node = create_neo_ast(NEO_AST_TYPE_ARG_DEF, 0, 0, 0);
  node->left = neo_compiler_read_word(compiler);
  neo_token token = neo_list_node_get(compiler->position);
  if (*token->start == '=') {
    neo_compiler_next(compiler);
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
  neo_token token = neo_list_node_get(compiler->position);
  neo_list_node current = compiler->position;
  neo_ast meta = create_neo_ast(NEO_AST_TYPE_FUNCTION_META, 0, 0, 0);
  if (token->type == NEO_TOKEN_TYPE_WORD) {
    meta->left = neo_compiler_read_word(compiler);
    token = neo_list_node_get(compiler->position);
  }
  if (*token->start != '(') {
    char buf[1024];
    sprintf(buf, "'(' expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    compiler->position = current;
    free_neo_ast(meta);
    return NULL;
  }
  neo_compiler_next(compiler);
  token = neo_list_node_get(compiler->position);
  if (*token->start != ')') {
    neo_ast args = meta;
    neo_ast arg = NULL;
    for (;;) {
      arg = neo_compiler_read_arg_def(compiler);
      if (!arg) {
        free_neo_ast(meta);
        return NULL;
      }
      token = neo_list_node_get(compiler->position);
      if (*token->start == ',') {
        neo_ast next = create_neo_ast(NEO_AST_TYPE_LIST, 0, arg, 0);
        args->right = next;
        args = next;
        neo_compiler_next(compiler);
      } else if (*token->start == ')') {
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
        compiler->position = current;
        free_neo_ast(meta);
        return NULL;
      }
    }
  }
  if (*token->start != ')') {
    char buf[1024];
    sprintf(buf, "'(' expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    compiler->position = current;
    free_neo_ast(meta);
    return NULL;
  }
  neo_compiler_next(compiler);
  return meta;
}
static neo_ast neo_compiler_read_function_def(neo_compiler compiler) {
  neo_list_node current = compiler->position;
  neo_compiler_next(compiler);
  neo_token token = neo_list_node_get(compiler->position);
  if (token->type == NEO_TOKEN_TYPE_EOF) {
    char buf[1024];
    sprintf(buf, "Identifier expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    compiler->position = current;
    return NULL;
  }
  neo_ast node = create_neo_ast(NEO_AST_TYPE_FUNCTION, 0, 0, 0);
  node->level = -1;
  if (*token->start == '*') {
    node->type = NEO_AST_TYPE_GENERATOR_FUNCTION;
  }
  neo_ast meta = neo_compiler_read_function_meta(compiler);
  if (!meta) {
    free_neo_ast(node);
    return meta;
  }
  node->left = meta;
  neo_ast body = neo_compiler_read_block_statement(compiler);
  if (!body) {
    free_neo_ast(node);
    compiler->position = current;
    return NULL;
  }
  node->right = body;
  return node;
}
static neo_ast neo_compiler_read_lambda(neo_compiler compiler) {
  neo_token token = neo_list_node_get(compiler->position);
  neo_list_node current = compiler->position;
  neo_ast node = create_neo_ast(NEO_AST_TYPE_LAMBDA, 0, 0, 0);
  node->level = -1;
  if (*token->start == '(') {
    neo_ast meta = neo_compiler_read_function_meta(compiler);
    if (!meta) {
      compiler->position = current;
      free_neo_ast(node);
      return NULL;
    }
    node->left = meta;
    token = neo_list_node_get(compiler->position);
  } else if (token->type == NEO_TOKEN_TYPE_WORD) {
    neo_ast meta = create_neo_ast(NEO_AST_TYPE_FUNCTION_META, 0, 0, 0);
    node->left = meta;
    meta->right = create_neo_ast(NEO_AST_TYPE_ARG_DEF, 0, 0, 0);
    meta->right->left = neo_compiler_read_word(compiler);
    token = neo_list_node_get(compiler->position);
  } else {
    char buf[1024];
    sprintf(buf, "Unexpect token. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    compiler->position = current;
    free_neo_ast(node);
    return NULL;
  }
  if (!neo_token_is(token, "=>")) {
    char buf[1024];
    sprintf(buf, "'=>' expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    compiler->position = current;
    free_neo_ast(node);
    return NULL;
  }
  neo_compiler_next(compiler);
  token = neo_list_node_get(compiler->position);
  neo_ast body = NULL;
  if (*token->start == '{') {
    body = neo_compiler_read_block_statement(compiler);
  } else {
    int8_t is_inline = compiler->is_inline;
    compiler->is_inline = 1;
    body = neo_compiler_read_expression(compiler);
    compiler->is_inline = is_inline;
  }
  if (!body) {
    compiler->position = current;
    free_neo_ast(node);
    return NULL;
  }
  node->right = body;
  return node;
}
static neo_ast neo_compiler_read_async(neo_compiler compiler) {
  neo_compiler_next(compiler);
  neo_token token = neo_list_node_get(compiler->position);
  if (neo_token_is(token, "function")) {
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
  neo_compiler_next(compiler);
  int8_t is_inline = compiler->is_inline;
  compiler->is_inline = 1;
  neo_ast truth = neo_compiler_read_expression(compiler);
  if (!truth) {
    return NULL;
  }
  compiler->is_inline = 1;
  neo_token token = neo_list_node_get(compiler->position);
  if (*token->start != ':') {
    char buf[1024];
    sprintf(buf, "':' expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    free_neo_ast(truth);
    return NULL;
  }
  neo_compiler_next(compiler);
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
  neo_compiler_next(compiler);
  neo_compiler_next(compiler);
  neo_token token = neo_list_node_get(compiler->position);
  if (*token->start == '[') {
    neo_ast node = neo_compiler_read_array_member(compiler);
    node->type = NEO_AST_TYPE_OPTIONAL_MEMBER;
    return node;
  } else if (*token->start == '(') {
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
static neo_ast neo_compiler_read_expression(neo_compiler compiler) {
  neo_ast root = NULL;
  int8_t is_completed = 0;
  while (compiler->position != neo_list_tail(compiler->tokens)) {
    neo_list_node current = compiler->position;
    neo_ast node = NULL;
    neo_token token = neo_list_node_get(compiler->position);
    switch (token->type) {
    case NEO_TOKEN_TYPE_COMMENT:
      neo_compiler_next(compiler);
      break;
    case NEO_TOKEN_TYPE_END:
      neo_compiler_next(compiler);
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
      if (neo_token_is(token, "true")) {
        node = create_neo_boolean_ast(1);
        neo_compiler_next(compiler);
      } else if (neo_token_is(token, "false")) {
        node = create_neo_boolean_ast(0);
        neo_compiler_next(compiler);
      } else if (neo_token_is(token, "null")) {
        node = create_neo_ast(NEO_AST_TYPE_NULL, NULL, NULL, NULL);
        neo_compiler_next(compiler);
      } else if (neo_token_is(token, "function")) {
        node = neo_compiler_read_function_def(compiler);
      } else if (neo_token_is(token, "class")) {
        // TODO: class def
      } else if (neo_token_is(token, "typeof")) {
        node = neo_compiler_read_typeof(compiler);
      } else if (neo_token_is(token, "instanceof")) {
        node = neo_compiler_read_instanceof(compiler);
      } else if (neo_token_is(token, "new")) {
        node = neo_compiler_read_new(compiler);
      } else if (neo_token_is(token, "delete")) {
        node = neo_compiler_read_delete(compiler);
      } else if (neo_token_is(token, "async")) {
        node = neo_compiler_read_async(compiler);
      } else if (neo_token_is(token, "await")) {
        node = neo_compiler_read_await(compiler);
      } else {
        return root;
      }
      break;
    case NEO_TOKEN_TYPE_WORD: {
      neo_token next =
          neo_list_node_get(neo_list_node_next(compiler->position));
      if (next && neo_token_is(next, "=>")) {
        node = neo_compiler_read_lambda(compiler);
      } else {
        node = neo_compiler_read_word(compiler);
      }
      break;
    }
    case NEO_TOKEN_TYPE_SYMBOL: {
      if (*token->start == '{') {
        // TODO: object def
      } else if (*token->start == '[') {
        if (!is_completed) {
          node = neo_compiler_read_array(compiler);
        } else {
          node = neo_compiler_read_array_member(compiler);
        }
      } else if (*token->start == '(') {
        if (is_completed) {
          node = neo_compiler_read_function_call(compiler);
        } else {
          node = neo_compiler_read_brackets(compiler);
        }
      } else if (*token->start == '.') {
        node = neo_compiler_read_member(compiler);
      } else if (*token->start == '@') {
        // TODO: decorator& function def|class def
      } else if (*token->start == '?') {
        neo_token next = neo_compiler_next_token(compiler);
        if (*next->start == '.') {
          node = neo_compiler_read_optional(compiler);
        } else {
          node = neo_compiler_read_ternary(compiler);
        }
      } else {
        if (*token->start == ',' && compiler->is_inline) {
          return root;
        }
        int level = 0;
        if (token->end - token->start == 1 &&
            (*token->start == '+' || *token->start == '-') && !is_completed) {
          level = 0;
        } else if (is_completed &&
                   (neo_token_is(token, "++") || neo_token_is(token, "--"))) {
          level = -1;
        } else {
          for (; symbols[level] != 0; level++) {
            int index = 0;
            for (; symbols[level][index] != 0; index++) {
              if (neo_token_is(token, symbols[level][index])) {
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
              sprintf(buf, "Declaration or statement expected. at\n\t%s:%d:%d",
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
    default:
      return root;
    }
    if (node) {
      if (!neo_compiler_expression_append(&root, node)) {
        compiler->position = current;
        free_neo_ast(node);
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
static neo_ast neo_compiler_read_block_statement(neo_compiler compiler) {
  neo_compiler_next(compiler);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_BLOCK_STATEMENT, 0, 0, 0);
  neo_ast statements = node;
  neo_token token = neo_list_node_get(compiler->position);
  if (*token->start != '}') {
    for (;;) {
      neo_ast node = neo_compiler_read_statement(compiler);
      if (!node) {
        free_neo_ast(node);
        return NULL;
      }
      token = neo_list_node_get(compiler->position);
      if (token->type == NEO_TOKEN_TYPE_EOF) {
        break;
      }
      if (*token->start == '}') {
        statements->right = node;
        break;
      } else {
        neo_ast nodes = create_neo_ast(NEO_AST_TYPE_STATEMENTS, 0, 0, 0);
        nodes->left = node;
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
  neo_compiler_next(compiler);
  return node;
}
static neo_ast neo_compiler_read_statement(neo_compiler compiler) {
  while (compiler->position != neo_list_tail(compiler->tokens)) {
    neo_token token = neo_list_node_get(compiler->position);
    if (*token->start == ';') {
      neo_compiler_next(compiler);
    } else {
      break;
    }
  }
  neo_token token = neo_list_node_get(compiler->position);
  if (*token->start == '{') {
    return neo_compiler_read_block_statement(compiler);
  }
  int8_t is_inline = compiler->is_inline;
  compiler->is_inline = 0;
  neo_ast node = neo_compiler_read_expression(compiler);
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
