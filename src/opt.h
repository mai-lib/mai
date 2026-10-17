typedef enum { TOP, CONST_OBJ, CONST_LAMBDA, TYPE, BOTTOM } cellstate;
typedef struct
{
    cellstate state;
    rtype_t *type;
    void *value;
} cell_t;
static inline rtype_t *cell_type(cell_t *cell)
    { return (cell->state == TOP || cell->state == BOTTOM)
             ? NULL : cell->type; }
static inline bool cell_const_obj(cell_t *cell)
    { return cell->state == CONST_OBJ; }
static inline bool cell_const_lambda(cell_t *cell)
    { return cell->state == CONST_LAMBDA; }
static inline void *cell_const(cell_t *cell)
    { return cell->value; }
static inline void cell_init(cell_t *cell, cellstate state)
    { cell->state = state; }
static inline void cell_set_const_at(cell_t *cell, robject_t *obj, rtype_t *type)
{
    *cell = (cell_t) {
        .state = CONST_OBJ,
        .type = type,
        .value = obj
    };
}
static inline void cell_set_const(cell_t *cell, robject_t *obj)
    { cell_set_const_at(cell, obj, r_typeof(obj)); }
static inline void cell_set_lambda(cell_t *cell, cfunction_t *fn)
{
    *cell = (cell_t) {
        .state = CONST_LAMBDA,
        .type = fn->cl_type,
        .value = fn
    };
}
static inline void cell_set_type(cell_t *cell, rtype_t *type)
{
    *cell = (cell_t) {
        .state = TYPE,
        .type = type
    };
}
extern cell_t *cells;
extern bool *flags;
static inline cell_t *cell_for(cnode_t *node)
    { return &cells[node->id]; }
static inline bool flag_for(cblock_t *block)
    { return flags[block->id]; }
extern cnode_array_t inlines;
typedef enum { TRANSFER, TRANSFORM } opt_phase;
typedef struct builtin_ops
{
    cresult (*trans_fn)(opt_phase, cnode_t *, cell_t *,
                        const cbuiltin_t *, cnode_array_t *);
    cresult (*enforce_fn)(cnode_t *);
    void (*generate_fn)(cnode_t *);
    bool (*is_pure)(cnode_t *, bool);
    bool is_void;
} builtin_ops_t;
// for arith/opt.c
void call_become_builtin(cnode_t *node, const cbuiltin_t *bi,
                         rtype_t *optype, rtype_t *type);
void call_become_copy(cnode_t *node, rtype_t *type);
// for opt_post.c
robject_t *constant_convert(robject_t *sval, rtype_t *dtype, bool *pvalid);
void node_become_constant(cnode_t *node, robject_t *val);
// for opt_sccp.c (entry points)
void opt_transfer(cfunction_t *fn, cnode_t *node, cell_t *cell);
cresult opt_transform(cfunction_t *fn, cnode_t *node, cell_t *cell);
// opt_post.c
cresult call_normalise(cnode_t *node, funsig_t *sig);
// opt_inline.c
cresult inline_lambdas(cfunction_t *outer, cnode_array_t *nodes);
static inline rtype_t *arg_type(cnode_array_t *args, int n)
{
    cnode_t *arg = aref(args, n);
    return arg ? cell_type(cell_for(arg)) : r_type_nil;
}
static inline bool check_arg_names(cnode_t *node)
{
    if(node->type == CN_CALL && alen(&node->call.names) > 0)
        return false;
    return true;
}
static inline bool check_args(cnode_t *node, cnode_array_t *args)
{
    cnode_t *arg;
    array_foreach_entry(args, arg)
        if(!arg)
            return false;
    return check_arg_names(node);
}
static inline bool check_nargs(int arity, cnode_t *node, cnode_array_t *args)
{
    if(alen(args) != arity)
        return false;
    return check_args(node, args);
}
