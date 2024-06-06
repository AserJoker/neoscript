#include "compiler/include/token.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
struct _neo_token {
  uint32_t line;
  uint32_t column;
  const char *filename;
  uint32_t len;
  uint32_t type;
};
neo_token create_neo_token() {
  neo_token token = malloc(sizeof(struct _neo_token));
  memset(token, 0, sizeof(struct _neo_token));
  return token;
}