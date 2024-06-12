#include "compiler/include/compiler.h"
#include "ast.h"
#include "astdef.h"
#include "common/include/cstring.h"
#include "common/include/list.h"
#include "token.h"
#include "tokendef.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
static neo_ast neo_compiler_read_expression(neo_compiler compiler) {
  while (compiler->position != neo_list_tail(compiler->tokens)) {
    neo_token token = neo_list_node_get(compiler->position);
    if (*token->start == ';') {
      compiler->position = neo_list_node_next(compiler->position);
    } else {
      break;
    }
  }
  neo_ast root = NULL;
  int8_t is_value = 1;
  while (compiler->position != neo_list_tail(compiler->tokens)) {
    neo_list_node current = compiler->position;
    neo_ast node = NULL;
    neo_token token = neo_list_node_get(compiler->position);
    switch (token->type) {
    case NEO_TOKEN_TYPE_COMMENT:
      compiler->position = neo_list_node_next(compiler->position);
      break;
    case NEO_TOKEN_TYPE_END:
      compiler->position = neo_list_node_next(compiler->position);
      break;
    case NEO_TOKEN_TYPE_NUMBER: {
      char *buf = malloc(token->end - token->start + 1);
      memset(buf, 0, token->end - token->start + 1);
      strncpy(buf, token->start, token->end - token->start);
      node = create_neo_number_ast(atof(buf));
      free(buf);
      compiler->position = neo_list_node_next(compiler->position);
      is_value = 0;
      break;
    }
    case NEO_TOKEN_TYPE_STRING: {
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
      node = create_neo_string_ast(buf);
      free(buf);
      compiler->position = neo_list_node_next(compiler->position);
      is_value = 0;
      break;
    }
    case NEO_TOKEN_TYPE_REGEX: {
      char *buf = malloc(token->end - token->start + 1);
      memset(buf, 0, token->end - token->start + 1);
      strncpy(buf, token->start, token->end - token->start);
      node = create_neo_regex_ast(buf);
      free(buf);
      compiler->position = neo_list_node_next(compiler->position);
      is_value = 0;
      break;
    }
    case NEO_TOKEN_TYPE_WORD: {
      if (strncmp(token->start, "true", 4) == 0) {
        node = create_neo_boolean_ast(1);
      } else if (strncmp(token->start, "false", 5) == 0) {
        node = create_neo_boolean_ast(0);
      } else if (strncmp(token->start, "null", 4) == 0) {
        node = create_neo_ast(NEO_AST_TYPE_NULL, NULL, NULL, NULL);
      } else if (strncmp(token->start, "function", 8) == 0) {
        // TODO: function def
      } else if (strncmp(token->start, "class", 4) == 0) {
        // TODO: class def
      } else {
        // TODO: keyword
        // TODO: function call
        char *buf = malloc(token->end - token->start + 1);
        memset(buf, 0, token->end - token->start + 1);
        strncpy(buf, token->start, token->end - token->start);
        node = create_neo_symbol_ast(buf);
        free(buf);
      }
      compiler->position = neo_list_node_next(compiler->position);
      is_value = 0;
      break;
    }
    case NEO_TOKEN_TYPE_SYMBOL: {
      if (*token->start == '{') {
        // TODO: object def
      } else if (*token->start == '[') {
        // TODO: array def|member
      } else if (*token->start == '(') {
        // TODO: lambda def or bracket
      } else if (*token->start == '.') {
        // TODO: member
      } else if (*token->start == '@') {
        // TODO: decorator& function def|class def
      } else if (*token->start == '?') {
        // TODO: triple
      } else {
        int level = 0;
        if (token->end - token->start == 1 &&
            (*token->start == '+' || *token->start == '-') && is_value) {
          level = -1;
        } else {
          for (; symbols[level] != 0; level++) {
            int index = 0;
            for (; symbols[level][index] != 0; index++) {
              if (strncmp(token->start, symbols[level][index],
                          strlen(symbols[level][index])) == 0) {
                break;
              }
            }
            if (symbols[level][index] != 0) {
              break;
            }
          }
          if (!symbols[level]) {
            return root;
          }
          is_value = 1;
        }
        char buf[8] = {0};
        strncpy(buf, token->start, token->end - token->start);
        node = create_neo_ast(NEO_AST_TYPE_EXPRESSION, token, NULL, NULL);
        node->level = level;
        compiler->position = neo_list_node_next(compiler->position);
      }
      break;
    }
    default:
      return root;
    }
    if (node) {
      if (!neo_compiler_expression_append(&root, node)) {
        free_neo_ast(node);
        break;
      }
    }
  }
  return root;
}
static neo_ast neo_compiler_read_statement(neo_compiler compiler) {
  return neo_compiler_read_expression(compiler);
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
  if (!statements) {
    return statement;
  }
  return create_neo_ast(NEO_AST_TYPE_STATEMENTS, NULL, statement, statements);
}

static neo_ast neo_compiler_read_program(neo_compiler compiler) {
  neo_ast meta = NULL;
  neo_ast statements = neo_compiler_read_statements(compiler);
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