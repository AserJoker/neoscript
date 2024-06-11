#ifndef __NEO_COMPILER_POSITION__
#define __NEO_COMPILER_POSITION__
#include <stdint.h>
typedef struct _neo_position {
  const char *filename;
  const char *source;
  const char *position;
  uint32_t line;
  uint32_t column;
} neo_position;
#endif