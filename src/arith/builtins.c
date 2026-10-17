#include "global.h"
#include "ir.h"
#include "rt/builtin.h"
#include "vm/vm_ops.h"
#include "arith/scalar.h"
#include "arith/vec.h"
#include "arith/dispatch.h"

static void builtin_binary(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        robject_t *x, *y;
    } end_args(args, a);
    const arith_builtin_t *ab = fn->cbi->data;
    robject_t *res = arith_binary(vm, ab->fntab, ab->is_pred, a->x, a->y);
    builtin_return(r, robject_t *, res);
}

static void builtin_unary(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        robject_t *x;
    } end_args(args, a);
    const arith_builtin_t *ab = fn->cbi->data;
    robject_t *res = arith_unary(vm, ab->fntab, ab->is_pred, a->x);
    builtin_return(r, robject_t *, res);
}

static void builtin_reduce(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        robject_t *x;
    } end_args(args, a);
    const arith_builtin_t *ab = fn->cbi->data;
    robject_t *res = arith_reduce(vm, ab->fntab, a->x);
    builtin_return(r, robject_t *, res);
}

static void builtin_convert(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        robject_t *x;
    } end_args(args, a);
    rtype_t *etyp = *(rtype_t **)fn->cbi->data; // note: not an arith_builtin_t
    robject_t *res = arith_convert(vm, a->x, etyp);
    builtin_return(r, robject_t *, res);
}

static void builtin_coerce(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        robject_t *x;
        rtype_t *etyp;
    } end_args(args, a);
    if(!rtype_is_scalar(a->etyp))
        vm_error(vm, "as_type", "invalid argument type.");
    robject_t *res = arith_convert(vm, a->x, a->etyp);
    builtin_return(r, robject_t *, res);
}

static inline bool fill_scal(rbuf_t *dest, robject_t *val, rtype_t *etyp, rtype_t *typ)
{
    rvalue_union_t temp;
    size_t esz = rtype_eltsz(etyp);
    int len = dest->length;
    uint8_t *ptr = dest->elts;

    // arithmetic conversion if needed
    if(typ == etyp)
        r_unbox(&temp, val);
    else if(!rtype_is_scalar(typ) || !scalar_convert(&temp, val, etyp, typ))
        return false;
    for(int i=0; i<len; i++, ptr+=esz)
        memcpy(ptr, &temp, esz);
    return true;
}

static inline bool fill_ptr(rbuf_t *dest, robject_t *val, rtype_t *etyp, rtype_t *typ)
{
    robject_t **ptr = dest->elts;
    int len = dest->length;

    if(!r_subtypep(typ, etyp))
        return false;
    for(int i=0; i<len; i++, ptr++)
        *ptr = val;
    return true;
}

static inline bool fill_buf(rbuf_t *dest, robject_t *val)
{
    rtype_t *etyp = elt_type(dest);
    rtype_t *typ = r_typeof(val);

    if(rtype_is_scalar(etyp))
        return fill_scal(dest, val, etyp, typ);
    return fill_ptr(dest, val, etyp, typ);
}

static inline void fill_buf_from(rbuf_t *dest, rbuf_t *src)
{
    copy_recycle(dest->elts, src->elts, dest->length, src->length,
                 rtype_eltsz(elt_type(dest)));
}

// create a vector of given element type and length, retaining both
// (i.e. +2 release)
static rvector_t *safe_vec_create(vm_ctx_t *vm, rtype_t *etyp, unsigned length)
{
    rtype_t *vtyp = vm_retain(vm, rvec_type_create(etyp));
    return vm_retain(vm, rvec_create(vtyp, length));
}

static rarray_t *safe_arr_create(vm_ctx_t *vm, rtype_t *etyp, int rank, int *dims)
{
    rtype_t *atyp = vm_retain(vm, rarr_type_create(etyp));
    return vm_retain(vm, rarr_create(atyp, rank, dims));
}

// extract the types of each pointer in elts[nelts]
// return the least(ish!) common supertype of them all
static rtype_t *examine_args(void **elts, int nelts)
{
    rtype_t *etyp = NULL;
    for(int i=0; i<nelts; i++)
    {
        rtype_t *typ = r_typeof(elts[i]);
        if(!etyp)
            etyp = typ;
        else
            etyp = r_common_type(typ, etyp);
    }
    return etyp ? etyp : r_type_object;
}

