#include "allocator.h"
#include <stdlib.h>
typedef struct _neo_allocator_chunk *neo_allocator_chunk;
struct _neo_allocator_chunk {
  void *buf;
  size_t size;
  int8_t busy;
  neo_allocator_chunk next;
};
struct _neo_allocator {
  neo_allocator_chunk chunks;
};
neo_allocator create_neo_allocator() {
  neo_allocator allocator = malloc(sizeof(struct _neo_allocator));
  allocator->chunks = malloc(sizeof(struct _neo_allocator_chunk));
  allocator->chunks->buf = NULL;
  allocator->chunks->busy = 1;
  allocator->chunks->size = 0;
  allocator->chunks->next = allocator->chunks;
  return allocator;
};
void free_neo_allocator(neo_allocator allocator) {
  neo_free_all(allocator);
  free(allocator->chunks);
  free(allocator);
}
void *neo_malloc(neo_allocator allocator, size_t size) {
  neo_allocator_chunk chunk = malloc(sizeof(struct _neo_allocator_chunk));
  chunk->size = size;
  chunk->busy = 1;
  chunk->buf = malloc(size);
  chunk->next = allocator->chunks->next;
  allocator->chunks->next = chunk;
  return chunk->buf;
}
void neo_free(neo_allocator allocator, void *buf) {
  neo_allocator_chunk chunk = allocator->chunks;
  while (chunk->next != allocator->chunks) {
    if (chunk->next->buf == buf) {
      free(chunk->next->buf);
      neo_allocator_chunk tmp = chunk->next;
      chunk->next = tmp->next;
      free(tmp);
    }
    chunk = chunk->next;
  }
}
void neo_free_all(neo_allocator allocator) {
  while (allocator->chunks->next != allocator->chunks) {
    neo_free(allocator, allocator->chunks->next->buf);
  }
}