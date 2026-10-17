typedef struct
{
    robject_t base;
    int length;
    void *elts;
} rbuf_t;
typedef struct rvector_s rvector_t;
typedef struct rvector_s
{
    rbuf_t buf;
    rvector_t *names;
} rvector_t;
typedef struct
{
    rbuf_t buf;
    int rank;
    int *shape;
    rvector_t *dimnames;
} rarray_t;
extern rtype_t *r_type_vector, *r_type_array,
    *r_type_vec_symbol, *r_type_vec_names, *r_type_vec_object,
    *r_type_vec_boolean, *r_type_vec_int, *r_type_vec_double;
rtype_t *rvec_type_create(rtype_t *elt_type);
rvector_t *rvec_create(rtype_t *typ, unsigned length);
rvector_t *rvec_add_names(rvector_t *vec);

rtype_t *rarr_type_create(rtype_t *etyp);
rarray_t *rarr_create_buf(rtype_t *typ, int length);
void rarr_set_shape(rarray_t *arr, int rank, int *dims);
rarray_t *rarr_create(rtype_t *typ, int rank, int *dims);
rvector_t *rarr_add_names(rarray_t *arr);

void rt_install_vec_types();
static inline rtype_t *elt_type(const rbuf_t *buf)
{
    rtype_t *typ = r_typeof(buf);
    assert(rtype_is_container(typ) && typ->elt);
    return typ->elt;
}
#define rvec_elts(_b) ((_b)->buf.elts)
#define rvec_len(_b)  ((_b)->buf.length)
static inline bool rvec_is_named(const rvector_t *vec)
    { return vec->names != NULL; }
static inline bool rarr_is_named(const rarray_t *arr)
    { return arr->dimnames != NULL; }
static inline bool is_named(const robject_t *obj)
{
    rtype_t *type = r_typeof(obj);
    if(rtype_is_vector(type))
        return rvec_is_named((rvector_t *)obj);
    else if(rtype_is_array(type))
        return rarr_is_named((rarray_t *)obj);
    return false;
}
static inline bool rarr_shape_conform(int xr, int yr, int *xs, int *ys)
    { return (xr == yr) && !memcmp(xs, ys, sizeof(int) * xr); }
static inline bool rarr_conform(const rarray_t *x, const rarray_t *y)
    { return rarr_shape_conform(x->rank, y->rank, x->shape, y->shape); }