// mimicking R would mean recursive descent into member vectors during
// element type determination and copying. we don't do this.

// creates a new vector containing the elements of the rest vector.
// if they're all the same type (or are scalars so can be converted),
// the result has that element type.
static void builtin_vec(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        rvector_t *rest;
    } end_args(args, a);

    if(!a->rest)
    {
        builtin_return(r, robject_t *, NULL);
        return;
    }

    rvector_t *src = a->rest;
    rtype_t *etyp = examine_args(rvec_elts(src), rvec_len(src));
    // XXX move to arith.c??
    if(rtype_is_scalar(etyp))
    {
        robject_t *res = arith_convert(vm, (robject_t *)src, etyp);
        builtin_return(r, robject_t *, res);
    }
    else
    {
        rvector_t *dest = safe_vec_create(vm, etyp, rvec_len(src));
        memcpy(rvec_elts(dest), rvec_elts(src), rvec_len(src) * sizeof(robject_t *));
        // XXX don't bother, rest list construction does the wrong thing
        // with named args, due to two-phase arg matching
        if(is_named((robject_t *)src))
            copy_names((robject_t *)dest, (robject_t *)src);
        builtin_return(r, rvector_t *, dest);
        vm_release(vm, 2);
    }
}

static rvector_t *typed_vec_init(vm_ctx_t *vm, rtype_t *etyp, int len, robject_t *val)
{
    rvector_t *vec = safe_vec_create(vm, etyp, len);

    if(!val)
        memset(rvec_elts(vec), 0, len * rtype_eltsz(etyp));
    else if(!fill_buf(&vec->buf, val))
        vm_error(vm, "vec_create", "invalid argument type.");
    vm_release(vm, 2);
    return vec;
}

// XXX rationalise these; we want a minimal, orthogonal, useful set of
// constructors
static void builtin_typed_vec(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        argbits_t argbits;
        rint_t len;
        robject_t *val; // optional
    } end_args(args, a);
    argbits_t ab = a->argbits; // TEMP

    if(a->len < 0 || a->len == rint_na)
        vm_error(vm, "typed_vec", "invalid argument.");

    rtype_t *etyp = *(rtype_t **)fn->cbi->data;
    rvector_t *res = typed_vec_init(vm, etyp, a->len, !missingp(ab, 1) ? a->val : NULL);
    builtin_return(r, rvector_t *, res);
}

static void builtin_vector(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        argbits_t argbits;
        rtype_t *etyp;
        rint_t len;
        robject_t *val; // optional
    } end_args(args, a);
    argbits_t ab = a->argbits; // TEMP

    if(a->len < 0 || a->len == rint_na)
        vm_error(vm, "vector", "invalid argument.");

    rvector_t *res = typed_vec_init(vm, a->etyp, a->len, !missingp(ab, 2) ? a->val : NULL);
    builtin_return(r, rvector_t *, res);
}

// mimicks some R behaviour:
// length(anything except a container) = 1
// length(NULL) = 0
static void builtin_length(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        robject_t *x;
    } end_args(args, a);
    rint_t len;
    if(a->x)
    {
        rtype_t *typ = r_typeof(a->x);
        if(rtype_is_container(typ))
            len = ((rbuf_t *)a->x)->length;
        else
            len = 1;
    }
    else
        len = 0;
    builtin_return(r, rint_t, len);
}

static void builtin_names_get(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        rvector_t *x;
    } end_args(args, a);
    // names(nil) is nil
    rvector_t *res = a->x ? a->x->names : NULL;
    builtin_return(r, rvector_t *, res);
}

static void builtin_names_set(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        rvector_t *x;
        rvector_t *names;
    } end_args(args, a);

    if(!a->x)
        vm_error(vm, "names=", "invalid argument.");
    if(a->names)
    {
        assert(r_typeof(a->names) == r_type_vec_symbol);
        if(rvec_len(a->names) != rvec_len(a->x))
            vm_error(vm, "names=", "invalid argument length.");
    }
    a->x->names = a->names;
    builtin_return(r, rvector_t *, a->names);
}

