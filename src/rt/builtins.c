#include "global.h"
#include "rt/builtin.h"
#include "rt/opt_builtins.h"
#include <time.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/stat.h>

static void builtin_rm(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        rsymbol_t *name;
    } end_args(args, a);
    bool ret = r_unset(a->name);
    //if(!ret)
    //    vm_error(vm, "rm", "can't remove global `%s`.", r_symstr(a->name));
    //builtin_return(r, robject_t *, NULL);
    builtin_return(r, rboolean_t, ret);
}

static void builtin_print(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        argbits_t argbits;
        robject_t *x;
    } end_args(args, a);
    argbits_t ab = a->argbits; // TEMP

    if(!missingp(ab, 0))
        r_print(stdout, a->x);
    fprintf(stdout, "\n");

    builtin_return(r, robject_t *, NULL);
}

static void builtin_eq(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        robject_t *x, *y;
    } end_args(args, a);
    builtin_return(r, rboolean_t, (a->x == a->y));
}

static void builtin_equal(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        robject_t *x, *y;
    } end_args(args, a);
    builtin_return(r, rboolean_t, r_equal(a->x, a->y));
}

// ---

static void builtin_typeof(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        robject_t *obj;
    } end_args(args, a);
    builtin_return(r, rtype_t *, r_typeof(a->obj));
}

static rtype_t *common_type(rtype_t *self, rtype_t *other)
{
    if(self && other)
        return r_common_type(self, other);
    return NULL;
}

static void builtin_common_type(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        rtype_t *self, *other;
    } end_args(args, a);
    builtin_return(r, rtype_t *, common_type(a->self, a->other));
}

static rtype_t *r_ret_type(const void  *arg)
{
    const rtype_t *typ = arg;
    if(rtype_is_callable(typ))
        return typ->sig->ret_type;
    else if(typ == r_type_callable)
        return r_type_object;
    return NULL;
}

static void builtin_ret_type(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        rtype_t *typ;
    } end_args(args, a);
    rtype_t *ret = r_ret_type(a->typ);
    if(!ret)
        vm_error(vm, "ret_type", "invalid argument.");
    builtin_return(r, rtype_t *, ret);
}

static rtype_t *r_elt_type(const void *arg)
{
    const rtype_t *typ = arg;
    if(rtype_is_container(typ))
        return typ->elt;
    else if(typ == r_type_vector || typ == r_type_array)
        return r_type_object;
    return NULL;
}

static void builtin_elt_type(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        rtype_t *typ;
    } end_args(args, a);
    rtype_t *ret = r_elt_type(a->typ);
    if(!ret)
        vm_error(vm, "elt_type", "invalid argument.");
    builtin_return(r, rtype_t *, ret);
}

// ---

static void builtin_subtype(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        rtype_t *self, *other;
    } end_args(args, a);
    if(!a->self || !a->other)
        vm_error(vm, "subtype", "invalid argument.");
    builtin_return(r, rboolean_t, r_subtypep(a->self, a->other));
}

static void builtin_compat(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        rtype_t *self, *other;
    } end_args(args, a);
    if(!a->self || !a->other)
        vm_error(vm, "compat", "invalid argument.");
    builtin_return(r, rint_t, r_type_compat(a->self, a->other, true));
}

static void builtin_hash(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        robject_t *x;
    } end_args(args, a);
    builtin_return(r, rint_t, r_hash(a->x));
}

static inline double tv_to_sec(struct timeval *tv)
{
    return tv->tv_sec + tv->tv_usec/(double)1000000;
}

// { real, user, sys } times in seconds
static void builtin_time(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    rvector_t *res = vm_retain(vm, rvec_create(r_type_vec_double, 3));
    rdouble_t *times = rvec_elts(res);
    rsymbol_t **names = rvec_elts(rvec_add_names(res));
    struct rusage ru;
    struct timeval tv;

    getrusage(RUSAGE_SELF, &ru);
    gettimeofday(&tv, NULL);
    times[0] = tv_to_sec(&tv);
    times[1] = tv_to_sec(&ru.ru_utime);
    times[2] = tv_to_sec(&ru.ru_stime);
    names[0] = r_intern("real");
    names[1] = r_intern("user");
    names[2] = r_intern("system");
    builtin_return(r, rvector_t *, res);
    vm_release(vm, 1);
}

