#include "global.h"
#include "rt/builtin.h"
#include "ast.h"
static inline bool
buf_scal_equal(const rbuf_t *x, const rbuf_t *y, rtype_t *typ)
    { return !memcmp(x->elts, y->elts, x->length * rscal_size(typ)); }
static inline bool buf_obj_equal(const rbuf_t *x, const rbuf_t *y)
{
    robject_t **vx = x->elts, **vy = y->elts;
    for(int i=0; i < x->length; i++)
        if(!r_equal(vx[i], vy[i]))
            return false;
    return true;
}
static inline bool rbuf_equal(const rbuf_t *x, const rbuf_t *y)
{
    rtype_t *xt = elt_type(x), *yt = elt_type(y);

    if(xt != yt)
        return false;
    if(x->length != y->length)
        return false;
    return rtype_is_scalar(xt)
        ? buf_scal_equal(x, y, xt)
        : buf_obj_equal(x, y);
}
static inline uint32_t
buf_scal_hash(const rbuf_t *buf, uint32_t hash, rtype_t *etyp)
{
    size_t vecsz = buf->length * rtype_eltsz(etyp);
    return hash_code_seed(buf->elts, vecsz, hash);
}
static inline uint32_t buf_obj_hash(const rbuf_t *buf, uint32_t hash)
{
    robject_t **elts = buf->elts;
    for(int i = 0; i < buf->length; i++)
        hash = hash_roll(hash, r_hash(elts[i]));
    return hash;
}
static inline uint32_t rbuf_hash(const rbuf_t *buf)
{
    uint32_t hash = r_hash(r_typeof(&buf->base));
    rtype_t *etyp = elt_type(buf);

    return rtype_is_scalar(etyp)
        ? buf_scal_hash(buf, hash, etyp)
        : buf_obj_hash(buf, hash);
}
static inline void rbuf_gc(rbuf_t *buf)
{
    if(!rtype_is_scalar(elt_type(buf)))
    {
        robject_t **elts = buf->elts;
        for(int i = 0; i < buf->length; i++)
            gc_mark(elts[i]);
    }
}
static inline void rbuf_free(rbuf_t *buf)
{
    if(buf->elts)
        gc_free_vec(buf->elts, buf->length * rtype_eltsz(elt_type(buf)));
}
static inline void *rbuf_create(rtype_t *typ, unsigned length, size_t sz)
{
    assert(rtype_is_container(typ));
    void *elts;
    size_t vecsz = length * rtype_eltsz(typ->elt);
    rbuf_t *buf = gc_alloc_vec(typ, sz, vecsz, &elts);

    buf->elts = elts;
    buf->length = length;
    return buf;
}
static inline bool clip_to_max(int *plen)
{
    if(*plen > opt.print_max)
    {
        *plen = opt.print_max;
        return true;
    }
    return false;
}
static bool clip_npieces(int *npieces, int piecelen)
{
    int len = *npieces * piecelen;
    bool truncated = clip_to_max(&len);

    if(piecelen > 0)
    {
        // divide into pieces
        div_t r = div(len, piecelen);
        *npieces = r.quot + (r.rem > 0);
    }
    return truncated;
}
static inline int name_width(rsymbol_t *name)
{
    int l = strlen(r_symstr(name));
    return min(l, opt.print_name_max);
}
static int names_width(rsymbol_t **names, int len)
{
    int width = 0;
    for(int i = 0; i < len; i++)
    {
        int w = name_width(names[i]);
        width = max(width, w);
    }
    return width;
}
typedef struct
{
    int width;
    void *data;
} scalfmt_t;
#define scalfmt_init(w, s) \
    (scalfmt_t) { .width = (w), .data = scalprint_init(s) }