static rvector_t *copy_dimnames(vm_ctx_t *vm, rvector_t *src)
{
    int len = rvec_len(src);
    rvector_t **srcnames = rvec_elts(src);
    rvector_t *dest = safe_vec_create(vm, r_type_vec_symbol, len);
    rvector_t **destnames = rvec_elts(dest);

    memcpy(destnames, srcnames, sizeof(rvector_t *) * len);
    vm_release(vm, 2);
    return dest;
}

static void builtin_dimnames_get(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        rarray_t *x;
    } end_args(args, a);
    // mutation could break the shape invariant! must take a copy.
    rvector_t *res = NULL;
    if(a->x && a->x->dimnames)
        res = copy_dimnames(vm, a->x->dimnames);
    builtin_return(r, rvector_t *, res);
}

static void builtin_dimnames_set(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        rarray_t *x;
        rvector_t *dimnames;
    } end_args(args, a);

    if(!a->x)
        vm_error(vm, "dimnames=", "invalid argument.");
    if(a->dimnames)
    {
        if(r_typeof(a->dimnames) != r_type_vec_names)
            vm_error(vm, "dimnames=", "invalid argument type.");
        if(rvec_len(a->dimnames) != a->x->rank)
            vm_error(vm, "dimnames=", "invalid argument length.");

        rvector_t **dimnames = rvec_elts(a->dimnames);
        for(int i=0; i<a->x->rank; i++)
        {
            if(dimnames[i])
            {
                if(r_typeof(dimnames[i]) != r_type_vec_symbol)
                    vm_error(vm, "dimnames=", "invalid argument type.");
                if(rvec_len(dimnames[i]) != a->x->shape[i])
                    vm_error(vm, "dimnames=", "invalid argument length.");
            }
        }
    }
    a->x->dimnames = copy_dimnames(vm, a->dimnames);
    builtin_return(r, rvector_t *, a->dimnames);
}

static void builtin_fetch(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        argbits_t argbits;
        robject_t *obj;
        robject_t *idx; // optional
        rvector_t *rest; // optional
    } end_args(args, a);
    robject_t *res;

    // not an R-ism: nil[anything] is an error
    if(!a->obj)
        vm_error(vm, "[", "invalid subscript of nil.");
    if(!a->rest)
        res = vec_fetch(vm, a->obj, a->idx);
    else
        res = arr_fetch(vm, a->obj, a->idx,
                        rvec_elts(a->rest), rvec_len(a->rest));
    builtin_return(r, robject_t *, res);
}

static void builtin_store(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        argbits_t argbits;
        robject_t *obj;
        robject_t *val;
        robject_t *idx; // optional
        rvector_t *rest; // optional
    } end_args(args, a);
    robject_t *res;

    // R-ism: nil[anything] = anything is an error
    if(!a->obj)
        vm_error(vm, "[=", "invalid assignment to nil.");
    if(!a->rest)
        res = vec_store(vm, a->obj, a->val, a->idx);
    else
        res = arr_store(vm, a->obj, a->val, a->idx,
                        rvec_elts(a->rest), rvec_len(a->rest));
    builtin_return(r, robject_t *, res);
}

// XXX this is not very useful.
static void builtin_vec_cat(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        rvector_t *rest;
    } end_args(args, a);
    int len = 0;
    rvector_t *rest = a->rest;
    rtype_t *etyp = NULL;

    if(!rest)
    {
        builtin_return(r, robject_t *, NULL);
        return;
    }

    rvector_t **vecs = rvec_elts(rest);
    int nrest = rvec_len(rest);

    for(int i=0; i < nrest; i++)
    {
        if(!rtype_is_vector(r_typeof(vecs[i])))
            vm_error(vm, "cat", "can't cat things which aren't vectors.");
        if(i == 0)
            etyp = elt_type(&vecs[i]->buf);
        else if(etyp != elt_type(&vecs[i]->buf))
            vm_error(vm, "cat", "all arguments must be of the same type.");
        len += rvec_len(vecs[i]);
    }

    rvector_t *dest = safe_vec_create(vm, etyp, len);
    uint8_t *destptr = rvec_elts(dest);

    for(int i=0; i < nrest; i++)
    {
        int alen = rvec_len(vecs[i]);
        size_t sz = alen * rtype_eltsz(etyp);

        memcpy(destptr, rvec_elts(vecs[i]), sz);
        destptr += sz;
    }

    // XXX concatenate names
    builtin_return(r, rvector_t *, dest);
    vm_release(vm, 2);
}

