
#include "common/include/cstring.h"
#include "engine/include/context.h"
#include "engine/include/runtime.h"
#include "engine/include/type.h"
#include <assert.h>
#include <stdlib.h>
#define TEST_DEMO_TYPE 999
typedef struct _test_demo_type_impl {
  cstring str;
} *test_demo_type;
int flag = 0;
void neo_demo_type_init(void *target, void *source, void *arg) {
  test_demo_type dst = (test_demo_type)target;
  test_demo_type src = (test_demo_type)source;
  dst->str = cstring_clone(src->str);
  flag = *(int *)arg;
}
void neo_demo_type_dispose(void *target, void *arg) {
  test_demo_type dst = (test_demo_type)target;
  free(dst->str);
  flag = *(int *)arg;
}
neo_atom neo_demo_type_copy(neo_atom source, void *arg) {
  flag = *(int *)arg;
  return source;
}
int main(int argc, cstring argv[]) {
  neo_runtime rt = create_neo_runtime();
  int flag_init = 1;
  int flag_dispose = 2;
  int flag_copy = 3;
  neo_type_hook hook = {neo_demo_type_init,    &flag_init,
                        neo_demo_type_dispose, &flag_dispose,
                        neo_demo_type_copy,    &flag_copy};
  neo_type demo_type = create_neo_type(
      TEST_DEMO_TYPE, sizeof(struct _test_demo_type_impl), &hook);
  neo_runtime_define_type(rt, demo_type);
  neo_context ctx = create_neo_context(rt);
  struct _test_demo_type_impl impl = {"hello world"};
  neo_value val = neo_context_create_value(ctx, demo_type, &impl);
  assert(flag == flag_init);
  neo_value val2 = neo_scope_clone_value(neo_context_get_scope(ctx), val);
  assert(flag == flag_copy);
  free_neo_context(ctx);
  free_neo_runtime(rt);
  assert(flag == flag_dispose);
  return 0;
}