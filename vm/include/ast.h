#ifndef __NEO_VM_AST__
#define __NEO_VM_AST__
#include <stdint.h>
#define NEO_AST_TYPE_EOF 0
#define NEO_AST_TYPE_INTEGER 1
#define NEO_AST_TYPE_FLOAT 2
#define NEO_AST_TYPE_STRING 3
#define NEO_AST_TYPE_SYMBOL 4

typedef struct _neo_ast *neo_ast;
neo_ast *neo_ast_get_children(neo_ast self);
uint32_t neo_ast_get_type(neo_ast self);
neo_ast create_neo_ast(uint32_t type, void *data, neo_ast children[]);
neo_ast create_neo_integer_ast(int64_t value);
neo_ast create_neo_float_ast(double value);
neo_ast create_neo_string_ast(const char *value);
neo_ast create_neo_symbol_ast(const char *value);
#endif