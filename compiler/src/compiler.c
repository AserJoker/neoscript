#include "compiler/include/compiler.h"
#include <stdlib.h>
struct _neo_compiler {};
neo_compiler create_neo_compiler() {
  neo_compiler compiler = malloc(sizeof(struct _neo_compiler));
  return compiler;
}
void free_neo_compiler(neo_compiler compiler) { free(compiler); }

neo_ast neo_compiler_compile(neo_compiler compiler, const char *source) {

}