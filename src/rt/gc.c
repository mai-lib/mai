#define HAS_VALGRIND
//#define STRESS_GC
#include "global.h"
#include <errno.h>
#include <sys/mman.h>
#ifdef HAS_VALGRIND
#include <valgrind/memcheck.h>
#endif
#if (defined(_XBS5_ILP32_OFF32) && _XBS5_ILP32_OFF32 != -1)
#define WORDSHIFT 2
#define NRESERVED 3
#elif (defined(_XBS5_LP64_OFF64) && _XBS5_LP64_OFF64 != -1)
#define WORDSHIFT 3
#define NRESERVED 6
#else
#error "platform bitness undefined."
#endif
#define CHUNKBITS 21
#define CHUNKSIZE (1<<CHUNKBITS)
#define CHUNKMASK (CHUNKSIZE - 1)
#define PAGEBITS  12
#define PAGESIZE  (1<<PAGEBITS)
#define PAGEMASK  (PAGESIZE - 1)
#define NPAGES    ((1<<(CHUNKBITS - PAGEBITS)) - NRESERVED)
static gc_stats_t gc_stats;
static bool gc_enabled;
static bool gc_request;
static list_t chunk_head;
static unsigned gc_nchunks = 0;
typedef struct free_obj
{
    robject_t hdr;
    SLIST(struct free_obj) next;
} free_obj_t;
typedef unsigned short objsz_t;
typedef struct
{
    list_t page_list;
    SLIST(free_obj_t) free;
    bitmap_t *markbits;
    objsz_t objsz;
    bool marked;
} page_t;
typedef struct
{
    list_t page_head;
    page_t *open;
    objsz_t objsz;
    unsigned nobj;
    unsigned npages, nalloc, nfree, nswept;
} pool_t;
typedef struct
{
    list_t chunk_list;
    list_t free_head;
    unsigned nlive;
    page_t pages[NPAGES];
} chunk_t;
static_assert(NRESERVED * PAGESIZE >= sizeof(chunk_t),
              "chunk header too large, reserve more pages.");
