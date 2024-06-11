#ifndef __NEO_COMPILER_COMPILER__
#define __NEO_COMPILER_COMPILER__
#include "token.h"
#include "vm/include/ast/ast.h"
typedef struct _neo_compiler *neo_compiler;
neo_compiler create_neo_compiler();
void free_neo_compiler(neo_compiler compiler);
neo_ast neo_compiler_compile(neo_compiler compiler, const cstring source,
                             const cstring filename);
const char *neo_compiler_get_error(neo_compiler compiler);
neo_tokenizer neo_compiler_get_tokenizer(neo_compiler compiler);
#endif