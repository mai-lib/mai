// fixed-length ring buffer with presence bitmap
// XXX stretchy buffer is a relatively simple extension (hah)
// note that, since the maximum index is specified, and no item can be
// present more than once, overflow can't occur
#define WORKLIST(typ) struct {                          \
        int len, count, idx;                            \
        bitmap_t *map;                                  \
        typ *ptr;                                       \
    }

static inline int _wrap(int v, int l)
    { return (v >= l) ? v-l : v; }

#define worklist_isempty(wl) ((wl)->count == 0)

#define worklist_init(wl, n) do {                       \
        assert(n > 0);                                  \
        (wl)->len = n;                                  \
        (wl)->count = 0;                                \
        (wl)->idx = 0;                                  \
        (wl)->map = bitmap_create(n);                   \
        (wl)->ptr = xcalloc(n, sizeof(*((wl)->ptr)));   \
    } while(0)

#define worklist_fini(wl) do {                          \
        xfree((wl)->ptr);                               \
        xfree((wl)->map);                               \
    } while(0)

#define worklist_push(wl,v,id) do {                     \
        if(!bitmap_get_bit((wl)->map, id))              \
        {                                               \
            (wl)->ptr[_wrap((wl)->idx + (wl)->count,    \
                            (wl)->len)] = v;            \
            (wl)->count++;                              \
            bitmap_set_bit((wl)->map, id);              \
        }                                               \
    } while(0)

#define worklist_take(wl,p,id)                          \
    (!worklist_isempty(wl) ?                            \
     (*p = (wl)->ptr[(wl)->idx],                        \
      (wl)->count--,                                    \
      (wl)->idx = _wrap((wl)->idx + 1, (wl)->len),      \
      bitmap_clear_bit((wl)->map, id),                  \
      true) :                                           \
     false)
