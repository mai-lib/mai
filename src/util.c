#include "global.h"
#include <sys/mman.h>

void _vfatal(const char *fn, const char *fmt, va_list va)
{
    fprintf(stderr, "%s: ", fn);
    vfprintf(stderr, fmt, va);
    fprintf(stderr, "\n");
    fflush(stderr);
    abort();
}

void _fatal(const char *fn, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    _vfatal(fn, fmt, va);
    va_end(va); /* NOTREACHED */
}

void *xrealloc(void *ptr, size_t size)
{
    if(size == 0)
        fatal("allocation size 0.");
    void *ret = realloc(ptr, size);
    if(!ret)
        fatal("allocation failed.");
    return ret;
}

// FIXME: attribute malloc for these
void *xmalloc(size_t size)
{
    if(size == 0)
        fatal("allocation size 0.");
    void *ret = malloc(size);
    if(!ret)
        fatal("allocation failed.");
    return ret;
}

void *xcalloc(size_t num, size_t size)
{
    if(size == 0)
        fatal("allocation size 0.");
    void *ret = calloc(num, size);
    if(!ret)
        fatal("allocation failed.");
    return ret;
}

void xfree(void *ptr)
{
    if(!ptr)
        fatal("freeing NULL.");
    free(ptr);
}

void *xmap(size_t sz)
{
    void *ret = mmap(NULL, sz, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, 0, 0);
    if(ret == MAP_FAILED)
    {
        perror("mmap failed");
        exit(1);
    }
    return ret;
}

bool string_equal(const void *x, const void *y)
{
    return !strcmp((char *)x, (char *)y);
}
uint32_t string_hash(const void *ptr)
{
    const char *string = ptr;
    return hash_code(string, strlen(string));
}
bool ptr_eq(const void *x, const void *y)
{
    return x == y;
}
uint32_t ptr_hash(const void *ptr)
{
    return hash_code(&ptr, sizeof(ptr));
}

void hexdump(FILE *fp, uint8_t *mem, ptrdiff_t len, unsigned width)
{
    do
    {
        for(int i=0; (!width || i<width) && len>0; i++, len--)
            fprintf(fp, "%02X ", *mem++);
        if(width)
            fprintf(fp, "\n");
    } while (len > 0);
}

void mem_dump(uint8_t *mem, ptrdiff_t len)
{
    printf("%p: ", mem);
    hexdump(stdout, mem, len, sizeof(intptr_t));
}

// static return buffer, caveat caller
char *mem_string(uint8_t *mem, ptrdiff_t len)
{
    static char buf[32];
    char *ptr = buf;

    while(len-- > 0)
    {
        ptr += snprintf(ptr, sizeof(buf) - (ptr-buf), "%02X ", *mem++);
    }

    return buf;
}
