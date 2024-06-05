#include "engine/include/context.h"
#include "engine/include/runtime.h"
#include "vm/include/vm.h"
#include <stdint.h>
int main(int argc, char *argv[]) {
  neo_runtime rt = create_neo_runtime();
  neo_context ctx = create_neo_context(rt);
  neo_vm vm = create_neo_vm(ctx);
  free_neo_vm(vm);
  free_neo_context(ctx);
  free_neo_runtime(rt);
  return 0;
}