static void builtin_runif(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        argbits_t argbits;
        rint_t n;
        rdouble_t min, max; // both optional
    } end_args(args, a);
    argbits_t ab = a->argbits; // TEMP

    if(isnan(a->min) || isnan(a->max) || a->n < 0
       || a->n == rint_na || a->max < a->min)
        vm_error(vm, "runif", "invalid argument.");

    rvector_t *res = safe_vec_create(vm, r_type_double, a->n);
    rdouble_t *vals = rvec_elts(res);

    if(missingp(ab, 1))
        a->min = 0;
    if(missingp(ab, 2))
        a->max = 1;

    rdouble_t span = a->max - a->min;
    for(int i=0; i<a->n; i++)
        vals[i] = unif_rand() * span + a->min;
    builtin_return(r, rvector_t *, res);
    vm_release(vm, 2);
}

static void builtin_rnorm(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        argbits_t argbits;
        rint_t n;
        rdouble_t mu, sigma; // both optional
    } end_args(args, a);
    argbits_t ab = a->argbits; // TEMP

    if(a->n < 0 || a->n == rint_na)
        vm_error(vm, "rnorm", "invalid argument.");

    rvector_t *res = safe_vec_create(vm, r_type_double, a->n);
    rdouble_t *vals = rvec_elts(res);

    if(missingp(ab, 1))
        a->mu = 0;
    if(missingp(ab, 2))
        a->sigma = 1;
    for(int i=0; i<a->n; i++)
        vals[i] = rnorm(a->mu, a->sigma);
    builtin_return(r, rvector_t *, res);
    vm_release(vm, 2);
}


static void builtin_range(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        rint_t from, to;
    } end_args(args, a);

    if(a->from == rint_na || a->to == rint_na)
        vm_error(vm, ":", "invalid argument.");

    int step = (a->from <= a->to) ? 1 : -1;
    int len = abs(a->to - a->from) + 1;
    rvector_t *res = safe_vec_create(vm, r_type_int, len);
    rint_t *vals = rvec_elts(res);

    for(int i = a->from; i != a->to + step; i += step, vals++)
        *vals = i;

    builtin_return(r, rvector_t *, res);
    vm_release(vm, 2);
}

static rarray_t *typed_arr_init(vm_ctx_t *vm, rtype_t *etyp, int rank,
                                int *dims, robject_t *val)
{
    rarray_t *arr = safe_arr_create(vm, etyp, rank, dims);
    rtype_t *typ = r_typeof(val);

    // all-bits-zero by default
    if(!val)
        memset(rvec_elts(arr), 0, rvec_len(arr) * rtype_eltsz(etyp));
    // vector value and its elements are the right type; recycle along length if needed
    else if(rtype_is_container(typ) && typ->elt == etyp)
        fill_buf_from(&arr->buf, (rbuf_t *)val);
    // single value, initialise all cells with it
    else if(!fill_buf(&arr->buf, val))
        vm_error(vm, "array", "invalid argument type.");
    vm_release(vm, 2);
    return arr;
}

static void builtin_array(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        argbits_t argbits;
        rtype_t *etyp;
        rvector_t *dims;
        robject_t *val; // optional
    } end_args(args, a);
    argbits_t ab = a->argbits; // TEMP

    if(!a->dims || !a->etyp)
        vm_error(vm, "array", "invalid argument.");

    // rank is length(dims)
    int rank = rvec_len(a->dims),
        *dims = rvec_elts(a->dims);

    // elements must be nonnegative and not NA
    for(int i=0; i < rank; i++)
        if(dims[i] < 0 || dims[i] == rint_na)
            vm_error(vm, "array", "invalid dimension.");

    rarray_t *res = typed_arr_init(vm, a->etyp, rank, dims,
                                   !missingp(ab, 2) ? a->val : NULL);
    builtin_return(r, rarray_t *, res);
}

