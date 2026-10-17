typedef enum
{
    BASE_8 = SC_BOOLEAN,
    BASE_32 = SC_INT,
    BASE_64 = SC_DOUBLE,
    BASE_PTR,
    BASE_ARG,
    BASE_HINT,
    BASE_NONE
} locbase;
#define LOCAL_BASES BASE_ARG
#define SCALAR_BASES (BASE_64+1)
#define UNASSIGNED BASE_HINT
typedef struct
{
    locbase base;
    unsigned index;
} loc_t;
static inline locbase base_for(cnode_t *node)
{
    rtype_t *type = decl_type(node->decl);

    return rtype_is_scalar(type)
        ? rscal_code(type)
        : BASE_PTR;
}
static inline size_t size_base(locbase base)
{
    static const int base_sizes[] = { 1, 4, 8, sizeof(robject_t *) };
    return base_sizes[base];
}