static inline void scalfmt_width(scalfmt_t *fmt, const scaldesc_t *scal,
                                 uint8_t *eptr, int len)
{
    for(int i = 0; i < len; i++, eptr += scal->size)
        fmt->width = scal->width(eptr, fmt->width, fmt->data);
}
static inline int idx_width(int i)
{
    const scaldesc_t *intscal = r_type_int->scal;
    return intscal->width(&i, 0, NULL);
}
static void print_name(FILE *fp, rsymbol_t *name, int width)
{
    fprintf(fp, "%*.*s", width, width, r_symstr(name));
}
static void print_vec_hdr(FILE *fp, const rvector_t *vec)
{
    r_print(fp, r_typeof(vec));
    fprintf(fp, " [%d]", rvec_len(vec));
}
static inline void
print_vec_scal_elts(FILE *fp, const scaldesc_t *scal, uint8_t *eptr,
                    int len, scalfmt_t fmt)
{
    for(int i = 0; i < len; i++, eptr += scal->size)
    {
        fprintf(fp, " ");
        scal->print(fp, eptr, fmt.width, fmt.data);
    }
}
static inline void print_vec_scal_names(FILE *fp, rsymbol_t **names,
                                        int len, int width)
{
    fprintf(fp, "\n");
    for(int i = 0; i < len; i++)
    {
        fprintf(fp, " ");
        print_name(fp, names[i], width);
    }
}
static void print_vec_obj(FILE *fp, robject_t **objs, rsymbol_t **names,
                          int len)
{
    int width = names
        ? names_width(names, len)
        : idx_width(len);
    bool truncated = clip_to_max(&len);

    for(int i = 0; i < len; i++)
    {
        fprintf(fp, "\n");
        if(names)
            print_name(fp, names[i], width);
        else
            fprintf(fp, "[%*d]", width, i + 1);
        fprintf(fp, ": ");
        r_print(fp, objs[i]);
    }

    if(truncated)
        fprintf(fp, "\n ...");
}
static void print_vec_scal(FILE *fp, const scaldesc_t *scal, void *elts,
                           rsymbol_t **names, int len)
{
    uint8_t *eptr = elts;
    bool truncated = clip_to_max(&len);
    scalfmt_t fmt = scalfmt_init(names ? names_width(names, len) : 0, scal);
    scalfmt_width(&fmt, scal, elts, len);
    int iwidth = idx_width(len);
    int nch = names ? 0 : iwidth + 2;
    int linech = nch, llen = 0;
    size_t lsz = scal->size;
    for(int i = 0; i < len; i++)
    {
        linech += fmt.width + 1;
        if(linech > opt.print_line && i > 0)
            break;
        llen++;
    }
    lsz *= llen;
    for(int i = 0; i < len; i += llen, eptr += lsz)
    {
        int plen = min(llen, len - i);

        fprintf(fp, "\n");
        if(!names)
            fprintf(fp, "[%*d]", iwidth, i + 1);
        print_vec_scal_elts(fp, scal, eptr, plen, fmt);
        if(names)
            print_vec_scal_names(fp, names + i, plen, fmt.width);
    }
    if(truncated)
        fprintf(fp, " ...");
}
static inline void idxs_advance(int rank, int *shape, int *idxs)
{
    for(int i = 0; i < rank; i++)
    {
        if(++idxs[i] >= shape[i])
            idxs[i] = 0;
        else
            break;
    }
}
static inline int idxs_init(int rank, int *shape, int *idxs)
{
    int len = 1;
    for(int i = 0; i < rank; i++)
    {
        idxs[i] = 0;
        len *= shape[i];
    }
    return len;
}
static inline rsymbol_t *idx_name(rvector_t *names, int i)
    { return ((rsymbol_t **)rvec_elts(names))[i]; }
