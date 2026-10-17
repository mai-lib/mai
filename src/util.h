/*
  double expansion - stringification or token-pasting
  disables argument expansion in function-like macros.
  so, wrap the operation in another macro to get the
  desired (expanded and then stringified/pasted) result.
*/
#define STRINGIFY(a) #a
#define PASTE2(a, b) a##b
#define PASTE3(a, b, c) a##b##c
#define PASTE4(a, b, c, d) a##b##c##d
#define PASTE5(a, b, c, d, e) a##b##c##d##e
#define PASTE6(a, b, c, d, e, f) a##b##c##d##e##f
/*
  (triple expansion - for token-pasting parameterised by an
  object-like macro)
*/
#define PPASTE2(a, b) PASTE2(a, b)
#define PPASTE3(a, b, c) PASTE3(a, b, c)

// caveat: double evaluation!
#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#define lengthof(a) (sizeof(a) / sizeof(*a))

// reasonably standards-safe; thanks stddef.h
#define container_of(ptr, typ, field)           \
    ((typ *)((char *)(ptr) - offsetof(typ, field)))

// caveat: assumes all pointers are the same size!
static inline void *_take_ptr(void **ptr, void *next)
{
    void *tmp = *ptr;
    *ptr = next;
    return tmp;
}

// define NOSPAM per-file: dbg output suppressed for that file.
// define DBG_SUPPRESS in config.mk: no dbg output.
// define NDEBUG: no asserts, no dbg output
#if !defined(NDEBUG) && !defined(NOSPAM) && !defined(DBG_SUPPRESS)
#define DBG(args...) fprintf(stderr, args)
#define DBGPRINT(val) r_print(stderr, val)
#define DBG_OUTPUT
#else
#define DBG(args...) do { } while(0)
#define DBGPRINT(val) do { } while(0)
#endif

// NOTE: gcc-ism
#define fatal(fmt, args...) _fatal(__func__, fmt , ##args)
#define vfatal(fmt, va) _vfatal(__func__, fmt, va)
void _vfatal(const char *fn, const char *fmt, va_list va);
void _fatal(const char *fn, const char *fmt, ...);

void *xrealloc(void *ptr, size_t size);
void *xmalloc(size_t size);
void *xcalloc(size_t num, size_t size);
void xfree(void *ptr);
void *xmap(size_t sz);

// bit vector
typedef uint8_t bitmap_t;
static inline size_t size_for_bits(unsigned n)
    { return (n+7)>>3; }
static inline bool bitmap_get_bit(bitmap_t *bits, unsigned n)
    { return (bits[n>>3] & (1<<(n&7))) != 0; }
static inline uint8_t bitmap_get_byte(bitmap_t *bits, unsigned n)
    { return bits[n>>3]; }
static inline void bitmap_set_bit(bitmap_t *bits, unsigned n)
    { bits[n>>3] |= 1<<(n&7); }
static inline void bitmap_clear_bit(bitmap_t *bits, unsigned n)
    { bits[n>>3] &= ~(1<<(n&7)); }
static inline void bitmap_free(bitmap_t *bits)
    { xfree(bits); }
static inline void bitmap_reset(bitmap_t *bits, unsigned n)
    { memset(bits, 0, size_for_bits(n)); }
static inline bitmap_t *bitmap_create(unsigned n)
    { return xcalloc(1, size_for_bits(n)); }

// XXX int __builtin_popcount(unsigned int)
static inline unsigned bitmap_count_bits(bitmap_t *bits, unsigned n)
{
    unsigned r = 0;
    for(int i=0; i<size_for_bits(n); i++)
    {
        uint8_t byte = bits[i];
        for(int j=0; j<8; j++, byte >>= 1)
        {
            if(byte & 1)
                r++;
        }
    }
    return r;
}

void hexdump(FILE *fp, uint8_t *mem, ptrdiff_t len, unsigned width);
void mem_dump(uint8_t *mem, ptrdiff_t len);
char *mem_string(uint8_t *mem, ptrdiff_t len);

bool string_equal(const void *x, const void *y);
uint32_t string_hash(const void *ptr);
bool ptr_eq(const void *x, const void *y);
uint32_t ptr_hash(const void *ptr);
