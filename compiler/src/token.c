#include "compiler/include/token.h"
#include "common/include/cstring.h"
#include "common/include/list.h"
#include "tokendef.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct _neo_tokenizer {
  neo_list tokens;
  const cstring source;
  const cstring position;
  const cstring filename;
  uint32_t line;
  uint32_t column;
  uint32_t pair;
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
  tokenizer->error = NULL;
  return tokenizer;
}
void free_neo_tokenizer(neo_tokenizer tokenizer) {
  if (tokenizer->error) {
    free(tokenizer->error);
  }
  free_neo_list(tokenizer->tokens);
  free(tokenizer);
}
static cstring symbols[] = {
    "===", "!==", "<<=", ">>=", "==", "!=", "&&", "||", ">=", "<=", "+=", "-=",
    "*=",  "/=",  "%=",  "&=",  "|=", "^=", "++", "--", ">>", "<<", "=>", "+",
    "-",   "*",   "/",   "%",   "&",  "|",  "^",  "!",  "(",  ")",  "[",  "]",
    "{",   "}",   ",",   ";",   "?",  ":",  ".",  ">",  "<",  "@",  "=",  0};
static void neo_tokenizer_skip_white_space(neo_tokenizer tokenizer) {
  for (;;) {
    if (*tokenizer->position == ' ' || *tokenizer->position == '\t') {
      tokenizer->position++;
      tokenizer->column++;
      continue;
    }
    if (*tokenizer->position == '\n') {
      tokenizer->position++;
      tokenizer->line++;
      tokenizer->column = 0;
      continue;
    }
    if (*tokenizer->position == '\r') {
      tokenizer->position++;
      continue;
    }
    break;
  }
}
static int8_t neo_tokenizer_read_symbol(neo_tokenizer tokenizer) {
  uint32_t index = 0;
  while (symbols[index] != 0) {
    if (strncmp(tokenizer->position, symbols[index], strlen(symbols[index])) ==
        0) {
      if (cstring_compare(symbols[index], "{")) {
        tokenizer->pair++;
      }
      if (cstring_compare(symbols[index], "}")) {
        tokenizer->pair--;
      }
      neo_token token = create_neo_token();
      token->filename = tokenizer->filename;
      token->start = tokenizer->position;
      token->end = tokenizer->position + strlen(symbols[index]);
      token->type = NEO_TOKEN_TYPE_SYMBOL;
      token->line = tokenizer->line;
      token->column = tokenizer->column;
      tokenizer->position = token->end;
      tokenizer->column += token->end - token->start;
      neo_list_push(tokenizer->tokens, token);
      return 1;
    }
    index++;
  }
  return 0;
}
static int8_t neo_tokenizer_read_number(neo_tokenizer tokenizer) {
  if (*tokenizer->position >= '0' && *tokenizer->position <= '9') {
    neo_token token = create_neo_token();
    token->start = tokenizer->position;
    token->end = tokenizer->position;
    token->filename = tokenizer->filename;
    token->column = tokenizer->column;
    token->line = tokenizer->line;
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
    } else if (*token->end == 'e') {
      token->end++;
      if (*token->end == '+') {
        token->end++;
      }
      while (*token->end >= '0' && *token->end <= '9') {
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
    }
    tokenizer->position = token->end;
    tokenizer->column += token->end - token->start;
    neo_list_push(tokenizer->tokens, token);
    return 1;
  } else if (*tokenizer->position == '.' &&
             (*(tokenizer->position + 1) >= '0' &&
              *(tokenizer->position + 1) <= '9')) {
    neo_token token = create_neo_token();
    token->start = tokenizer->position;
    token->end = tokenizer->position;
    token->filename = tokenizer->filename;
    token->column = tokenizer->column;
    token->line = tokenizer->line;
    token->type = NEO_TOKEN_TYPE_NUMBER;
    token->end++;
    while ((*token->end >= '0' && *token->end <= '9')) {
      token->end++;
    }
    tokenizer->position = token->end;
    tokenizer->column += token->end - token->start;
    neo_list_push(tokenizer->tokens, token);
    return 1;
  }
  return 0;
}
static int8_t neo_tokenizer_read_comment(neo_tokenizer tokenizer) {
  if (*tokenizer->position == '/') {
    if (*(tokenizer->position + 1) == '/') {
      neo_token token = create_neo_token();
      token->start = tokenizer->position;
      token->end = tokenizer->position;
      token->filename = tokenizer->filename;
      token->column = tokenizer->column;
      token->line = tokenizer->line;
      token->type = NEO_TOKEN_TYPE_COMMENT;
      token->end++;
      while (*token->end && *token->end != '\n') {
        token->end++;
      }
      tokenizer->position = token->end;
      tokenizer->column += token->end - token->start;
      neo_list_push(tokenizer->tokens, token);
      return 1;
    } else if (*(tokenizer->position + 1) == '*') {
      neo_token token = create_neo_token();
      token->start = tokenizer->position;
      token->end = tokenizer->position;
      token->filename = tokenizer->filename;
      token->column = tokenizer->column;
      token->line = tokenizer->line;
      token->type = NEO_TOKEN_TYPE_COMMENT;
      token->end++;
      for (;;) {
        if (*token->end == '/' && *(token->end - 1) == '*') {
          token->end++;
          break;
        }
        token->end++;
      }
      tokenizer->position = token->end;
      tokenizer->column += token->end - token->start;
      neo_list_push(tokenizer->tokens, token);
      return 1;
    }
  }
  return 0;
}
static int8_t neo_tokenizer_read_word(neo_tokenizer tokenizer) {
  if ((*tokenizer->position >= 'a' && *tokenizer->position <= 'z') ||
      (*tokenizer->position >= 'A' && *tokenizer->position <= 'Z') ||
      (*tokenizer->position == '_') || (*tokenizer->position == '$')) {
    neo_token token = create_neo_token();
    token->start = tokenizer->position;
    token->end = tokenizer->position;
    token->filename = tokenizer->filename;
    token->column = tokenizer->column;
    token->line = tokenizer->line;
    token->type = NEO_TOKEN_TYPE_WORD;
    token->end++;
    while ((*token->end >= '0' && *token->end <= '9') ||
           (*token->end >= 'a' && *token->end <= 'z') ||
           (*token->end >= 'A' && *token->end <= 'Z') || *token->end == '_' ||
           *token->end == '_') {
      token->end++;
    }
    tokenizer->position = token->end;
    tokenizer->column += token->end - token->start;
    neo_list_push(tokenizer->tokens, token);
    return 1;
  }
  return 0;
}
static int8_t neo_tokenizer_read_regex(neo_tokenizer tokenizer) {
  if (*tokenizer->position == '/' && *(tokenizer->position + 1) != '*' &&
      *(tokenizer->position + 1) != '/') {

    neo_token token = create_neo_token();
    token->start = tokenizer->position;
    token->end = tokenizer->position;
    token->filename = tokenizer->filename;
    token->column = tokenizer->column;
    token->line = tokenizer->line;
    token->type = NEO_TOKEN_TYPE_REGEX;
    token->end++;
    while (*token->end != '/' && *(token->end - 1) != '\\') {
      token->end++;
    }
    token->end++;
    while (*token->end == 'i' || *token->end == 'm' || *token->end == 'g') {
      token->end++;
    }
    tokenizer->position = token->end;
    tokenizer->column += token->end - token->start;
    neo_list_push(tokenizer->tokens, token);
    return 1;
  }
  return 0;
}
static int8_t neo_tokenizer_read_string(neo_tokenizer tokenizer) {
  static uint32_t part_string_count = 0;
  if (*tokenizer->position == '\'' || *tokenizer->position == '\"') {
    neo_token token = create_neo_token();
    token->start = tokenizer->position;
    token->end = tokenizer->position;
    token->filename = tokenizer->filename;
    token->column = tokenizer->column;
    token->line = tokenizer->line;
    token->type = NEO_TOKEN_TYPE_STRING;
    token->end++;
    while (*token->end != *tokenizer->position || *(token->end - 1) == '\\') {
      token->end++;
    }
    token->end++;
    tokenizer->position = token->end;
    tokenizer->column += token->end - token->start;
    neo_list_push(tokenizer->tokens, token);
    return 1;
  } else if (*tokenizer->position == '`') {
    neo_token token = create_neo_token();
    token->start = tokenizer->position;
    token->end = tokenizer->position;
    token->filename = tokenizer->filename;
    token->column = tokenizer->column;
    token->line = tokenizer->line;
    token->end++;
    for (;;) {
      if (*token->end == '`' && *(token->end - 1) != '\\') {
        token->end++;
        token->type = NEO_TOKEN_TYPE_TEMPLATE_STRING;
        break;
      }
      if (*token->end == '{' && *(token->end - 1) == '$' &&
          *(token->end - 2) != '\\') {
        token->type = NEO_TOKEN_TYPE_TEMPLATE_STRING_START;
        part_string_count++;
        token->end++;
        break;
      }
      token->end++;
    }
    tokenizer->position = token->end;
    tokenizer->column += token->end - token->start;
    neo_list_push(tokenizer->tokens, token);
    return 1;
  } else if (*tokenizer->position == '}' && part_string_count != 0 &&
             tokenizer->pair == 0) {
    neo_token token = create_neo_token();
    token->start = tokenizer->position;
    token->end = tokenizer->position;
    token->filename = tokenizer->filename;
    token->column = tokenizer->column;
    token->line = tokenizer->line;
    token->end++;
    for (;;) {
      if (*token->end == '`' && *(token->end - 1) != '\\') {
        token->end++;
        token->type = NEO_TOKEN_TYPE_TEMPLATE_STRING_END;
        break;
      }
      if (*token->end == '{' && *(token->end - 1) == '$' &&
          *(token->end - 2) != '\\') {
        token->type = NEO_TOKEN_TYPE_TEMPLATE_STRING_PART;
        part_string_count++;
        token->end++;
        break;
      }
      token->end++;
    }
    tokenizer->position = token->end;
    tokenizer->column += token->end - token->start;
    neo_list_push(tokenizer->tokens, token);
    return 1;
  }
  return 0;
}
static int8_t neo_tokenizer_read_token(neo_tokenizer tokenizer) {
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
  tokenizer->filename = filename;
  tokenizer->column = 0;
  tokenizer->line = 0;
  tokenizer->source = source;
  tokenizer->position = tokenizer->source;
  while (*tokenizer->position) {
    neo_tokenizer_skip_white_space(tokenizer);
    if (!*tokenizer->position) {
      break;
    }
    if (!neo_tokenizer_read_token(tokenizer)) {
      tokenizer->error = malloc(1024);
      sprintf(tokenizer->error, "Invalid token:%c as %s:%d:%d",
              *tokenizer->position, tokenizer->filename, tokenizer->line,
              tokenizer->column);
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