static inline void *mmap_chunk(void *hint, size_t sz)
{
    void *r = mmap(hint, sz, PROT_READ|PROT_WRITE,
                   MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

    if(r == MAP_FAILED)
        fatal("can't allocate %.1fk for chunk: %s.",
              sz / 1024.0, strerror(errno));
    return r;
}
static inline void munmap_chunk(void *ptr, size_t sz)
{
    if(munmap(ptr, sz) == -1)
        fatal("can't munmap chunk @ %p: %s.", ptr, strerror(errno));
}
static chunk_t *mmap_aligned_chunk(void *hint)
{
    uint8_t *head, *ptr = mmap_chunk(hint, CHUNKSIZE);

    if((uintptr_t)ptr & CHUNKMASK)
    {
        // throw it back
        munmap_chunk(ptr, CHUNKSIZE);
        // get one twice the size
        head = mmap_chunk(hint, CHUNKSIZE<<1);
        // align within that
        ptr = (uint8_t *)(((uintptr_t)head + CHUNKMASK) & ~CHUNKMASK);
        // give back the head
        munmap_chunk(head, ptr - head);
        // and tail
        munmap_chunk(ptr + CHUNKSIZE,
                     (head + (CHUNKSIZE<<1)) - (ptr + CHUNKSIZE));
    }
    return (chunk_t *)ptr;
}
static void *next_mmap = NULL;
static chunk_t *chunk_create()
{
    chunk_t *chunk = mmap_aligned_chunk(next_mmap);

    next_mmap = (uint8_t *)chunk + CHUNKSIZE;
    list_init(&chunk->free_head);
    chunk->nlive = 0;
    for(page_t *page = chunk->pages; page < &chunk->pages[NPAGES]; page++)
    {
        list_init(&page->page_list);
        list_add_before(&chunk->free_head, &page->page_list);
    }
    list_init(&chunk->chunk_list);
    list_add(&chunk_head, &chunk->chunk_list);
    gc_nchunks++;
    gc_stats.nchunks++;
    return chunk;
}
static void chunk_free(chunk_t *chunk)
{
    list_remove(&chunk->chunk_list);
    munmap_chunk(chunk, CHUNKSIZE);
    gc_nchunks--;
}
static inline page_t *chunk_get_page(chunk_t *chunk)
{
    page_t *page = container_of(chunk->free_head.next, page_t, page_list);

    list_remove(&page->page_list);
    return page;
}
static inline chunk_t *chunk_for_addr(void *obj)
    { return (chunk_t *)((uintptr_t)obj & ~CHUNKMASK); }
static inline void *page_storage(page_t *page)
{
    chunk_t *chunk = chunk_for_addr(page);
    ptrdiff_t idx = page - chunk->pages;

    return (uint8_t *)chunk + ((idx + NRESERVED) << PAGEBITS);
}
static inline page_t *page_for_object(void *obj)
{
    chunk_t *chunk = chunk_for_addr(obj);
    unsigned idx = ((uint8_t *)obj - (uint8_t *)chunk) >> PAGEBITS;
    assert(idx >= NRESERVED);
    return &chunk->pages[idx-NRESERVED];
}
static inline unsigned page_object_index(page_t *page, void *obj)
{
    uintptr_t offset = (uintptr_t)obj - ((uintptr_t)obj & ~PAGEMASK);
    assert(offset >= 0);
    return offset / page->objsz;
}
static const rtype_t *r_type_freed = (rtype_t *)0xDEADBEEF;
static inline bool object_is_free(free_obj_t *obj)
    { return obj->hdr.type == r_type_freed; }
static inline void page_link_free_object(page_t *page, free_obj_t *obj)
{
    obj->hdr.type = (rtype_t *)r_type_freed;
    slist_push(page->free, obj, next);
#ifdef HAS_VALGRIND
    VALGRIND_MAKE_MEM_NOACCESS((uint8_t *)obj + sizeof(rtype_t *),
                               page->objsz - sizeof(rtype_t *));
#endif
}
static inline void page_free_object(page_t *page, free_obj_t *obj)
{
    r_free(obj);
#ifndef NDEBUG
    memset(obj, 0xFE, page->objsz);
#endif
    page_link_free_object(page, obj);
}
#define page_foreach_object(pg, obj)                                    \
    for(void *_p = page_storage(pg);                                    \
        _p <= (void *)((uint8_t *)page_storage(pg)+PAGESIZE-pg->objsz)  \
            && (obj = _p);                                              \
        _p = (uint8_t *)_p + pg->objsz)
static page_t *page_init(page_t *page, objsz_t objsz, unsigned nobj)
{
    free_obj_t *obj;

    *page = (page_t) {
        .marked = false,
        .objsz = objsz,
        .markbits = bitmap_create(nobj),
        .free = NULL
    };
    page_foreach_object(page, obj)
    {
#ifdef HAS_VALGRIND
        VALGRIND_MAKE_MEM_DEFINED(obj, sizeof(free_obj_t));
#endif
        page_link_free_object(page, obj);
    }
    return page;
}
static void page_fini(page_t *page)
{
    chunk_t *chunk = chunk_for_addr(page);
    bitmap_free(page->markbits);
    list_add(&chunk->free_head, &page->page_list);
#ifdef HAS_VALGRIND
    VALGRIND_MAKE_MEM_NOACCESS(page_storage(page), PAGESIZE);
#endif
}
#define MAX_WORDS 9
static const unsigned objwords[] = { 2, 3, 4, 5, 6, MAX_WORDS };
static pool_t *sizepools[MAX_WORDS];
#define NPOOLS ((unsigned) lengthof(objwords)+1)
static pool_t pools[NPOOLS], *type_pool = &pools[NPOOLS-1];
static inline pool_t *pool_for_size(objsz_t sz)
    { return sizepools[sz >> WORDSHIFT]; }
static void pool_add_page(pool_t *pool, page_t *page)
{
    page_init(page, pool->objsz, pool->nobj);
    list_add(&pool->page_head, &page->page_list);
    pool->nfree += pool->nobj;
    pool->npages++;
}
static void pool_remove_page(pool_t *pool, page_t *page)
{
    assert(pool->open != page);
    pool->npages--;
    pool->nfree -= pool->nobj;
    list_remove(&page->page_list);
    page_fini(page);
}
static void pool_init(objsz_t sz, pool_t *pool)
{
    *pool = (pool_t) {
        .page_head = LIST_INIT(pool->page_head),
        .objsz = sz,
        .nobj = PAGESIZE / sz,
    };
}
static unsigned page_empty(page_t *page);
static void pool_fini(pool_t *pool)
{
    page_t *page, *tmp;

    list_foreach_entry_safe(&pool->page_head, page, tmp, page_list)
    {
        page_empty(page);
        list_remove(&page->page_list);
        page_fini(page);
    }
}
static inline void *alloc_from_page(page_t *page)
{
    free_obj_t *obj = page->free;

#ifdef HAS_VALGRIND
    VALGRIND_MAKE_MEM_DEFINED(obj, page->objsz);
#endif
    slist_remove_head(page->free, next);
    return obj;
}
static page_t *pool_get_page(pool_t *pool)
{
    page_t *page = pool->open;
    chunk_t *chunk;

    if(page && page->free)
        return page;
    list_foreach_entry(&pool->page_head, page, page_list)
        if(page->free)
            return page;
    list_foreach_entry(&chunk_head, chunk, chunk_list)
    {
        if(!list_isempty(&chunk->free_head))
        {
            page = chunk_get_page(chunk);
            pool_add_page(pool, page);
            return page;
        }
    }
    return NULL;
}
static unsigned gc_nchunks_target = 1;
static size_t gc_vec_target = 2*1024*1024;
static size_t gc_vec_bytes, gc_vec_swept;
static inline void *alloc_from_pool(pool_t *pool, size_t vecsz)
{
    page_t *page = pool_get_page(pool);
    bool obj_gc = (gc_nchunks >= gc_nchunks_target);
    bool vec_gc = (gc_vec_bytes + vecsz) > gc_vec_target;

#ifdef STRESS_GC
    if(true)
#else
    if(vec_gc || (obj_gc && !page))
#endif
    {
        if(gc_enabled)
        {
            gc_collect();
            page = pool_get_page(pool);
        }
        else
            gc_request = true;
    }
    if(!page)
    {
        page = chunk_get_page(chunk_create());
        pool_add_page(pool, page);
    }
    pool->open = page;
    pool->nalloc++;
    pool->nfree--;
    assert(pool->nfree < UINT_MAX);
    return alloc_from_page(page);
}
static inline void *alloc_for_vec(size_t vecsz)
{
    if(vecsz == 0)
        return NULL;
    gc_vec_bytes += vecsz;
    gc_vec_target = max(gc_vec_target, gc_vec_bytes);
    gc_stats.vec_bytes += vecsz;
    return xmalloc(vecsz);
}
#define QFRAG_LEN 256
#define QFRAG_MASK (QFRAG_LEN-1)
typedef struct qfrag
{
    robject_t *objs[QFRAG_LEN];
    unsigned s, m;
    SLIST(struct qfrag) next;
} qfrag_t;
static qfrag_t qhead, *qtail;
static inline unsigned advance(unsigned i)
    { return (i + 1) & QFRAG_MASK; }
static inline void qfrag_init(qfrag_t *frag)
{
    *frag = (qfrag_t) {
        .s = 0,
        .m = 0,
        .next = NULL
    };
}
static list_t roots_head = LIST_INIT(roots_head);
void gc_register(gc_root_t *root)
{
    list_init(&root->roots_list);
    list_add(&roots_head, &root->roots_list);
}
void gc_unregister(gc_root_t *root)
{
    list_remove(&root->roots_list);
}
static void collect_begin()
{
    gc_root_t *root;

    qfrag_init(&qhead);
    qtail = NULL;
    list_foreach_entry(&roots_head, root, roots_list)
        root->fn(root);
}
static inline bool mark_one_object(robject_t *obj)
{
    page_t *page = page_for_object(obj);
    unsigned idx = page_object_index(page, obj);

    if(!bitmap_get_bit(page->markbits, idx))
    {
        page->marked = true;
        bitmap_set_bit(page->markbits, idx);
        return true;
    }
    return false;
}
static inline void enqueue_one_object(robject_t *obj)
{
    if(!qtail)
        qtail = &qhead;
    else if(qtail->m == qtail->s)
    {
        qfrag_t *next = xmalloc(sizeof(*next));

        qfrag_init(next);
        qtail->next = next;
        qtail = next;
    }
    qtail->objs[qtail->m] = obj;
    qtail->m = advance(qtail->m);
}
void gc_mark(void *ptr)
{
    if(ptr && mark_one_object(ptr))
        enqueue_one_object(ptr);
}
static void collect_mark()
{
    for(qfrag_t *next, *scan = &qhead; scan; scan = next)
    {
        do
        {
            r_gc(scan->objs[scan->s]);
            scan->s = advance(scan->s);
        }
        while(scan->s != scan->m);
        next = scan->next;
        if(scan != &qhead)
            xfree(scan);
    }
}
static unsigned page_sweep(page_t *page)
{
    free_obj_t *obj;
    unsigned idx = 0, nswept = 0;
    chunk_t *chunk = chunk_for_addr(page);

    page_foreach_object(page, obj)
    {
        if(!bitmap_get_bit(page->markbits, idx)
           && !object_is_free(obj))
        {
            page_free_object(page, obj);
            nswept++;
        }
        idx++;
    }
    bitmap_reset(page->markbits, idx);
    page->marked = false;
    chunk->nlive++;
    return nswept;
}
static unsigned page_empty(page_t *page)
{
    free_obj_t *obj;
    unsigned nswept = 0;

    page_foreach_object(page, obj)
    {
        if(!object_is_free(obj))
        {
            r_free(obj);
            nswept++;
        }
    }
    return nswept;
}
static void pool_sweep(pool_t *pool)
{
    page_t *page, *tmp;
    unsigned nswept = 0; // mmm, fresh garbage.

    list_foreach_entry_safe(&pool->page_head, page, tmp, page_list)
    {
        if(!page->marked)
        {
            nswept += page_empty(page);
            if(pool->open == page)
                pool->open = NULL;
            pool_remove_page(pool, page);
        }
        else
            nswept += page_sweep(page);
    }
    pool->nalloc -= nswept;
    pool->nfree += nswept;
    pool->nswept = nswept;
}
static void collect_sweep()
{
    gc_vec_swept = 0;

    for(int i=0; i<NPOOLS; i++)
        pool_sweep(&pools[i]);
}
static inline intmax_t div_round_up(intmax_t a, int b)
    { return (a > 0) ? (a + b - 1) / b : a / b; }
#define HEADROOM_DIV 4
#define SWEPT_DIV 2
static unsigned target_size(unsigned nchunks, unsigned target)
{
    chunk_t *chunk;
    page_t *page;
    intmax_t delta = 0;

    for(int i=0; i<NPOOLS; i++)
    {
        pool_t *pool = &pools[i];
        intmax_t ndelta = div_round_up(pool->nalloc, HEADROOM_DIV);

        ndelta += div_round_up(pool->nswept, SWEPT_DIV);
        ndelta -= pool->nfree;
        delta += max(0, ndelta) * (size_t)pool->objsz;
    }
    list_foreach_entry(&chunk_head, chunk, chunk_list)
        list_foreach_entry(&chunk->free_head, page, page_list)
            delta -= PAGESIZE;
    target = nchunks + div_round_up(delta, CHUNKSIZE);
    return max(1, target);
}
static size_t target_vec_size(size_t bytes, size_t target)
{
    size_t delta = 0;

    if(bytes > target * 0.7)
        delta = (80 * 1024) + bytes * 0.1;
    else if(bytes < target * 0.3)
        delta = bytes * -0.2;
    return max(target + delta, 0);
}
static void collect_resize()
{
    list_t tmp_head = LIST_INIT(tmp_head);
    chunk_t *chunk, *tmp;

    gc_nchunks_target = target_size(gc_nchunks, gc_nchunks_target);
    gc_vec_target = target_vec_size(gc_vec_bytes, gc_vec_target);
    list_foreach_entry_safe(&chunk_head, chunk, tmp, chunk_list)
    {
        if(chunk->nlive == 0 && gc_nchunks > gc_nchunks_target)
        {
            chunk_free(chunk);
        }
        else
        {
            list_remove(&chunk->chunk_list);
            if(chunk->nlive == NPAGES)
                list_add_before(&tmp_head, &chunk->chunk_list);
            else
                list_add(&tmp_head, &chunk->chunk_list);
            chunk->nlive = 0;
        }
    }
    assert(list_isempty(&chunk_head));
    list_splice_after(&chunk_head, &tmp_head);
}
void gc_collect()
{
    assert(gc_enabled);
    gc_request = false;
    collect_begin();
    collect_mark();
    collect_sweep();
    collect_resize();
    gc_stats.ncollect++;
}
void *gc_alloc_vec(rtype_t *type, size_t sz, size_t vecsz, void **pvec)
{
    robject_t *obj;
    pool_t *pool;

    if(!type || type == r_type_type)
        pool = type_pool;
    else
        pool = pool_for_size(sz);

    obj = alloc_from_pool(pool, vecsz); // may collect
    obj->type = type;
    gc_stats.obj_bytes += sz;

    if(pvec)
      *pvec = alloc_for_vec(vecsz);
    return obj;
}
void gc_free_vec(void *ptr, size_t sz)
{
    xfree(ptr);
    gc_vec_bytes -= sz;
    gc_stats.vec_bytes -= sz;
    gc_vec_swept += sz;
}
void gc_release(void *ptr)
{
    if(ptr)
    {
        page_t *page = page_for_object(ptr);
        pool_t *pool = pool_for_size(page->objsz);
        unsigned idx = page_object_index(page, ptr);
        assert(!object_is_free(ptr));
        bitmap_clear_bit(page->markbits, idx);
        page_free_object(page, ptr);
        pool->nalloc--;
        pool->nfree++;
    }
}
void gc_set_enabled(bool enable)
{
    gc_enabled = enable;
    if(!enable)
        gc_request = false;
}
bool gc_was_requested()
{
    return gc_request;
}
void gc_collect_stats(gc_stats_t *stats)
{
    *stats = gc_stats;
    gc_stats = (gc_stats_t) { 0 };
}
void gc_init()
{
    list_init(&chunk_head);
    for(int i=0; i<NPOOLS-1; i++)
    {
        objsz_t sz = objwords[i] << WORDSHIFT;

        pool_init(sz, &pools[i]);
    }
    pool_init(sizeof(rtype_t), type_pool);
    for(int i=1, j=0; i<MAX_WORDS; i++)
    {
        if(i > objwords[j])
            j++;
        sizepools[i] = &pools[j];
    }
    gc_enabled = false;
}
void gc_fini()
{
    chunk_t *chunk, *tmp;

    for(int i=0; i<NPOOLS-1; i++)
        pool_fini(&pools[i]);
    pool_fini(type_pool);

    list_foreach_entry_safe(&chunk_head, chunk, tmp, chunk_list)
        chunk_free(chunk);
}
