typedef enum
{
    SC_BOOLEAN = 0,
    SC_INT,
    SC_DOUBLE,
    SC_MAX
} scalar_code;
typedef struct scaldesc
{
    char *name;
    size_t size;
    int (*width)(const void *, int, void *);
    void (*print)(FILE *, const void *, int, void *);
    size_t printsz;
} scaldesc_t;
#define UNBOX(typ, box) *((typ *)BOXPTR(box))
#define BOXPTR(box) ((uint8_t *)(box) + sizeof(robject_t))
static const rint_t rint_na = INT_MIN,
    rint_min = INT_MIN+1,
    rint_max = INT_MAX;
static const rdouble_t rdouble_na = __builtin_nan("0xdeadbeef"),
    rdouble_min = DBL_MIN,
    rdouble_max = DBL_MAX;
static const rboolean_t rboolean_na = ~0;
extern rtype_t *r_type_int, *r_type_double, *r_type_boolean;
extern const scaldesc_t scal_descs[];
robject_t *r_box_create(rtype_t *typ);
robject_t *r_box(rtype_t *typ, const void *value);
void r_unbox(void *dest, robject_t *src);
void rt_install_scalar_types();
static inline scalar_code rscal_code(const rtype_t *typ)
    { return (scalar_code)(typ->scal - scal_descs); }
static inline size_t rscal_size(const rtype_t *typ)
    { return typ->scal->size; }
static inline rtype_t *rscal_promote(rtype_t *xt, rtype_t *yt)
    { return (rscal_code(xt) > rscal_code(yt)) ? xt : yt; }
#define scalprint_init(scal) memset(alloca(scal->printsz), 0, scal->printsz)