#define clamp_int(v) ((v) > INT_MAX ? rint_na : (rint_t)(v))

static void builtin_gc(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    rvector_t *res = vm_retain(vm, rvec_create(r_type_vec_int, 4));
    rint_t *counts = rvec_elts(res);
    rsymbol_t **names = rvec_elts(rvec_add_names(res));
    gc_stats_t stats;

    gc_collect();
    gc_collect_stats(&stats);
    counts[0] = clamp_int(stats.ncollect);
    counts[1] = clamp_int(stats.nchunks);
    counts[2] = clamp_int(stats.obj_bytes/1024.0);
    counts[3] = clamp_int(stats.vec_bytes/1024.0);
    names[0] = r_intern("collections");
    names[1] = r_intern("chunks");
    names[2] = r_intern("object KB");
    names[3] = r_intern("vector KB");
    builtin_return(r, rvector_t *, res);
    vm_release(vm, 1);
}

static void builtin_rand(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        rint_t min, max;
    } end_args(args, a);

    if(a->max == rint_na || a->min == rint_na || a->max <= a->min)
        vm_error(vm, "rand", "invalid argument.");
    unsigned range = (unsigned)((int64_t)a->max - a->min) + 1;
    builtin_return(r, rint_t, (rint_t)(unif_rand_u32() % range + a->min));
}

static void builtin_srand(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        argbits_t argbits;
        rint_t seed; // optional
    } end_args(args, a);
    argbits_t ab = a->argbits; // TEMP
    unsigned long long seed;

    if(!missingp(ab, 0))
    {
        if(a->seed == rint_na)
            vm_error(vm, "srand", "invalid argument.");
        seed = (unsigned)a->seed;
    }
    else
    {
        time_t tval = time(NULL);
        if(tval == (time_t)-1)
            vm_error(vm, "srand", "time(2) error."); // wat.
        seed = tval;
    }
    init_rand(seed);
    builtin_return(r, robject_t *, NULL);
}

static void builtin_source(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        rstring_t *name;
    } end_args(args, a);
    rclosure_t *cl;

    // the compiler is somewhat cavalier about managed pointers
    gc_set_enabled(false);
    // read and compile file
    cl = source(a->name->string);
    // closure is rooted on return
    gc_set_enabled(true);
    builtin_return(r, rclosure_t *, cl);
}

static void builtin_stop(vm_ctx_t *vm, rbuiltin_t *fn, uint8_t *args, void *r)
{
    def_args {
        rstring_t *msg;
    } end_args(args, a);
    vm_error(vm, "stop", "%s", a->msg ? a->msg->string : "");
    // does not return
}

static void builtin_assert(vm_ctx_t *vm, rbuiltin_t *fn,
                           uint8_t *args, void *r)
{
    def_args {
        rboolean_t test;
    } end_args(args, a);
    if(a->test != true)
        vm_error(vm, "assert", "assertion failed.");
    builtin_return(r, robject_t *, NULL);
}

// raw binary vector I/O, so other programs (e.g. R) can exchange data with
// mai without going through text. elements are written in native byte
// order: int as 32-bit, double as 64-bit IEEE, boolean as one byte.
static rtype_t *io_vec_type(rtype_t *etyp)
{
    if(etyp == r_type_double)
        return r_type_vec_double;
    if(etyp == r_type_int)
        return r_type_vec_int;
    if(etyp == r_type_boolean)
        return r_type_vec_boolean;
    return NULL;
}

static void builtin_read_vec(vm_ctx_t *vm, rbuiltin_t *fn,
                             uint8_t *args, void *r)
{
    def_args {
        rstring_t *file;
        rtype_t *etyp;
    } end_args(args, a);
    rtype_t *vtyp = a->etyp ? io_vec_type(a->etyp) : NULL;
    struct stat sbuf;
    FILE *fp;

    if(!a->file || !vtyp)
        vm_error(vm, "read_vec", "invalid argument.");
    size_t eltsz = rtype_eltsz(a->etyp);
    if(stat(a->file->string, &sbuf) < 0 || !(fp = fopen(a->file->string, "rb")))
        vm_error(vm, "read_vec", "can't open `%s`.", a->file->string);
    if(sbuf.st_size % eltsz || sbuf.st_size / eltsz > INT_MAX)
    {
        fclose(fp);
        vm_error(vm, "read_vec", "size of `%s` is not a whole number "
                 "of elements.", a->file->string);
    }
    size_t n = sbuf.st_size / eltsz;
    rvector_t *vec = rvec_create(vtyp, n);
    size_t got = fread(rvec_elts(vec), eltsz, n, fp);
    fclose(fp);
    if(got != n)
        vm_error(vm, "read_vec", "error reading `%s`.", a->file->string);
    builtin_return(r, rvector_t *, vec);
}

