#ifndef __NEO_COMPILER_TOKEN__
#define __NEO_COMPILER_TOKEN__
#include "common/include/cstring.h"
#include "common/include/list.h"
#include <stdint.h>
struct _neo_token {
  const cstring start;
  const cstring end;
  uint32_t line;
  uint32_t column;
  const cstring filename;
  uint32_t type;
};
typedef struct _neo_token *neo_token;
typedef struct _neo_tokenizer *neo_tokenizer;
neo_token create_neo_token();
void free_neo_token(neo_token token);
neo_tokenizer create_neo_tokenizer();
void free_neo_tokenizer(neo_tokenizer tokenizer);
int8_t neo_tokenizer_parse(neo_tokenizer tokenizer, const cstring source,
                           const cstring filename);
neo_list neo_tokenizer_get_token_list(neo_tokenizer tokenizer);
const char *neo_tokenizer_get_error(neo_tokenizer tokenizer);
#endif