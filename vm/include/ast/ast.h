#ifndef __NEO_VM_AST__
#define __NEO_VM_AST__
#include <stdint.h>
typedef struct _neo_ast *neo_ast;
struct _neo_ast {
  uint32_t type;
  union {
    const char *s_data;
    const char *s_symbol;
    int64_t i_data;
    double f_data;
    struct {
      neo_ast left;
      neo_ast right;
      uint32_t data;
    };
  };
};

uint32_t neo_ast_get_type(neo_ast self);
neo_ast create_neo_ast(uint32_t type, uint32_t data, neo_ast left,
                       neo_ast right);
void free_neo_ast(neo_ast ast);
neo_ast create_neo_integer_ast(int64_t value);
neo_ast create_neo_float_ast(double value);
neo_ast create_neo_string_ast(const char *value);
neo_ast create_neo_symbol_ast(const char *value);
neo_ast neo_ast_transform(neo_ast source, neo_ast (*)(neo_ast source));
#endif