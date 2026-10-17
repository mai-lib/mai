// type and function names
#define hash_t PPASTE2(HASH_NAME, _t)
#define node_t PPASTE2(HASH_NAME, _node_t)
#define func(n) PPASTE3(HASH_NAME, _, n)

#if defined(VALUE_TYPE)
// map
#define result_t VALUE_TYPE
#define result_found(n) (n)->value
#define args(k, v) k, v
typedef struct
{
    void *key;
    VALUE_TYPE value;
} node_t;
//
#else // defined(VALUE_TYPE)
// set
#define result_t void *
#define result_found(n) n->key
#define args(k, v) k
typedef struct
{
    void *key;
} node_t;
//
#endif // defined(VALUE_TYPE)

#define no_result ((result_t)0)

typedef struct HASH_NAME
{
    hashbase_t h;
    node_t *entries;
} hash_t;

static void func(rehash)(args(const void *key, void *value), void *ptr)
{
    func(insert)((hash_t *)ptr, args(key, value));
}

static bool func(resize)(hash_t *tbl, size_t newsize)
{
    hash_t tmp = *tbl;

    assert(newsize >= tbl->h.nentries);

    tmp.h.nentries = 0;
    tmp.h.size = newsize;
    tmp.entries = xcalloc(newsize, sizeof(node_t));

    func(map)(tbl, func(rehash), (void *)&tmp);
    assert(tmp.h.nentries == tbl->h.nentries);

    tbl->h.size = newsize;
    xfree(tbl->entries);
    tbl->entries = tmp.entries;

    return true;
}

// returns the value of nonnull key, or NULL if not present
// if found is nonnull, sets it to true or false on return.
// (only makes sense if VALUE_TYPE is set)
result_t func(get)(const hash_t *tbl, args(const void *key, bool *found))
{
    unsigned mask, index;
    node_t *node;

    if(!key)
        goto fail;

    mask = tbl->h.size - 1;
    index = tbl->h.hash_fn(key) & mask;

    while(node = &tbl->entries[index], node->key)
    {
        if(tbl->h.eq_fn(node->key, key))
        {
#if defined(VALUE_TYPE)
            if(found)
                *found = true;
#endif
            return result_found(node);
        }
        index = (index + 1) & mask;
    }

fail:
#if defined(VALUE_TYPE)
    if(found)
        *found = false;
#endif
    return no_result;
}


result_t func(add_update)(hash_t *tbl, args(const void *key, const void *value),
                          bool add, bool update)
{
    uint32_t hash;
    unsigned mask, index;
    node_t *node;

    if(!key)
        return no_result;

    if(add && ((float)tbl->h.nentries / tbl->h.size > 0.7))
    {
        if(!func(resize)(tbl, tbl->h.size<<1))
            fatal("can't resize hash table.\n");
    }

    mask = tbl->h.size - 1;
    hash = tbl->h.hash_fn(key);
    index = hash & mask;

    while(true)
    {
        node = &tbl->entries[index];
        if(!node->key)
        {
            if(add)
            {
                node->key = (void *)key;
#if defined(VALUE_TYPE)
                node->value = (void *)value;
#endif
                tbl->h.nentries++;
                return result_found(node);
            }
            return no_result;
        }
        else if(tbl->h.eq_fn(node->key, key))
        {
            if(update)
            {
                result_t old = result_found(node);
#if defined(VALUE_TYPE)
                node->value = (void *)value;
#endif
                return old;
            }
            //return no_result;
            return result_found(node);
        }

        index = (index + 1) & mask;
    }
    return no_result; /* NOTREACHED */
}

// remove nonnull key and its value from tbl, if present
// inspired substantially by http://en.wikipedia.org/wiki/Open_addressing
result_t func(remove)(hash_t *tbl, const void *key)
{
    unsigned mask, index, gap, natural;
    node_t *node;
    result_t result;

    if(!key)
        return no_result;

    if(tbl->h.size > 4 && (float)tbl->h.nentries / tbl->h.size < 0.3)
    {
        if(!func(resize)(tbl, tbl->h.size>>1))
            fatal("can't resize hash table.\n");
    }

    mask = tbl->h.size - 1;
    index = tbl->h.hash_fn(key) & mask;
    result = no_result;

    while(node = &tbl->entries[index], node->key)
    {
        if(tbl->h.eq_fn(node->key, key))
        {
            gap = index;
            result = result_found(node);
        }
        else if(result)
        {
            // the natural location of this node, which is actually at index
            natural = tbl->h.hash_fn(node->key) & mask;

            // if the gap lies in this node's chain, swap them.
            // index only advances, so gap is always before it in sequence order
            // this is just one test - (natural .before. gap)
            if((gap < index) ?
               (natural <= gap || natural > index) :
               (natural <= gap && natural > index))
            {
                tbl->entries[gap] = *node;
                gap = index;
            }
        }
        index = (index + 1) & mask;
    }

    if(result)
    {
        node = &tbl->entries[gap];
        node->key = NULL;
#if defined(VALUE_TYPE)
        node->value = no_result;
#endif
        tbl->h.nentries--;
        return result;
    }

    return no_result;
}

hash_t *func(create)(uint32_t (*hash_fn)(const void *),
                     bool (*eq_fn)(const void *, const void *))
{
    hash_t *tbl = xmalloc(sizeof(hash_t));

    hash_init(&tbl->h, hash_fn, eq_fn);
    tbl->entries = xcalloc(tbl->h.size, sizeof(node_t));

    return tbl;
}

void func(free)(hash_t *hash)
{
    xfree(hash->entries);
    xfree(hash);
}

void func(clear)(hash_t *tbl, size_t newsize)
{
    tbl->h.nentries = 0;
    if(newsize > 0)
    {
        tbl->h.size = newsize;
        xfree(tbl->entries);
        tbl->entries = xcalloc(newsize, sizeof(node_t));
    }
    else
        memset(tbl->entries, 0, tbl->h.size * sizeof(node_t));
}

// not stable under insertion or deletion!
void func(map)(const hash_t *tbl,
               void (*fn)(args(const void *, void *), void *),
               void *ptr)
{
    int i;
    node_t *node;

    for(i=0,node=tbl->entries; i<tbl->h.size; i++,node++)
    {
        if(node->key)
            fn(args(node->key, node->value), ptr);
    }
}

#undef hash_t
#undef node_t
#undef func
#undef result_t
#undef result_found
#undef args
