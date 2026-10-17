#define ARRAY(_typ)                              \
    struct                                       \
    {                                            \
        size_t length;                           \
        size_t size;                             \
        _typ *ptr;                               \
    }

#define ARRAY_INIT { 0, 0, NULL }

static inline void *
_adjust_array(void *ptr, size_t elt, size_t size)
{
    if(size == 0)
    {
        if(ptr)
            xfree(ptr);
        return NULL;
    }
    if(ptr)
        return xrealloc(ptr, elt * size);
    return xmalloc(elt * size);
}

static inline void *
_extend_array(size_t *plength, size_t *psize, unsigned n,
              void *ptr, size_t elt)
{
    *plength += n;
    if(*plength > *psize)
    {
        *psize = max(*psize, 1);
        while(*plength > *psize)
            *psize *= 2;
        return _adjust_array(ptr, elt, *psize);
    }
    return ptr;
}

static inline void *
_init_array(size_t *plength, size_t *psize, size_t elt, size_t init)
{
    *psize = init;
    *plength = 0;
    if(init != 0)
        return _adjust_array(NULL, elt, init);
    return NULL;
}

#define _size_elt(_arr) (sizeof(*((_arr)->ptr)))

#define array_init(_arr, _init)                                         \
    ((_arr)->ptr = _init_array(&(_arr)->length,                         \
                               &(_arr)->size,                           \
                               _size_elt(_arr),                         \
                               _init))

#define array_alloc(_arr, _init)                                \
    (*_arr = xmalloc(sizeof(*(*_arr))),                         \
     array_init(*_arr, _init))

#define array_fini(_arr) do                     \
    {                                           \
        assert(_arr);                           \
        if((_arr)->ptr)                         \
            free((_arr)->ptr);                  \
        (_arr)->ptr = NULL;                     \
    } while (0)

#define array_free(_arr) do                     \
    {                                           \
        array_fini(_arr);                       \
        free(_arr);                             \
    } while (0)

// resize the array exactly
#define array_shrink(_arr)                              \
    ((_arr)->ptr = _adjust_array((_arr)->ptr,           \
                                 _size_elt(_arr),       \
                                 alen(_arr)))

// resize the array exactly, clear the array, return the contents pointer
#define array_cede(_arr)                        \
    (array_shrink(_arr),                        \
     array_clear(_arr),                         \
     _take_ptr((void **)&(_arr)->ptr, NULL))

#define array_clear(_arr) ((_arr)->length = 0)

#define array_isempty(_arr) ((_arr)->length == 0)

#define atail(_arr) ((_arr)->length - 1)

// XXX does not zero the additional storage
#define array_extend(_arr, _n)                                          \
    ((_arr)->ptr = _extend_array(&(_arr)->length,                       \
                                 &(_arr)->size, _n,                     \
                                 (_arr)->ptr,                           \
                                 _size_elt(_arr)))

#define array_push(_arr, _val)                                          \
    (array_extend(_arr, 1),                                             \
     (_arr)->ptr[atail(_arr)] = (_val))

#define array_pop(_arr)                                 \
    (assert(!array_isempty(_arr)),                      \
     (_arr)->ptr[--(_arr)->length])

#define array_drop(_arr, _n) ((_arr)->length -= _n)

#define array_take(_arr, _ptr)                                          \
    (!array_isempty(_arr) ?                                             \
     (*(_ptr) = (_arr)->ptr[--(_arr)->length],                          \
      true) :                                                           \
     false)

#define array_remove(_arr, _i) do                                       \
    {                                                                   \
        assert(_i < (_arr)->length);                                    \
        memmove((_arr)->ptr + (_i), (_arr)->ptr + (_i) + 1,             \
                ((_arr)->length - (_i) - 1) * _size_elt(_arr));         \
        (_arr)->length--;                                               \
    } while (0)

// insert val before i
#define array_insert(_arr, _i, _val) do                                 \
    {                                                                   \
        assert((_i) <= (_arr)->length);                                 \
        array_extend(_arr, 1);                                          \
        if((_i) < atail(_arr))                                          \
           memmove((_arr)->ptr + (_i) + 1, (_arr)->ptr + (_i),          \
                   ((_arr)->length - (_i) - 1) * _size_elt(_arr));      \
        (_arr)->ptr[_i] = _val;                                         \
    } while (0)

// length = size = new
#define array_resize(_arr, _new) do                                     \
    {                                                                   \
        (_arr)->ptr = _adjust_array((_arr)->ptr,                        \
                                    _size_elt(_arr),                    \
                                    _new);                              \
        if((_new) > (_arr)->length)                                     \
            memset((_arr)->ptr + (_arr)->length, 0,                     \
                   ((_new) - (_arr)->length) * _size_elt(_arr));        \
        (_arr)->length = (_arr)->size = _new;                           \
    } while (0)

// element sizes must match
#define array_copy(_to, _from) do                                       \
    {                                                                   \
        assert(_size_elt(_from) == _size_elt(_to));                     \
        (_to)->length = (_from)->length;                                \
        (_to)->ptr = _adjust_array((_to)->ptr,                          \
                                   _size_elt(_to),                      \
                                   (_to)->size = (_from)->length);      \
        memcpy((_to)->ptr, (_from)->ptr,                                \
               (_from)->length * _size_elt(_from));                     \
    } while (0)

// FIXME unused?
#define array_concat(_to, _from) do                             \
    {                                                           \
        unsigned _total = (_to)->length + (_from)->length;      \
        assert(_size_elt(_from) == _size_elt(_to));             \
        if(_total > (_to)->size)                                \
            (_to)->ptr = _adjust_array((_to)->ptr,              \
                                       _size_elt(_to),          \
                                       (_to)->size = _total);   \
        memcpy((_to)->ptr + (_to)->length, (_from)->ptr,        \
               (_from)->length * _size_elt(_from));             \
        (_to)->length = _total;                                 \
    } while (0)


// trivial
// indexed only
#define array_foreach(_arr, _i)                 \
    for(_i = 0;                                 \
        _i < alen(_arr);                        \
        ++_i)

// pointer to element
#define array_foreach_ptr(_arr, _ptr)                \
    for(_ptr = (_arr)->ptr;                          \
        _ptr < (_arr)->ptr + alen(_arr);             \
        ++_ptr)

// only set _entry in the test clause,
// otherwise it will happily index off the end of the array
#define array_foreach_entry(_arr, _entry)                       \
    for(int _i = 0;                                             \
        _i < alen(_arr) && (_entry = (_arr)->ptr[_i], true);    \
        ++_i)

#define array_map(_arr, _fn, _ptr) do           \
    {                                           \
        int _i;                                 \
        for(_i=0; _i<(_arr)->length; _i++)      \
            _fn((_arr)->ptr[_i], _i, _ptr);     \
    } while(0)

#define alen(_arr) ((_arr)->length)
#define asize(_arr) ((_arr)->size)
#define adata(_arr) ((_arr)->ptr)

#ifdef NDEBUG
#define aref(_arr, _i) ((_arr)->ptr[_i])
#define aset(_arr, _i, _val) ((_arr)->ptr[_i] = (_val))
#define aptr(_arr, _i) ((_arr)->ptr + _i)
#else
// FIXME: double evaluation makes these unsafe for side-effecting arguments
#define aref(_arr, _i) (assert(_i < (_arr)->length), (_arr)->ptr[_i])
#define aset(_arr, _i, _val) (assert(_i < (_arr)->length), (_arr)->ptr[_i] = (_val))
#define aptr(_arr, _i) (assert(_i < (_arr)->length), (_arr)->ptr + _i)
#endif
