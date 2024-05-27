#ifndef __NEO_UTIL_IMAP__
#define __NEO_UTIL_IMAP__
#include "./list.h"
#include <stdint.h>
typedef struct _neo_imap *neo_imap;
neo_imap create_neo_imap(neo_free_fn fn);
void free_neo_imap(neo_imap imap);
void neo_imap_set(neo_imap self, int32_t key, void *value);
void *neo_imap_get(neo_imap self, int32_t key);
void neo_imap_delete(neo_imap self, int32_t key);
size_t neo_imap_length(neo_imap self);
#endif