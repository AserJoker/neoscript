#include "compiler/include/ast.h"
#include "compiler/include/compiler.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, cstring argv[]) {
  FILE *fp = fopen("./index.mjs", "r");
  char *buf = NULL;
  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  buf = malloc(size + 1);
  buf[size] = 0;
  fseek(fp, 0, SEEK_SET);
  fread(buf, size, 1, fp);
  fclose(fp);
  neo_compiler compiler = create_neo_compiler();
  neo_ast root = neo_compiler_compile(compiler, buf, "index.mjs");
  free(buf);
  free_neo_compiler(compiler);
  return 0;
}
