#include "global.h"

static inline uint32_t rotl32(uint32_t x, int8_t r)
{
    return (x << r) | (x >> (32 - r));
}

static uint32_t fmix(uint32_t h)
{
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}

uint32_t hash_code(const void *key, size_t len)
{
    return hash_code_seed(key, len, 0);
}

uint32_t hash_code_seed(const void *key, size_t len, uint32_t h1)
{
    const uint8_t *data = (const uint8_t *)key;
    const int nblocks = len / 4;

    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;

//----------
// body

    const uint32_t *blocks = (const uint32_t *)(data + nblocks*4);

    for(int i = -nblocks; i; i++)
    {
        uint32_t k1 = blocks[i];

        k1 *= c1;
        k1 = rotl32(k1,15);
        k1 *= c2;

        h1 ^= k1;
        h1 = rotl32(h1,13);
        h1 = h1*5+0xe6546b64;
    }

//----------
// tail

    const uint8_t *tail = (const uint8_t *)(data + nblocks*4);

    uint32_t k1 = 0;

    switch(len & 3)
    {
    case 3: k1 ^= tail[2] << 16;
    case 2: k1 ^= tail[1] << 8;
    case 1: k1 ^= tail[0];
        k1 *= c1; k1 = rotl32(k1,15); k1 *= c2; h1 ^= k1;
    };

//----------
// finalization

    h1 ^= len;

    return fmix(h1);
}

typedef struct hashbase
{
    size_t nentries;
    size_t size;
    uint32_t (*hash_fn)(const void *key);
    bool (*eq_fn)(const void *x, const void *y);
} hashbase_t;

static void hash_init(hashbase_t *hash,
                      uint32_t (*hash_fn)(const void *),
                      bool (*eq_fn)(const void *, const void *))
{
    *hash = (hashbase_t) {
        .size = 4,
        .nentries = 0,
        .hash_fn = hash_fn,
        .eq_fn = eq_fn
    };
}

// map of void * -> void *
#define HASH_NAME hashmap
#define VALUE_TYPE void *
#include "hash.c.inc"
#undef HASH_NAME
#undef VALUE_TYPE
// set of void *
#define HASH_NAME hashset
#include "hash.c.inc"
#undef HASH_NAME
