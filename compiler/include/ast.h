#ifndef __NEO_VM_AST__
#define __NEO_VM_AST__
#include "astdef.h"
#include "common/include/cstring.h"
#include "token.h"
#include <stdint.h>
typedef struct neo_ast_position {
  const cstring filename;
  uint32_t line;
  uint32_t column;
} neo_ast_position;
typedef struct _neo_ast *neo_ast;
struct _neo_ast {
  enum NEO_AST_TYPE type;
  int32_t level;
  union {
    cstring s_data;
    cstring s_symbol;
    cstring s_regex;
    double f_data;
    int8_t b_data;
  };
  struct {
    neo_ast left;
    neo_ast right;
    neo_token operator;
  };
};

uint32_t neo_ast_get_type(neo_ast self);
neo_ast create_neo_ast(uint32_t type, neo_token operator, neo_ast left,
                       neo_ast right);
void free_neo_ast(neo_ast ast);
neo_ast create_neo_boolean_ast(int8_t value);
neo_ast create_neo_number_ast(double value);
neo_ast create_neo_string_ast(const cstring value);
neo_ast create_neo_symbol_ast(const cstring value);
neo_ast create_neo_regex_ast(const cstring value);
neo_ast neo_ast_transform(neo_ast source, neo_ast (*)(neo_ast source));
#endif
