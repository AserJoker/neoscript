#include "compiler/include/token.h"
#include "common/include/cstring.h"
#include "common/include/imap.h"
#include "common/include/list.h"
#include "position.h"
#include "tokendef.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct _neo_tokenizer {
  neo_list tokens;
  neo_position pos;
  neo_imap context;
  neo_list tokenizer_handlers;
  uint32_t pair;
  int8_t value;
  char *error;
};
neo_token create_neo_token() {
  neo_token token = malloc(sizeof(struct _neo_token));
  memset(token, 0, sizeof(struct _neo_token));
  return token;
}
neo_tokenizer create_neo_tokenizer() {
  neo_tokenizer tokenizer = malloc(sizeof(struct _neo_tokenizer));
  tokenizer->tokens = create_neo_list(free);
  tokenizer->pair = 0;
  tokenizer->context = create_neo_imap(free);
  tokenizer->tokenizer_handlers = create_neo_list(NULL);
  tokenizer->error = NULL;
  tokenizer->value = 1;
  return tokenizer;
}
void free_neo_tokenizer(neo_tokenizer tokenizer) {
  if (tokenizer->error) {
    free(tokenizer->error);
  }
  free_neo_imap(tokenizer->context);
  free_neo_list(tokenizer->tokenizer_handlers);
  free_neo_list(tokenizer->tokens);
  free(tokenizer);
}
static cstring symbols[] = {
    "===", "!==", "<<=", ">>=", ">>>", "==", "!=", "&&", "||", ">=",
    "<=",  "+=",  "-=",  "*=",  "/=",  "%=", "&=", "|=", "^=", "++",
    "--",  ">>",  "<<",  "=>",  "+",   "-",  "*",  "/",  "%",  "&",
    "|",   "^",   "!",   "(",   ")",   "[",  "]",  "{",  "}",  ",",
    "?",   ";",   ":",   ".",   ">",   "<",  "@",  "=",  0};
