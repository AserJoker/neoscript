#ifndef __NEO_COMPILER_TOKEN__
#define __NEO_COMPILER_TOKEN__
#include "common/include/cstring.h"
#include "common/include/list.h"
#include "position.h"
#include <stdint.h>
#include <string.h>
struct _neo_token {
  const cstring start;
  const cstring end;
  neo_position pos;
  uint32_t type;
};
typedef struct _neo_token *neo_token;
typedef struct _neo_tokenizer *neo_tokenizer;
typedef int8_t (*neo_tokenizer_fn)(neo_tokenizer tokenizer);
neo_token create_neo_token();
void free_neo_token(neo_token token);
neo_tokenizer create_neo_tokenizer();
void free_neo_tokenizer(neo_tokenizer tokenizer);
int8_t neo_tokenizer_parse(neo_tokenizer tokenizer, const cstring source,
                           const cstring filename);
neo_list neo_tokenizer_get_token_list(neo_tokenizer tokenizer);
const char *neo_tokenizer_get_error(neo_tokenizer tokenizer);
neo_position *neo_tokenizer_get_position(neo_tokenizer tokenizer);
void neo_tokenizer_set_context(neo_tokenizer tokenizer, int32_t key,
                               int32_t val);
int32_t neo_tokenizer_get_context(neo_tokenizer tokenizer, int32_t key);
void neo_tokenizer_add_tokenizer_fn(neo_tokenizer tokenizer,
                                    neo_tokenizer_fn fn);
static inline int8_t neo_token_is(neo_token token, const char *str,
                                  uint32_t type) {
  return token && token->type == type &&
         strncmp(token->start, str, token->end - token->start) == 0 &&
         *(str + (token->end - token->start)) == 0;
}
#endif