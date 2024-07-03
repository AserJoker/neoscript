#ifndef __NEO_COMMON_ALLOCATOR__
#define __NEO_COMMON_ALLOCATOR__
#include <stdlib.h>
typedef struct _neo_allocator *neo_allocator;
neo_allocator create_neo_allocator();
void free_neo_allocator(neo_allocator allocator);
void *neo_malloc(neo_allocator allocator, size_t size);
void neo_free(neo_allocator allocator, void *buf);
void neo_free_all(neo_allocator allocator);
#endif