static void builtin_shape(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        robject_t *x;
    } end_args(args, a);

    rtype_t *type = r_typeof(a->x);
    rvector_t *res = NULL;

    if(rtype_is_array(type))
    {
        rarray_t *arr = (rarray_t *)a->x;

        res = safe_vec_create(vm, r_type_int, arr->rank);
        memcpy(rvec_elts(res), arr->shape, arr->rank * sizeof(int));
        vm_release(vm, 2);
    }
    else
    {
        int len;

        if(rtype_is_vector(type))
            len = ((rbuf_t *)a->x)->length;
        else
            len = 1;
        res = safe_vec_create(vm, r_type_int, 1);
        *(int *)rvec_elts(res) = len;
        vm_release(vm, 2);
    }
    builtin_return(r, rvector_t*, res);
}

static void builtin_rank(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        robject_t *x;
    } end_args(args, a);

    rtype_t *type = r_typeof(a->x);
    rint_t rank;

    if(rtype_is_array(type))
        rank = ((rarray_t *)a->x)->rank;
    else if(rtype_is_vector(type))
        rank = 1;
    else
        rank = 0;
    builtin_return(r, rint_t, rank);
}


const static builtin_init_arg_t binary_args[] =
    init_args({ "x", &r_type_object },
              { "y", &r_type_object });
const static builtin_init_arg_t atan2_args[] =
    init_args({ "y", &r_type_object },
              { "x", &r_type_object });
const static builtin_init_arg_t unary_args[] =
    init_args({ "x", &r_type_object });
const static builtin_init_arg_t create_args[] =
    init_args({ "len", &r_type_int, false },
              { "val", &r_type_object, true });

const extern builtin_ops_t binary_ops, unary_ops, binary_boolean_ops,
    unary_boolean_ops, conv_ops, reduce_ops, fetch_ops, store_ops,
    vector_ops, array_ops, pure_ops;

#define init_arith(ft, p, c) \
    &(arith_builtin_t) { .fntab = ft, .is_pred = p, .op = c }

#define _defarith(n, f, a, o, ft, p, c) \
    { init_builtin(n, f, &r_type_object, a), \
      .cbi = init_cbi(o, n, init_arith(ft, p, c)) }

#define defoper(name, op, pred, opcode, n, k...) \
    _defarith(name, builtin_##n##ary,  n##ary_args, &n##ary##k##_ops, \
              PASTE2(op, _funcs), pred, opcode)

// these have cbuiltins for type recovery, but don't generate VM instructions
#define defbinary(name, op, pred) defoper(name, op, pred, 0, bin)
#define defunary(name, op, pred) defoper(name, op, pred, 0, un)
// these have instructions to go with them
#define defbin_ins(name, op, pred) defoper(name, op, pred, ENUM(op), bin)
#define defuna_ins(name, op, pred) defoper(name, op, pred, ENUM(op), un)
// these ones are never predicates, but need a different .cbi.ops
#define defbin_bool(name, op) defoper(name, op, false, ENUM(op, boolean), bin, _boolean)
#define defuna_bool(name, op) defoper(name, op, false, ENUM(op, boolean), un, _boolean)

// the function and the reduction operator have different names
#define defreduce(name, op, pred) \
    _defarith(name, builtin_reduce, unary_args, &reduce_ops, \
              PASTE3(reduce_, op, _funcs), pred, 0)

// .cbi is needed for these; the builtin_ function looks at .cbi.data to
// know what type it was called at

// converts its input to have element type t
#define defconv(n, t) \
    { init_builtin(n, builtin_convert, &r_type_object, unary_args), \
      .cbi = init_cbi(&conv_ops, n, &RTYPE(t)) }
