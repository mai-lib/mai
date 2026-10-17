#include "global.h"
static int boolean_width(const void *ptr, int width, void *data)
{
    rboolean_t v = *(rboolean_t *)ptr;
    int w = (v == rboolean_na) ? 2 : 5-v;
    return max(width, w);
}
static void boolean_print(FILE *fp, const void *ptr, int width, void *data)
{
    rboolean_t v = *(rboolean_t *)ptr;
    if(v == rboolean_na)
        fprintf(fp, "%*s", width, "NA");
    else
        fprintf(fp, "%*s", width, v ? "true" : "false");
}
static int int_width(const void *ptr, int width, void *data)
{
    rint_t v = *(rint_t *)ptr;
    int w;
    if(v == rint_na)
        w = 2;
    else
        w = (v < 0) + floor(log10(abs(v))) + 1;
    return max(width, w);
}
static void int_print(FILE *fp, const void *ptr, int width, void *data)
{
    rint_t v = *(rint_t *)ptr;
    if(v == rint_na)
        fprintf(fp, "%*s", width, "NA");
    else
        fprintf(fp, "%*d", width, v);
}
static inline bool is_na(rdouble_t val)
{
    const uint64_t *pval = (uint64_t *)&val, *pna = (uint64_t *)&rdouble_na;
    return *pval == *pna;
}
typedef struct
{
    unsigned short dec, prec;
} dwidth_t;
static int double_width(const void *ptr, int width, void *data)
{
    rdouble_t val = *(rdouble_t *)ptr;
    dwidth_t *dwidth = data;
    int dec = 0, prec = 0;
    if(is_na(val))
        return max(width, 2);
    else if(!isfinite(val))
        return max(width, 3 + (signbit(val) != 0));
    else
    {
        dec = (signbit(val) != 0);
        val = fabs(val);
        if(val > 1.0)
          dec += floor(log10(val));
        dec++;
        val -= floor(val);
        val /= exp10(-opt.print_digits);
        val = nearbyint(val);
        for(prec = opt.print_digits;
            prec >= 0 && val == floor(val);
            prec--, val /= 10);
        prec++;
        prec = max(prec, 1);
        //printf("= dec %d, prec %d\n", dec, prec);
    }
    dwidth->dec = max(dwidth->dec, dec);
    dwidth->prec = max(dwidth->prec, prec);
    return max(width, dwidth->dec + dwidth->prec + 1);
}
static void double_print(FILE *fp, const void *ptr, int width, void *data)
{
    rdouble_t v = *(rdouble_t *)ptr;
    dwidth_t *dwidth = data;

    assert(dwidth);
    if(is_na(v))
        fprintf(fp, "%*s", width, "NA");
    else
        fprintf(fp, "%*.*f", width, dwidth->prec, v);
}
const scaldesc_t scal_descs[] = {
    [SC_BOOLEAN] = {
        .name = "boolean",
        .size = sizeof(rboolean_t),
        .print = boolean_print,
        .width = boolean_width,
    },
    [SC_INT] = {
        .name = "int",
        .size = sizeof(rint_t),
        .print = int_print,
        .width = int_width,
    },
    [SC_DOUBLE] = {
        .name = "double",
        .size = sizeof(rdouble_t),
        .print = double_print,
        .width = double_width,
        .printsz = sizeof(dwidth_t)
    }
};
static void rscal_print(FILE *fp, const void *ptr)
{
    rtype_t *typ = r_typeof(ptr);
    void *data = scalprint_init(typ->scal);
    int width = typ->scal->width(BOXPTR(ptr), 0, data);

    typ->scal->print(fp, BOXPTR(ptr), width, data);
}
static uint32_t rscal_hash(const void *ptr)
{
    rtype_t *typ = r_typeof(ptr);
    return hash_code_seed(BOXPTR(ptr), rscal_size(typ), r_hash(typ));
}
static bool rscal_equal(const void *x, const void *y)
{
    rtype_t *typ = r_typeof(x);
    return !memcmp(BOXPTR(x), BOXPTR(y), rscal_size(typ));
}
static const typeops_t scal_ops = {
    .hash = rscal_hash,
    .equal = rscal_equal,
    .print = rscal_print
};
robject_t *r_box_create(rtype_t *typ)
{
    assert(rtype_is_scalar(typ));
    return gc_alloc(typ, sizeof(robject_t) + rscal_size(typ));
}
robject_t *r_box(rtype_t *typ, const void *value)
{
    robject_t *box = r_box_create(typ);
    memcpy(BOXPTR(box), value, rscal_size(typ));
    return box;
}
void r_unbox(void *dest, robject_t *src)
{
    rtype_t *typ = r_typeof(src);
    assert(rtype_is_scalar(typ));
    memcpy(dest, BOXPTR(src), rscal_size(typ));
}
static rtype_t *rtype_scal_init(scalar_code code)
{
    const scaldesc_t *scal = &scal_descs[code];
    rtype_t *typ = rtype_create(RT_SCALAR, (void *)scal,
                                &scal_ops, scal->name);
    rtype_install(typ, scal->name);
    return typ;
}
rtype_t *r_type_int, *r_type_double, *r_type_boolean;
void rt_install_scalar_types()
{
    r_type_double = rtype_scal_init(SC_DOUBLE);
    r_type_int = rtype_scal_init(SC_INT);
    r_type_boolean = rtype_scal_init(SC_BOOLEAN);

    r_defscalar("true", r_type_boolean,
                (rvalue_union_t) { .boolean = true } );
    r_defscalar("false", r_type_boolean,
                (rvalue_union_t) { .boolean = false } );
    r_defscalar("NA", r_type_boolean,
                (rvalue_union_t) { .boolean = rboolean_na } );
    r_defscalar("Inf", r_type_double,
                (rvalue_union_t) { .dfloat = HUGE_VAL } );
}
