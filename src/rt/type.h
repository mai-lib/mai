typedef enum
{
    RT_OBJECT, RT_SCALAR, RT_TYPECONS,
    RT_CALLABLE, RT_VECTOR, RT_ARRAY, RT_CELL,
} type_kind;
typedef struct ast ast_t;
typedef struct consdesc consdesc_t;
typedef rtype_t *(*typecons_fn)(const consdesc_t *, unsigned, ast_t *);
typedef struct consdesc
{
    typecons_fn from_spec;
    type_kind kind;
} consdesc_t;
typedef struct funsig funsig_t;
typedef struct scaldesc scaldesc_t;
typedef struct rtype
{
    robject_t base;
    type_kind kind;
    uint32_t hash;
    const typeops_t *ops;
    char *name;
    union
    {
        void *data;
        funsig_t *sig;
        const scaldesc_t *scal;
        struct rtype *elt;
        const consdesc_t *cons;
    };
} rtype_t;
typedef enum { NO, YES, MAYBE } compat;
compat r_type_compat(const rtype_t *self, const rtype_t *other, bool unbox);
rtype_t *r_common_type(rtype_t *self, rtype_t *other);
char *rtype_init_name(const rtype_t *type);
rtype_t *rtype_create(type_kind kind, void *data,
                      const typeops_t *ops, const char *name);
rtype_t *rtype_cons_create(const consdesc_t *cons, const char *name);
rtype_t *rtype_init(type_kind kind, const typeops_t *ops, const char *name);
rtype_t *rtype_cons_init(const consdesc_t *cons, const char *name);
void rtype_install(rtype_t *type, const char *name);
rtype_t *rtype_from_spec(ast_t *ast);
void rt_bootstrap();
extern hashmap_t *r_global_types;
extern hashset_t *r_typetab;
extern rtype_t *r_type_type;

static inline bool rtype_is_vector(const rtype_t *typ)
    { return typ->kind == RT_VECTOR; }
static inline bool rtype_is_array(const rtype_t *typ)
    { return typ->kind == RT_ARRAY; }
static inline bool rtype_is_scalar(const rtype_t *typ)
    { return typ->kind == RT_SCALAR; }
static inline bool rtype_is_callable(const rtype_t *typ)
    { return typ->kind == RT_CALLABLE; }
static inline bool rtype_is_constructor(const rtype_t *typ)
    { return typ->kind == RT_TYPECONS; }
static inline bool rtype_is_cell(const rtype_t *typ)
    { return typ->kind == RT_CELL; }
static inline bool rtype_is_container(const rtype_t *typ)
    { return typ->kind == RT_VECTOR || typ->kind == RT_ARRAY; }
static inline bool
rtype_is_instance(const rtype_t *self, const rtype_t *other)
    { return other->kind == RT_TYPECONS && other->cons
          && other->cons->kind == self->kind; }
static inline size_t rscal_size(const rtype_t *typ);
static inline size_t rtype_eltsz(const rtype_t *typ)
{
    assert(typ);
    return rtype_is_scalar(typ)
        ? rscal_size(typ)
        : sizeof(robject_t *);
}
static inline bool r_subtypep(const rtype_t *self, const rtype_t *other)
    { return r_type_compat(self, other, false) == YES; }
static inline const char *rtype_name(rtype_t *type)
{
    if(!type->name)
        type->name = rtype_init_name(type);
    return type->name;
}