// makes a new vector of type t
#define defcreate(n, t) \
    { init_builtin(n, builtin_typed_vec, &r_type_vec_##t, create_args), \
      .cbi = init_cbi(NULL, n, &RTYPE(t)) }
// FIXME: generate these from the m4 _ops lists
const builtin_init_t arith_builtins[] = {
    defbin_ins("+", add, false),
    defbin_ins("-", sub, false),
    defbin_ins("*", mul, false),
    defbin_ins("/", div, false),

    defbin_bool("&", and),
    defbin_bool("|", or),

    defbin_ins("==", eql, true),
    defbin_ins("!=", neq, true),
    defbin_ins("<", lth, true),
    defbin_ins("<=", lte, true),
    defbin_ins(">", gth, true),
    defbin_ins(">=", gte, true),

    defbin_ins("^", pow, false),
    // because the arguments need to be named "y", "x"...
    _defarith("atan2", builtin_binary,  atan2_args,
              &binary_ops, atan2_funcs, false, 0),

    defuna_bool("!", not),
    defuna_ins("0-", neg, false),

    defuna_ins("is_na", is_na, true),

    defunary("sqrt", sqrt, false),
    defunary("sin", sin, false),
    defunary("cos", cos, false),
    defunary("tan", tan, false),
    defunary("asin", asin, false),
    defunary("acos", acos, false),
    defunary("atan", atan, false),
    defunary("log", log, false),
    defunary("exp", exp, false),
    defunary("round", round, false),

    defreduce("any", or, true),
    defreduce("all", and, true),
    defreduce("sum", add, false),
    defreduce("prod", mul, false),

    defconv("as_bool", boolean),
    defconv("as_int", int),
    defconv("as_double", double),

    defcreate("bool", boolean),
    defcreate("int", int),
    defcreate("dbl", double),
    defcreate("list", object), // slight abuse of notation

    defbuiltin("as_type", builtin_coerce, &r_type_object,
               { "x", &r_type_object },
               { "type", &r_type_type }),
    defbuiltin("vec", builtin_vec, &r_type_vector,
               { "...", &r_type_vec_object, true }),
    defcbuiltin("vector", builtin_vector, &r_type_vector,
                &vector_ops, NULL,
               { "etyp", &r_type_type, false },
               { "len", &r_type_int, false },
               { "val", &r_type_object, true }),
    defcbuiltin("length", builtin_length, &r_type_int,
                &pure_ops, NULL,
               { "x", &r_type_object, false }),
    defbuiltin("names", builtin_names_get, &r_type_vec_symbol,
               { "x", &r_type_vector, false }),
    defbuiltin("names=", builtin_names_set, &r_type_vec_symbol,
               { "x", &r_type_vector, false },
               { "names", &r_type_vec_symbol, false }),
    defbuiltin("dimnames", builtin_dimnames_get, &r_type_vec_names,
               { "x", &r_type_array, false }),
    defbuiltin("dimnames=", builtin_dimnames_set, &r_type_vec_names,
               { "x", &r_type_array, false },
               { "dimnames", &r_type_vec_names, false }),
    defbuiltin("cat", builtin_vec_cat, &r_type_vector,
               { "...", &r_type_vec_object, true }),
    defbuiltin("runif", builtin_runif, &r_type_vec_double,
               { "n", &r_type_int, false },
               { "min", &r_type_double, true },
               { "max", &r_type_double, true }),
    defbuiltin("rnorm", builtin_rnorm, &r_type_vec_double,
               { "n", &r_type_int, false },
               { "mu", &r_type_double, true },
               { "sigma", &r_type_double, true }),
    defcbuiltin("array", builtin_array, &r_type_array,
                &array_ops, NULL,
               { "etyp", &r_type_type, false },
               { "dims", &r_type_vec_int, false },
               { "val", &r_type_object, true }),
    defbuiltin("shape", builtin_shape, &r_type_vec_int,
               { "x", &r_type_object, false }),
    defcbuiltin("rank", builtin_rank, &r_type_int,
                &pure_ops, NULL,
               { "x", &r_type_object, false }),
    defbuiltin(":", builtin_range, &r_type_vec_int,
               { "from", &r_type_int, false },
               { "to", &r_type_int, false }),
    defcbuiltin("[", builtin_fetch, &r_type_object,
                &fetch_ops, NULL,
               { "vec", &r_type_object, false },
               { "i", &r_type_object, true },
               { "...", &r_type_vec_object, true }),
    defcbuiltin("[=", builtin_store, &r_type_object,
                &store_ops, NULL,
               { "vec", &r_type_object, false },
               { "val", &r_type_object, false },
               { "i", &r_type_object, true },
               { "...", &r_type_vec_object, true }),
    { }
};
