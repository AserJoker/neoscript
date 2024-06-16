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
static cstring keywords[] = {
    "async",   "await",  "assert",    "break",    "case",
    "catch",   "class",  "const",     "continue", "debugger",
    "default", "delete", "do",        "else",     "export",
    "extends", "false",  "finally",   "for",      "function",
    "from",    "if",     "import",    "in",       "instanceof",
    "let",     "null",   "new",       "return",   "super",
    "switch",  "static", "this",      "throw",    "try",
    "true",    "typeof", "undefined", "var",      "void",
    "while",   "with",   "yield",     0};
static neo_ast neo_compiler_read_number(neo_compiler compiler) {
  neo_token token = neo_list_node_get(compiler->position);
  char *buf = malloc(token->end - token->start + 1);
  memset(buf, 0, token->end - token->start + 1);
  strncpy(buf, token->start, token->end - token->start);
  neo_ast node = create_neo_number_ast(atof(buf));
  free(buf);
  compiler->position = neo_list_node_next(compiler->position);
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
  compiler->position = neo_list_node_next(compiler->position);
  return node;
}
static neo_ast neo_compiler_read_regex(neo_compiler compiler) {
  neo_token token = neo_list_node_get(compiler->position);
  char *buf = malloc(token->end - token->start + 1);
  memset(buf, 0, token->end - token->start + 1);
  strncpy(buf, token->start, token->end - token->start);
  neo_ast node = create_neo_regex_ast(buf);
  free(buf);
  compiler->position = neo_list_node_next(compiler->position);
  return node;
}
static neo_ast neo_compiler_read_word(neo_compiler compiler) {
  neo_token token = neo_list_node_get(compiler->position);
  char *buf = malloc(token->end - token->start + 1);
  memset(buf, 0, token->end - token->start + 1);
  strncpy(buf, token->start, token->end - token->start);
  neo_ast node = create_neo_symbol_ast(buf);
  free(buf);
  compiler->position = neo_list_node_next(compiler->position);
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
    token = neo_list_node_get(current);
    char buf[1024];
    sprintf(buf, "')' expected. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    compiler->position = current;
    return NULL;
  }
  neo_list_node next = neo_list_node_next(pair);
  neo_token next_token = neo_list_node_get(next);
  if (next_token->type == NEO_TOKEN_TYPE_SYMBOL &&
      strncmp(next_token->start, "=>", 2) == 0) {
    // TODO: lambda
  }
  compiler->position = neo_list_node_next(compiler->position);
  int8_t is_inline = compiler->is_inline;
  compiler->is_inline = 0;
  neo_ast content = neo_compiler_read_expression(compiler);
  compiler->is_inline = is_inline;
  if (!content) {
    return NULL;
  }
  token = neo_list_node_get(compiler->position);
  if (!token || *token->start != ')') {
    token = neo_list_node_get(current);
    char buf[1024];
    sprintf(buf, "unexpect token. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    free_neo_ast(content);
    compiler->position = current;
    return NULL;
  }
  neo_ast node = create_neo_ast(NEO_AST_TYPE_BRACKETS, 0, content, 0);
  node->level = -1;
  compiler->position = neo_list_node_next(compiler->position);
  return node;
}
static neo_ast neo_compiler_read_symbol(neo_compiler compiler) {
  neo_token token = neo_list_node_get(compiler->position);
  char buf[8] = {0};
  strncpy(buf, token->start, token->end - token->start);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_EXPRESSION, token, NULL, NULL);
  compiler->position = neo_list_node_next(compiler->position);
  return node;
}
static neo_ast neo_compiler_read_function_call(neo_compiler compiler) {
  neo_list_node current = compiler->position;
  compiler->position = neo_list_node_next(compiler->position);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_CALL, 0, 0, NULL);
  neo_ast arg = NULL;
  neo_ast args = node;
  neo_token token = neo_list_node_get(compiler->position);
  if (!token) {
    token = neo_list_node_get(current);
    char buf[1024];
    sprintf(buf, "unexpect ')'. at\n\t%s:%d:%d", token->pos.filename,
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
        compiler->position = neo_list_node_next(compiler->position);
      } else if (token && *token->start == ')') {
        args->right = arg;
        break;
      } else {
        token = neo_list_node_get(current);
        char buf[1024];
        sprintf(buf, "unexpect ')'. at\n\t%s:%d:%d", token->pos.filename,
                token->pos.line, token->pos.column);
        compiler->error = cstring_clone(buf);
        free_neo_ast(node);
        compiler->position = current;
        return NULL;
      }
    }
  }
  compiler->position = neo_list_node_next(compiler->position);
  node->level = -1;
  return node;
}
static neo_ast neo_compiler_read_array(neo_compiler compiler) {
  neo_ast node = create_neo_ast(NEO_AST_TYPE_ARRAY, 0, 0, 0);
  neo_list_node current = compiler->position;
  compiler->position = neo_list_node_next(compiler->position);
  neo_token token = neo_list_node_get(compiler->position);
  if (!token) {
    token = neo_list_node_get(current);
    char buf[1024];
    sprintf(buf, "unexpect ']'. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    free_neo_ast(node);
    compiler->position = current;
    return NULL;
  }
  if (*token->start == ']') {
    compiler->position = neo_list_node_next(compiler->position);
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
      compiler->position = neo_list_node_next(compiler->position);
    } else if (token && *token->start == ']') {
      items->right = item;
      break;
    } else {
      token = neo_list_node_get(current);
      char buf[1024];
      sprintf(buf, "unexpect ']'. at\n\t%s:%d:%d", token->pos.filename,
              token->pos.line, token->pos.column);
      compiler->error = cstring_clone(buf);
      free_neo_ast(node);
      compiler->position = current;
      return NULL;
    }
  }
  compiler->position = neo_list_node_next(compiler->position);
  node->level = -1;
  return node;
}
static neo_ast neo_compiler_read_array_member(neo_compiler compiler) {
  neo_list_node current = compiler->position;
  compiler->position = neo_list_node_next(compiler->position);
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
  if (!token || *token->start != ']') {
    token = neo_list_node_get(current);
    char buf[1024];
    sprintf(buf, "unexpect ']'. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    compiler->position = current;
    free_neo_ast(field);
    return NULL;
  }
  compiler->position = neo_list_node_next(compiler->position);
  node = create_neo_ast(NEO_AST_TYPE_MEMBER, 0, 0, field);
  node->level = -1;
  return node;
}
static neo_ast neo_compiler_read_new(neo_compiler compiler) {
  neo_token token = neo_list_node_get(compiler->position);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_EXPRESSION, token, 0, 0);
  node->level = 0;
  compiler->position = neo_list_node_next(compiler->position);
  return node;
}
static neo_ast neo_compiler_read_delete(neo_compiler compiler) {
  neo_token token = neo_list_node_get(compiler->position);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_EXPRESSION, token, 0, 0);
  node->level = 0;
  compiler->position = neo_list_node_next(compiler->position);
  return node;
}
static neo_ast neo_compiler_read_typeof(neo_compiler compiler) {
  neo_token token = neo_list_node_get(compiler->position);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_EXPRESSION, token, 0, 0);
  node->level = 0;
  compiler->position = neo_list_node_next(compiler->position);
  return node;
}
static neo_ast neo_compiler_read_instanceof(neo_compiler compiler) {
  neo_token token = neo_list_node_get(compiler->position);
  neo_ast node = create_neo_ast(NEO_AST_TYPE_EXPRESSION, token, 0, 0);
  node->level = 0;
  compiler->position = neo_list_node_next(compiler->position);
  return node;
}
static neo_ast neo_compiler_read_member(neo_compiler compiler) {
  compiler->position = neo_list_node_next(compiler->position);
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
    compiler->position = neo_list_node_next(compiler->position);
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
    token = neo_list_node_get(current);
    char buf[1024];
    sprintf(buf, "Unexpect '('. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    compiler->position = current;
    free_neo_ast(meta);
    return NULL;
  }
  compiler->position = neo_list_node_next(compiler->position);
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
        compiler->position = neo_list_node_next(compiler->position);
      } else if (*token->start == ')') {
        args->right = arg;
        break;
      } else {
        if (arg) {
          free_neo_ast(arg);
        }
        token = neo_list_node_get(current);
        char buf[1024];
        sprintf(buf, "Unexpect ')'. at\n\t%s:%d:%d", token->pos.filename,
                token->pos.line, token->pos.column);
        compiler->error = cstring_clone(buf);
        compiler->position = current;
        free_neo_ast(meta);
        return NULL;
      }
    }
  }
  if (*token->start != ')') {
    token = neo_list_node_get(current);
    char buf[1024];
    sprintf(buf, "Unexpect ')'. at\n\t%s:%d:%d", token->pos.filename,
            token->pos.line, token->pos.column);
    compiler->error = cstring_clone(buf);
    compiler->position = current;
    free_neo_ast(meta);
    return NULL;
  }
  compiler->position = neo_list_node_next(compiler->position);
  return meta;
}
static neo_ast neo_compiler_read_function_def(neo_compiler compiler) {
  neo_list_node current = compiler->position;
  compiler->position = neo_list_node_next(compiler->position);
  neo_token token = neo_list_node_get(compiler->position);
  if (!token) {
    token = neo_list_node_get(current);
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
static neo_ast neo_compiler_read_expression(neo_compiler compiler) {
  neo_ast root = NULL;
  int8_t is_completed = 0;
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
    case NEO_TOKEN_TYPE_WORD: {
      if (strncmp(token->start, "true", 4) == 0 &&
          token->end - token->start == 4) {
        node = create_neo_boolean_ast(1);
        compiler->position = neo_list_node_next(compiler->position);
      } else if (strncmp(token->start, "false", 5) == 0 &&
                 token->end - token->start == 5) {
        node = create_neo_boolean_ast(0);
        compiler->position = neo_list_node_next(compiler->position);
      } else if (strncmp(token->start, "null", 4) == 0 &&
                 token->end - token->start == 4) {
        node = create_neo_ast(NEO_AST_TYPE_NULL, NULL, NULL, NULL);
        compiler->position = neo_list_node_next(compiler->position);
      } else if (strncmp(token->start, "function", 8) == 0 &&
                 token->end - token->start == 8) {
        node = neo_compiler_read_function_def(compiler);
      } else if (strncmp(token->start, "class", 5) == 0 &&
                 token->end - token->start == 5) {
        // TODO: class def
      } else if (strncmp(token->start, "typeof", 6) == 0 &&
                 token->end - token->start == 6) {
        node = neo_compiler_read_typeof(compiler);
      } else if (strncmp(token->start, "instanceof", 10) == 0 &&
                 token->end - token->start == 10) {
        node = neo_compiler_read_instanceof(compiler);
      } else if (strncmp(token->start, "new", 3) == 0 &&
                 token->end - token->start == 3) {
        node = neo_compiler_read_new(compiler);
      } else if (strncmp(token->start, "delete", 6) == 0 &&
                 token->end - token->start == 6) {
        node = neo_compiler_read_delete(compiler);
      } else if (strncmp(token->start, "async", 5) == 0 &&
                 token->end - token->start == 5) {
        // TODO: async def
      } else if (strncmp(token->start, "await", 5) == 0 &&
                 token->end - token->start == 5) {
        // TODO: await def
      } else {
        for (int index = 0; keywords[index] != 0; index++) {
          int32_t len = strlen(keywords[index]);
          if (strncmp(keywords[index], token->start, len) == 0 &&
              token->end - token->start == len) {
            return root;
          }
        }
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
        // TODO: triple
        // TODO: optional
      } else {
        if (*token->start == ',' && compiler->is_inline) {
          return root;
        }
        int level = 0;
        if (token->end - token->start == 1 &&
            (*token->start == '+' || *token->start == '-') && !is_completed) {
          level = 0;
        } else if (is_completed && (strncmp(token->start, "++", 2) == 0 ||
                                    strncmp(token->start, "--", 2) == 0)) {
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
  compiler->position = neo_list_node_next(compiler->position);
  neo_ast nodes = neo_compiler_read_statements(compiler);
  compiler->position = neo_list_node_next(compiler->position);
  return create_neo_ast(NEO_AST_TYPE_BLOCK_STATEMENT, 0, nodes, 0);
}
static neo_ast neo_compiler_read_statement(neo_compiler compiler) {
  while (compiler->position != neo_list_tail(compiler->tokens)) {
    neo_token token = neo_list_node_get(compiler->position);
    if (*token->start == ';') {
      compiler->position = neo_list_node_next(compiler->position);
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