static void neo_tokenizer_skip_white_space(neo_tokenizer tokenizer) {
  for (;;) {
    if (*tokenizer->pos.position == ' ' || *tokenizer->pos.position == '\t') {
      tokenizer->pos.position++;
      tokenizer->pos.column++;
      continue;
    }
    if (*tokenizer->pos.position == '\n') {
      neo_token token = create_neo_token();
      token->pos = tokenizer->pos;
      token->start = tokenizer->pos.position;
      token->end = token->start + 1;
      token->type = NEO_TOKEN_TYPE_END;
      neo_list_push(tokenizer->tokens, token);
      tokenizer->pos.position++;
      tokenizer->pos.line++;
      tokenizer->pos.column = 0;
      continue;
    }
    if (*tokenizer->pos.position == '\r') {
      tokenizer->pos.position++;
      continue;
    }
    break;
  }
}
static int8_t neo_tokenizer_read_symbol(neo_tokenizer tokenizer) {
  uint32_t index = 0;
  while (symbols[index] != 0) {
    if (strncmp(tokenizer->pos.position, symbols[index],
                strlen(symbols[index])) == 0) {
      if (cstring_compare(symbols[index], "{")) {
        tokenizer->pair++;
      }
      if (cstring_compare(symbols[index], "}")) {
        tokenizer->pair--;
        tokenizer->value = 0;
      }
      if (*symbols[index] == ']' || *symbols[index] == ')') {
        tokenizer->value = 0;
      }
      neo_token token = create_neo_token();
      token->start = tokenizer->pos.position;
      token->end = tokenizer->pos.position + strlen(symbols[index]);
      token->type = NEO_TOKEN_TYPE_SYMBOL;
      token->pos = tokenizer->pos;
      tokenizer->pos.position = token->end;
      tokenizer->pos.column += token->end - token->start;
      neo_list_push(tokenizer->tokens, token);
      tokenizer->value = 1;
      return 1;
    }
    index++;
  }
  return 0;
}
static int8_t neo_tokenizer_read_number(neo_tokenizer tokenizer) {
  if (*tokenizer->pos.position >= '0' && *tokenizer->pos.position <= '9') {
    neo_token token = create_neo_token();
    token->start = tokenizer->pos.position;
    token->end = tokenizer->pos.position;
    token->pos = tokenizer->pos;
    token->type = NEO_TOKEN_TYPE_NUMBER;
    token->end++;
    if (*token->end == 'x') {
      token->end++;
      while ((*token->end >= '0' && *token->end <= '9') ||
             (*token->end >= 'a' && *token->end <= 'f') ||
             (*token->end >= 'A' && *token->end <= 'F')) {
        token->end++;
      }
    } else if (*token->end == 'o') {
      token->end++;
      while ((*token->end >= '0' && *token->end <= '7')) {
        token->end++;
      }
    } else if (*token->end == 'b') {
      token->end++;
      while ((*token->end >= '0' && *token->end <= '1')) {
        token->end++;
      }
    } else {
      int8_t flag = 0;
      for (;;) {
        if (!flag && *token->end == '.') {
          flag = 1;
        } else if (*token->end <= '0' || *token->end >= '9') {
          break;
        }
        token->end++;
      }
      if (*token->end == 'e' || *token->end == 'E') {
        token->end++;
        if (*token->end == '+' || *token->end == '-') {
          token->end++;
        }
        while (*token->end >= '0' && *token->end <= '9') {
          token->end++;
        }
      }
    }
    tokenizer->pos.position = token->end;
    tokenizer->pos.column += token->end - token->start;
    neo_list_push(tokenizer->tokens, token);
    tokenizer->value = 0;
    return 1;
  } else if (*tokenizer->pos.position == '.' &&
             (*(tokenizer->pos.position + 1) >= '0' &&
              *(tokenizer->pos.position + 1) <= '9')) {
    neo_token token = create_neo_token();
    token->start = tokenizer->pos.position;
    token->end = tokenizer->pos.position;
    token->pos = tokenizer->pos;
    token->type = NEO_TOKEN_TYPE_NUMBER;
    token->end++;
    while ((*token->end >= '0' && *token->end <= '9')) {
      token->end++;
    }
    tokenizer->pos.position = token->end;
    tokenizer->pos.column += token->end - token->start;
    neo_list_push(tokenizer->tokens, token);
    tokenizer->value = 0;
    return 1;
  }
  return 0;
}
static int8_t neo_tokenizer_read_comment(neo_tokenizer tokenizer) {
  if (*tokenizer->pos.position == '/') {
    if (*(tokenizer->pos.position + 1) == '/') {
      neo_token token = create_neo_token();
      token->start = tokenizer->pos.position;
      token->end = tokenizer->pos.position;
      token->pos = tokenizer->pos;
      token->type = NEO_TOKEN_TYPE_COMMENT;
      token->end++;
      while (*token->end && *token->end != '\n') {
        token->end++;
      }
      tokenizer->pos.position = token->end;
      tokenizer->pos.column += token->end - token->start;
      neo_list_push(tokenizer->tokens, token);
      return 1;
    } else if (*(tokenizer->pos.position + 1) == '*') {
      neo_token token = create_neo_token();
      token->start = tokenizer->pos.position;
      token->end = tokenizer->pos.position;
      token->pos = tokenizer->pos;
      token->type = NEO_TOKEN_TYPE_COMMENT;
      token->end++;
      for (;;) {
        if (!*token->end) {
          return 0;
        }
        if (*token->end == '/' && *(token->end - 1) == '*') {
          token->end++;
          break;
        }
        token->end++;
      }
      tokenizer->pos.position = token->end;
      tokenizer->pos.column += token->end - token->start;
      neo_list_push(tokenizer->tokens, token);
      return 1;
    }
  }
  return 0;
}
static int8_t neo_tokenizer_read_word(neo_tokenizer tokenizer) {
  if ((*tokenizer->pos.position >= 'a' && *tokenizer->pos.position <= 'z') ||
      (*tokenizer->pos.position >= 'A' && *tokenizer->pos.position <= 'Z') ||
      (*tokenizer->pos.position == '_') || (*tokenizer->pos.position == '$')) {
    neo_token token = create_neo_token();
    token->start = tokenizer->pos.position;
    token->end = tokenizer->pos.position;
    token->pos = tokenizer->pos;
    token->type = NEO_TOKEN_TYPE_WORD;
    token->end++;
    while ((*token->end >= '0' && *token->end <= '9') ||
           (*token->end >= 'a' && *token->end <= 'z') ||
           (*token->end >= 'A' && *token->end <= 'Z') || *token->end == '_' ||
           *token->end == '_') {
      token->end++;
    }
    tokenizer->pos.position = token->end;
    tokenizer->pos.column += token->end - token->start;
    neo_list_push(tokenizer->tokens, token);
    tokenizer->value = 0;
    return 1;
  }
  return 0;
}
static int8_t neo_tokenizer_read_regex(neo_tokenizer tokenizer) {
  if (tokenizer->value && *tokenizer->pos.position == '/' &&
      *(tokenizer->pos.position + 1) != '*' &&
      *(tokenizer->pos.position + 1) != '/') {
    neo_token token = create_neo_token();
    token->start = tokenizer->pos.position;
    token->end = tokenizer->pos.position;
    token->pos = tokenizer->pos;
    token->type = NEO_TOKEN_TYPE_REGEX;
    token->end++;
    while (*token->end != '/' && *(token->end - 1) != '\\') {
      if (!*token->end) {
        return 0;
      }
      token->end++;
    }
    token->end++;
    while (*token->end == 'i' || *token->end == 'm' || *token->end == 'g') {
      token->end++;
    }
    tokenizer->pos.position = token->end;
    tokenizer->pos.column += token->end - token->start;
    neo_list_push(tokenizer->tokens, token);
    tokenizer->value = 0;
    return 1;
  }
  return 0;
}
static int8_t neo_tokenizer_read_string(neo_tokenizer tokenizer) {
  static uint32_t part_string_count = 0;
  if (*tokenizer->pos.position == '\'' || *tokenizer->pos.position == '\"') {
    neo_token token = create_neo_token();
    token->start = tokenizer->pos.position;
    token->end = tokenizer->pos.position;
    token->pos = tokenizer->pos;
    token->type = NEO_TOKEN_TYPE_STRING;
    token->end++;
    for (;;) {
      if (!*token->end) {
        return 0;
      }
      if (*token->end != *tokenizer->pos.position ||
          *(token->end - 1) == '\\') {
        token->end++;
      } else {
        break;
      }
    }
    token->end++;
    tokenizer->pos.position = token->end;
    tokenizer->pos.column += token->end - token->start;
    neo_list_push(tokenizer->tokens, token);
    tokenizer->value = 0;
    return 1;
  } else if (*tokenizer->pos.position == '`') {
    neo_token token = create_neo_token();
    token->start = tokenizer->pos.position;
    token->end = tokenizer->pos.position;
    token->pos = tokenizer->pos;
    token->end++;
    for (;;) {
      if (!*token->end) {
        return 0;
      }
      if (*token->end == '`' && *(token->end - 1) != '\\') {
        token->end++;
        token->type = NEO_TOKEN_TYPE_TEMPLATE_STRING;
        tokenizer->value = 0;
        break;
      }
      if (*token->end == '{' && *(token->end - 1) == '$' &&
          *(token->end - 2) != '\\') {
        token->type = NEO_TOKEN_TYPE_TEMPLATE_STRING_START;
        part_string_count++;
        token->end++;
        tokenizer->value = 1;
        break;
      }
      token->end++;
    }
    tokenizer->pos.position = token->end;
    tokenizer->pos.column += token->end - token->start;
    neo_list_push(tokenizer->tokens, token);
    return 1;
  } else if (*tokenizer->pos.position == '}' && part_string_count != 0 &&
             tokenizer->pair == 0) {
    neo_token token = create_neo_token();
    token->start = tokenizer->pos.position;
    token->end = tokenizer->pos.position;
    token->pos = tokenizer->pos;
    token->end++;
    for (;;) {
      if (*token->end == '`' && *(token->end - 1) != '\\') {
        token->end++;
        token->type = NEO_TOKEN_TYPE_TEMPLATE_STRING_END;
        tokenizer->value = 0;
        break;
      }
      if (*token->end == '{' && *(token->end - 1) == '$' &&
          *(token->end - 2) != '\\') {
        token->type = NEO_TOKEN_TYPE_TEMPLATE_STRING_PART;
        part_string_count++;
        token->end++;
        tokenizer->value = 1;
        break;
      }
      token->end++;
    }
    tokenizer->pos.position = token->end;
    tokenizer->pos.column += token->end - token->start;
    neo_list_push(tokenizer->tokens, token);
    return 1;
  }
  return 0;
}
static int8_t neo_tokenizer_read_token(neo_tokenizer tokenizer) {
  neo_list_node node = neo_list_head(tokenizer->tokenizer_handlers);
  while (node != neo_list_tail(tokenizer->tokenizer_handlers)) {
    neo_tokenizer_fn handler = neo_list_node_get(node);
    if (handler) {
      if (handler(tokenizer)) {
        return 1;
      }
    }
    node = neo_list_node_next(node);
  }
  if (neo_tokenizer_read_regex(tokenizer)) {
    return 1;
  }
  if (neo_tokenizer_read_string(tokenizer)) {
    return 1;
  }
  if (neo_tokenizer_read_word(tokenizer)) {
    return 1;
  }
  if (neo_tokenizer_read_comment(tokenizer)) {
    return 1;
  }
  if (neo_tokenizer_read_number(tokenizer)) {
    return 1;
  }
  if (neo_tokenizer_read_symbol(tokenizer)) {
    return 1;
  }
  return 0;
}
int8_t neo_tokenizer_parse(neo_tokenizer tokenizer, const cstring source,
                           const cstring filename) {
  tokenizer->pos.filename = filename;
  tokenizer->pos.column = 1;
  tokenizer->pos.line = 1;
  tokenizer->pos.source = source;
  tokenizer->pos.position = tokenizer->pos.source;
  while (*tokenizer->pos.position) {
    neo_tokenizer_skip_white_space(tokenizer);
    if (!*tokenizer->pos.position) {
      break;
    }
    if (!neo_tokenizer_read_token(tokenizer)) {
      tokenizer->error = malloc(1024);
      sprintf(tokenizer->error, "Invalid token:%c as %s:%d:%d",
              *tokenizer->pos.position, tokenizer->pos.filename,
              tokenizer->pos.line, tokenizer->pos.column);
      return 0;
    }
  }
  return 1;
}
neo_list neo_tokenizer_get_token_list(neo_tokenizer tokenizer) {
  return tokenizer->tokens;
}
const char *neo_tokenizer_get_error(neo_tokenizer tokenizer) {
  return tokenizer->error;
}

neo_position *neo_tokenizer_get_position(neo_tokenizer tokenizer) {
  return &tokenizer->pos;
}
void neo_tokenizer_set_context(neo_tokenizer tokenizer, int32_t key,
                               int32_t val) {
  int32_t *data = malloc(sizeof(int32_t));
  *data = val;
  neo_imap_set(tokenizer->context, key, data);
}
int32_t neo_tokenizer_get_context(neo_tokenizer tokenizer, int32_t key) {
  int32_t *data = neo_imap_get(tokenizer->context, key);
  if (data) {
    return *data;
  }
  return 0;
}
void neo_tokenizer_add_tokenizer_fn(neo_tokenizer tokenizer,
                                    neo_tokenizer_fn fn) {
  neo_list_unshift(tokenizer->tokenizer_handlers, fn);
}
