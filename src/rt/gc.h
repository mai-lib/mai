typedef struct gc_root_s gc_root_t;
typedef void (*gc_root_fn)(gc_root_t *);
typedef struct gc_root_s
{
    list_t roots_list;
    gc_root_fn fn;
} gc_root_t;
typedef struct
{
    unsigned ncollect;
    unsigned nchunks;
    size_t obj_bytes;
    size_t vec_bytes;
} gc_stats_t;
void gc_register(gc_root_t *root);
void gc_unregister(gc_root_t *root);
void gc_collect();
void gc_mark(void *ptr);
void gc_collect_stats(gc_stats_t *stats);
void *gc_alloc_vec(rtype_t *type, size_t sz, size_t vecsz, void **pvec);
void gc_free_vec(void *ptr, size_t vecsz);
void gc_release(void *ptr);
void gc_set_enabled(bool enable);
bool gc_was_requested();
void gc_init();
void gc_fini();
static inline void *gc_alloc(rtype_t *type, size_t sz)
{
    return gc_alloc_vec(type, sz, 0, NULL);
}