static void builtin_write_vec(vm_ctx_t *vm, rbuiltin_t *fn,
                              uint8_t *args, void *r)
{
    def_args {
        robject_t *x;
        rstring_t *file;
    } end_args(args, a);
    rtype_t *typ = a->x ? r_typeof(a->x) : NULL;
    FILE *fp;

    if(!a->file || !typ || !rtype_is_container(typ)
       || !io_vec_type(typ->elt))
        vm_error(vm, "write_vec", "invalid argument.");
    rbuf_t *buf = (rbuf_t *)a->x;
    size_t eltsz = rtype_eltsz(typ->elt);
    if(!(fp = fopen(a->file->string, "wb")))
        vm_error(vm, "write_vec", "can't open `%s`.", a->file->string);
    size_t put = fwrite(buf->elts, eltsz, buf->length, fp);
    if(fclose(fp) != 0 || put != (size_t)buf->length)
        vm_error(vm, "write_vec", "error writing `%s`.", a->file->string);
    builtin_return(r, rint_t, buf->length);
}

const builtin_init_t runtime_builtins[] = {
    defbuiltin("rm", builtin_rm, &r_type_boolean,
               { "name", &r_type_symbol }),
    defcbuiltin("eq", builtin_eq, &r_type_boolean,
               &pure_ops, NULL, // pointers are immutable
               { "x", &r_type_object },
               { "y", &r_type_object }),
    defbuiltin("equal", builtin_equal, &r_type_boolean,
               { "x", &r_type_object },
               { "y", &r_type_object }),
    defbuiltin("print", builtin_print, &r_type_object,
               { "x", &r_type_object, true }),
    defbuiltin("rt_time", builtin_time, &r_type_object,
               { }),
    defbuiltin("rand", builtin_rand, &r_type_int,
               { "min", &r_type_int },
               { "max", &r_type_int }),
    defbuiltin("srand", builtin_srand, &r_type_object,
               { "seed", &r_type_int, true }),
    defcbuiltin("typeof", builtin_typeof, &r_type_type,
               &typeof_ops, NULL,
               { "obj", &r_type_object }),
    defcbuiltin("common_type", builtin_common_type, &r_type_type,
               &type_ops, type_binop(common_type),
               { "self", &r_type_type },
               { "other", &r_type_type }),
    defcbuiltin("ret_type", builtin_ret_type, &r_type_type,
               &type_ops, type_unop(r_ret_type),
               { "type", &r_type_type }),
    defcbuiltin("elt_type", builtin_elt_type, &r_type_type,
               &type_ops, type_unop(r_elt_type),
               { "type", &r_type_type }),
    defcbuiltin("subtype", builtin_subtype, &r_type_boolean,
               &pure_ops, NULL, // types are immutable
               { "self", &r_type_type },
               { "other", &r_type_type }),
    defcbuiltin("compat", builtin_compat, &r_type_int,
               &pure_ops, NULL, // types are immutable
               { "self", &r_type_type },
               { "other", &r_type_type }),
    defbuiltin("hash", builtin_hash, &r_type_int,
               { "x", &r_type_object }),
    defbuiltin("gc_collect", builtin_gc, &r_type_object,
               { }),
    defbuiltin("source", builtin_source, &r_type_callable,
               { "file", &r_type_string }),
    defbuiltin("stop", builtin_stop, &r_type_object,
               { "message", &r_type_string }),
    defbuiltin("assert", builtin_assert, &r_type_object,
               { "expr", &r_type_boolean }),
    defbuiltin("read_vec", builtin_read_vec, &r_type_vector,
               { "file", &r_type_string },
               { "type", &r_type_type }),
    defbuiltin("write_vec", builtin_write_vec, &r_type_int,
               { "x", &r_type_object },
               { "file", &r_type_string }),
    { }
};