static inline void print_idxs(FILE *fp, int *idxs, int rank,
                              rvector_t **dimnames)
{
    for(int i = 0; i < rank; i++)
    {
        int idx = idxs[i];

        if(i > 0)
            fprintf(fp, ",");
        if(dimnames && dimnames[i])
            fprintf(fp, "%s", r_symstr(idx_name(dimnames[i], idx)));
        else
            fprintf(fp, "%d", idx + 1);
    }
}
static void print_arr_hdr(FILE *fp, const rarray_t *arr)
{
    r_print(fp, r_typeof(arr));
    fprintf(fp, " [");
    for(int i = 0; i < arr->rank; i++)
    {
        if(i > 0)
            fprintf(fp, ", ");
        fprintf(fp, "%d", arr->shape[i]);
    }
    fprintf(fp, "]");
}
static void print_arr_obj(FILE *fp, robject_t **objs, rvector_t **dimnames,
                          int rank, int *shape)
{
    int *idxs = alloca(rank * sizeof(int));
    int len = idxs_init(rank, shape, idxs);
    bool truncated = clip_to_max(&len);

    for(int i = 0; i < len; i++)
    {
        fprintf(fp, "\n[");
        print_idxs(fp, idxs, rank, dimnames);
        fprintf(fp, "]: ");
        r_print(fp, objs[i]);
        idxs_advance(rank, shape, idxs);
    }
    if(truncated)
        fprintf(fp, "\n ...");
}
static inline void
print_arr_scal_elts(FILE *fp, const scaldesc_t *scal, uint8_t *eptr,
                    int i, int end, int csz, scalfmt_t *fmts)
{
    for(; i < end; i++, eptr += csz)
    {
        fprintf(fp, " ");
        scal->print(fp, eptr, fmts[i].width, fmts[i].data);
    }
}
static void
slice_width(const scaldesc_t *scal, uint8_t *eptr, scalfmt_t *fmts,
            int *ciwidths, rvector_t *colnames, int nrows, int ncols)
{
    size_t csz = scal->size * nrows;

    for(int i = 0; i < ncols; i++, eptr += csz)
    {
        int iwidth = colnames
            ? name_width(idx_name(colnames, i))
            : idx_width(i + 1);

        ciwidths[i] = iwidth;
        fmts[i].width = colnames ? iwidth : 3 + iwidth;
        scalfmt_width(&fmts[i], scal, eptr, nrows);
    }
}
static inline void print_col_idxs(FILE *fp, int i, int end, scalfmt_t *fmts,
                                  int *ciwidths, rvector_t *colnames)
{
    for(; i < end; i++)
    {
        fprintf(fp, " ");
        if(colnames)
            print_name(fp, idx_name(colnames, i), fmts[i].width);
        else
            fprintf(fp, "%*s[,%*d]",
                    max(fmts[i].width - ciwidths[i] - 3, 0), "",
                    ciwidths[i], i + 1);
    }
}
static void print_slice(FILE *fp, const scaldesc_t *scal, uint8_t *eptr,
                        rvector_t **dimnames, int *shape)
{
    rvector_t *rownames = dimnames ? dimnames[0] : NULL,
              *colnames = dimnames ? dimnames[1] : NULL;
    int nrows = shape[0], ncols = shape[1];
    size_t csz = scal->size * nrows;
    bool truncated = clip_npieces(&nrows, ncols);
    int riwidth = rownames
        ? names_width(rvec_elts(rownames), nrows)
        : idx_width(nrows);
    int nch = rownames ? riwidth : riwidth + 3;
    int *ciwidths = alloca(ncols * sizeof(int));
    scalfmt_t *fmts = alloca(ncols * sizeof(scalfmt_t));

    for(int i=0; i<ncols; i++)
        fmts[i] = scalfmt_init(0, scal);
    slice_width(scal, eptr, fmts, ciwidths, colnames, nrows, ncols);
    for(int i = 0, end = 0; i < ncols; eptr += (end - i) * csz, i = end)
    {
        int linech = nch;
        for(int j = i; j < ncols; j++)
        {
            linech += fmts[j].width + 1;
            if(linech > opt.print_line && j > i)
                break;
            end++;
        }
        fprintf(fp, "\n%*s", nch, "");
        print_col_idxs(fp, i, end, fmts, ciwidths, colnames);
        uint8_t *rptr = eptr;
        for(int j = 0; j < nrows; j++, rptr += scal->size)
        {
            fprintf(fp, "\n");
            if(rownames)
                print_name(fp, idx_name(rownames, j), riwidth);
            else
                fprintf(fp, "[%*d,]", riwidth, j + 1);
            print_arr_scal_elts(fp, scal, rptr, i, end, csz, fmts);
        }
    }
    if(truncated)
        fprintf(fp, "\n ...");
}
static void print_slices(FILE *fp, const scaldesc_t *scal, uint8_t *eptr,
                         rvector_t **dimnames, int rank, int *shape)
{
    int rrank = rank - 2, *rshape = shape + 2;
    int slicelen = shape[0] * shape[1];
    int *idxs = alloca(rrank * sizeof(int));
    int len = idxs_init(rrank, rshape, idxs);
    bool truncated = clip_npieces(&len, slicelen);
    size_t ssz = slicelen * scal->size;

    for(int i = 0; i < len; i++, eptr += ssz)
    {
        fprintf(fp, "\n\n[,,");
        print_idxs(fp, idxs, rrank, dimnames ? dimnames + 2 : NULL);
        fprintf(fp, "]");
        print_slice(fp, scal, eptr, dimnames, shape);
        idxs_advance(rrank, rshape, idxs);
    }
    if(truncated)
        fprintf(fp, "\n ...");
}
static void print_arr_scal(FILE *fp, const scaldesc_t *scal, void *elts,
                           rvector_t **dimnames, int rank, int *shape)
{
    switch(rank)
    {
    case 0:
        break;
    case 1:
        print_vec_scal(fp, scal, elts,
                       dimnames ? rvec_elts(dimnames[0]) : NULL,
                       shape[0]);
        break;
    case 2:
        print_slice(fp, scal, elts, dimnames, shape);
        break;
    default:
        print_slices(fp, scal, elts, dimnames, rank, shape);
        break;
    }
}
rvector_t *rvec_create(rtype_t *typ, unsigned length)
{
    assert(rtype_is_vector(typ));
    rvector_t *vec = rbuf_create(typ, length, sizeof(rvector_t));
    vec->names = NULL;
    return vec;
}
rvector_t *rvec_add_names(rvector_t *vec)
{
    int len = rvec_len(vec);
    assert(!vec->names);
    vec->names = rvec_create(r_type_vec_symbol, len);
    memset(rvec_elts(vec->names), 0, len * sizeof(rsymbol_t *));
    return vec->names;
}
static void rvec_free(void *ptr)
{
    rbuf_free(ptr);
}
static void rvec_gc(void *ptr)
{
    rvector_t *vec = ptr;
    rbuf_gc(&vec->buf);
    gc_mark(vec->names);
}
static uint32_t rvec_hash(const void *ptr)
{
    const rvector_t *vec = ptr;
    uint32_t hash = rbuf_hash(&vec->buf);
    if(vec->names)
        return hash_roll(hash, r_hash(vec->names));
    return hash;
}
static bool rvec_equal(const void *xp, const void *yp)
{
    const rvector_t *x = xp, *y = yp;
    if(!rbuf_equal(&x->buf, &y->buf)
       || rvec_is_named(x) != rvec_is_named(y))
        return false;
    return r_equal(x->names, y->names);
}
static void rvec_print(FILE *fp, const void *ptr)
{
    const rvector_t *vec = ptr;
    rtype_t *etyp = elt_type(&vec->buf);
    rsymbol_t **names = vec->names ? rvec_elts(vec->names) : NULL;

    print_vec_hdr(fp, vec);
    if(rtype_is_scalar(etyp))
        print_vec_scal(fp, etyp->scal, rvec_elts(vec), names, rvec_len(vec));
    else
        print_vec_obj(fp, rvec_elts(vec), names, rvec_len(vec));
}
static const typeops_t vec_ops = {
    .free = rvec_free,
    .gc = rvec_gc,
    .hash = rvec_hash,
    .equal = rvec_equal,
    .print = rvec_print
};
static int arr_length(int rank, int *dims)
{
    int len = rank > 0 ? 1 : 0;
    for(int i = 0; i < rank; i++)
        len *= dims[i];
    return len;
}
rarray_t *rarr_create_buf(rtype_t *typ, int len)
{
    rarray_t *arr = rbuf_create(typ, len, sizeof(rarray_t));

    arr->rank = 0;
    arr->shape = NULL;
    arr->dimnames = NULL;
    return arr;
}
void rarr_set_shape(rarray_t *arr, int rank, int *dims)
{
    size_t sz = rank * sizeof(int);
    int *shape = rank > 0 ? xmalloc(sz) : NULL;

    memcpy(shape, dims, sz);
    arr->rank = rank;
    if(arr->shape)
        xfree(arr->shape);
    arr->shape = shape;
}
rarray_t *rarr_create(rtype_t *typ, int rank, int *dims)
{
    assert(rank >= 0);
    int len = arr_length(rank, dims);
    rarray_t *arr = rarr_create_buf(typ, len);
    rarr_set_shape(arr, rank, dims);
    return arr;
}
rvector_t *rarr_add_names(rarray_t *arr)
{
    assert(!arr->dimnames);
    arr->dimnames = rvec_create(r_type_vec_names, arr->rank);
    memset(rvec_elts(arr->dimnames), 0, arr->rank * sizeof(rvector_t *));
    return arr->dimnames;
}
static void rarr_free(void *ptr)
{
    rarray_t *arr = ptr;
    rbuf_free(&arr->buf);
    if(arr->shape)
        xfree(arr->shape);
}
static void rarr_gc(void *ptr)
{
    rarray_t *arr = ptr;
    rbuf_gc(&arr->buf);
    gc_mark(arr->dimnames);
}
static uint32_t rarr_hash(const void *ptr)
{
    const rarray_t *arr = ptr;
    uint32_t hash = rbuf_hash(&arr->buf);
    hash = hash_code_seed(arr->shape, arr->rank * sizeof(int), hash);
    if(arr->dimnames)
        return hash_roll(hash, r_hash(arr->dimnames));
    return hash;
}

