#ifndef __NEO_COMPILER_TOKEN__
#define __NEO_COMPILER_TOKEN__
#include <stdint.h>
typedef struct _neo_token *neo_token;
typedef neo_token (*neo_tokenizer_fn)(char *source, int8_t isvalue);
neo_token create_neo_token();
void free_neo_token(neo_token token);
#endif