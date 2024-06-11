#include "compiler/include/compiler.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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
  if (!neo_compiler_compile(compiler, buf, "index.mjs")) {
    printf("%s\n", neo_compiler_get_error(compiler));
  }
  free(buf);
  free_neo_compiler(compiler);
  return 0;
}