static bool rarr_equal(const void *xp, const void *yp)
{
    const rarray_t *x = xp, *y = yp;

    if(!rarr_conform(x, y)
       || !rbuf_equal(&x->buf, &y->buf)
       || rarr_is_named(x) != rarr_is_named(y))
        return false;
    return r_equal(x->dimnames, y->dimnames);
}
static void rarr_print(FILE *fp, const void *ptr)
{
    const rarray_t *arr = ptr;
    rtype_t *etyp = elt_type(&arr->buf);
    rvector_t **dimnames = arr->dimnames ? rvec_elts(arr->dimnames) : NULL;

    print_arr_hdr(fp, arr);
    if(rtype_is_scalar(etyp))
        print_arr_scal(fp, etyp->scal, rvec_elts(arr), dimnames,
                       arr->rank, arr->shape);
    else
        print_arr_obj(fp, rvec_elts(arr), dimnames,
                      arr->rank, arr->shape);
}
static const typeops_t arr_ops = {
    .free = rarr_free,
    .gc = rarr_gc,
    .hash = rarr_hash,
    .equal = rarr_equal,
    .print = rarr_print
};
typedef struct
{
    consdesc_t cons;
    rtype_t **ptype;
    const typeops_t *ops;
} cntrdesc_t;
rtype_t *cntr_type_create(const cntrdesc_t *cntr, rtype_t *etyp)
{
    return rtype_create(cntr->cons.kind, etyp, cntr->ops, NULL);
}
static rtype_t *cntr_type_from_spec(const consdesc_t *cons,
                                    unsigned nargs, ast_t *args)
{
    const cntrdesc_t *cntr = container_of(cons, cntrdesc_t, cons);

    if(nargs == 0)
        return *cntr->ptype;
    else if(nargs == 1)
    {
        rtype_t *etyp = rtype_from_spec(&args[0]);
        if(etyp)
            return cntr_type_create(cntr, etyp);
    }
    return NULL;
}
static const cntrdesc_t cntr_vec = {
    .cons = {
        .from_spec = cntr_type_from_spec,
        .kind = RT_VECTOR
    },
    .ptype = &r_type_vector,
    .ops = &vec_ops
};
static const cntrdesc_t cntr_arr = {
    .cons = {
        .from_spec = cntr_type_from_spec,
        .kind = RT_ARRAY
    },
    .ptype = &r_type_array,
    .ops = &arr_ops
};
rtype_t *rvec_type_create(rtype_t *etyp)
    { return cntr_type_create(&cntr_vec, etyp); }
rtype_t *rarr_type_create(rtype_t *etyp)
    { return cntr_type_create(&cntr_arr, etyp); }
static inline rtype_t *rtype_vec_init(rtype_t *elt_type, const char *name)
{
    rtype_t *type = rvec_type_create(elt_type);
    rtype_install(type, name);
    return type;
}
rtype_t *r_type_vector, *r_type_array,
        *r_type_vec_symbol, *r_type_vec_names, *r_type_vec_object,
        *r_type_vec_boolean, *r_type_vec_int, *r_type_vec_double;

void rt_install_vec_types()
{
    r_type_vector = rtype_cons_init(&cntr_vec.cons, "vector");
    r_type_array = rtype_cons_init(&cntr_arr.cons, "array");
    r_type_vec_object = rtype_vec_init(r_type_object, "list");
    r_type_vec_symbol = rtype_vec_init(r_type_symbol, "names");
    r_type_vec_names = rtype_vec_init(r_type_vec_symbol, "dimnames");
    r_type_vec_boolean = rtype_vec_init(r_type_boolean, "logical");
    r_type_vec_int = rtype_vec_init(r_type_int, "integer");
    r_type_vec_double = rtype_vec_init(r_type_double, "numeric");
}
