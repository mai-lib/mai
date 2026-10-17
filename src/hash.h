uint32_t hash_code(const void *key, size_t len);
uint32_t hash_code_seed(const void *key, size_t len, uint32_t h1);

// map of void * -> void *
#define HASH_NAME hashmap
#define VALUE_TYPE void *
#include "hash.h.inc"
#undef HASH_NAME
#undef VALUE_TYPE

// set of void *
#define HASH_NAME hashset
#include "hash.h.inc"
#undef HASH_NAME
