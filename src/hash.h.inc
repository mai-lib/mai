// shared with hash_c.h
#define hash_t PPASTE2(HASH_NAME, _t)
#define func(n) PPASTE3(HASH_NAME, _, n)
#if defined(VALUE_TYPE)
#define result_t VALUE_TYPE
#define args(k, v) k, v
#else
#define result_t void *
#define args(k, v) k
#endif

typedef struct HASH_NAME hash_t;

result_t func(get)(const hash_t *tbl, args(const void *key, bool *found));
result_t func(add_update)(hash_t *tbl, args(const void *key, const void *value),
                          bool add, bool update);
result_t func(remove)(hash_t *tbl, const void *key);
void func(map)(const hash_t *tbl,
               void (*fn)(args(const void *, void *), void *),
               void *ptr);
hash_t *func(create)(uint32_t (*hash_fn)(const void *),
                     bool (*eq_fn)(const void *, const void *));
void func(free)(hash_t *tbl);
void func(clear)(hash_t *tbl, size_t newsize);

// returns old value if already present
static inline result_t
func(insert)(hash_t *tbl, args(const void *key, const void *value))
{
    return func(add_update)(tbl, args(key, value), true, false);
}

// only update value if present, returning old value. else, just return zero.
static inline result_t
func(update)(hash_t *tbl, args(const void *key, const void *value))
{
    return func(add_update)(tbl, args(key, value), false, true);
}

// add or update. returns old value if present, new value if not
static inline result_t
func(set)(hash_t *tbl, args(const void *key, const void *value))
{
    return func(add_update)(tbl, args(key, value), true, true);
}

#undef hash_t
#undef result_t
#undef func
#undef args
