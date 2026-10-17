typedef uint8_t op_code_t;
typedef uint16_t op_offset_t;
typedef int16_t op_stack_t; /* "32k of stack frame is enough for anyone" */
typedef unsigned argbits_t;
static_assert(sizeof(argbits_t) == sizeof(rint_t),
              "argbits not the same size as int.");
static inline bool argbits_missing(argbits_t a, unsigned b)
    { return (a & (1<<b)) == 0; }
typedef struct
{
    rtype_t *type;
    op_offset_t offset;
    rsymbol_t *name;
    bool is_optional;
} argdesc_t;
typedef struct funsig
{
    argdesc_t *args;
    unsigned nargs;
    rtype_t *ret_type;
    bool has_rest;
    unsigned reqbits;
    uint32_t hash;
    op_offset_t argsz;
} funsig_t;
typedef struct
{
    robject_t base;
    op_code_t *code;
    rtype_t *cl_type;
    op_offset_t loc_scalsz, loc_sz;
    op_offset_t env_scalsz, env_sz;
    robject_t **consts;
    unsigned nconsts;
} rfunction_t;
typedef enum { CALL_CLOSURE, CALL_BUILTIN } call_dispatch;
typedef struct rcallable
{
    robject_t base;
    call_dispatch disp;
} rcallable_t;
static inline bool rcall_is_builtin(const rcallable_t *cl)
    { return cl->disp != CALL_CLOSURE; }
typedef struct rclosure
{
    rcallable_t base;
    rfunction_t *fn;
    void *env;
} rclosure_t;
typedef struct builtin_ops builtin_ops_t;
typedef struct cbuiltin
{
    const builtin_ops_t *ops;
    void *data;
    const char *name;
} cbuiltin_t;
typedef struct rbuiltin rbuiltin_t;
typedef struct vm_ctx vm_ctx_t;
typedef void (*builtin_fn)(vm_ctx_t *, rbuiltin_t *, uint8_t *, void *);
typedef struct rbuiltin
{
    rcallable_t base;
    builtin_fn fn;
    const cbuiltin_t *cbi;
} rbuiltin_t;
rtype_t *rcall_type_create(rtype_t *ret_type, unsigned nargs, argdesc_t *args);
rfunction_t *rfunc_create(rtype_t *cl_type, op_code_t *code, size_t loc_scalsz,
                          size_t loc_sz, size_t env_scalsz, size_t env_sz,
                          robject_array_t *consts);
rclosure_t *rcall_closure_create(rtype_t *type, rfunction_t *fn);
rcallable_t *rcall_builtin_create(rtype_t *type, builtin_fn fn,
                                  const cbuiltin_t *builtin);
void rt_install_call_types();
extern rtype_t *r_type_function, *r_type_callable;
static inline argdesc_t argdesc_init(rsymbol_t *name, rtype_t *type, bool is_opt)
{
    return (argdesc_t) {
        .name = name,
        .type = type,
        .is_optional = is_opt
    };
}
static inline funsig_t *rcall_sig(const rcallable_t *cl)
    { return cl->base.type->sig; }
