typedef struct cnode cnode_t;
typedef struct cblock cblock_t;
typedef struct cfunction cfunction_t;
typedef struct cvar cvar_t;
typedef ARRAY(cfunction_t *) cfunction_array_t;
typedef ARRAY(cblock_t *) cblock_array_t;
typedef ARRAY(cnode_t *) cnode_array_t;
typedef ARRAY(cvar_t *) cvar_array_t;
typedef ARRAY(rsymbol_t *) rsymbol_array_t;
typedef struct cfunction
{
    cfunction_t *parent;
    cnode_t *node;
    rtype_t *cl_type;
    list_t cfunc_list, cfunc_head;
    list_t cvar_head, cblock_head;
    cblock_t *entry;
    unsigned id, nfuncs, nblocks, nnodes;
    unsigned ninlined;
    cvar_array_t closure;
} cfunction_t;
typedef enum
{
    LEXICAL, GLOBAL_INT, GLOBAL_EXT,
} cvartype;
typedef struct cvar
{
    list_t cvar_list;
    rsymbol_t *name;
    rtype_t *decl;
    bool is_const;
    unsigned id, mark;
    cvartype type;
    union
    {
        struct
        {
            cfunction_t *binder;
            bool is_arg;
            bool is_optional;
            bool is_closed;
        } local;
        struct
        {
            cnode_t *set;
        } intl;
        struct
        {
            rglobal_t *global;
        } extl;
    };
} cvar_t;
typedef struct cblock
{
    list_t cblock_list;
    list_t cnode_head;
    cblock_array_t pred;
    cblock_array_t succ;
    unsigned id, start, end, mark;
} cblock_t;
typedef enum
{
    CN_LAMBDA, CN_CALL, CN_CALL_FAST, CN_BUILTIN,
    CN_SET, CN_BIND, CN_REF, CN_IF, CN_RETURN,
    CN_CONST, CN_COPY, CN_PHI
} cnodetype;
typedef struct cnode
{
    list_t cnode_list;
    cblock_t *block;
    rtype_t *decl;
    cnode_array_t users;
    unsigned id, mark;
    rsymbol_t *file;
    cnodetype type;
    union
    {
        struct
        {
            cfunction_t *function;
            cnode_array_t closure;
        } lambda;
        struct
        {
            cnode_t *target;
            cnode_array_t args;
            union
            {
                rsymbol_array_t names;
                argbits_t argbits;
            };
        } call;
        struct
        {
            const cbuiltin_t *bi;
            rtype_t *optype;
            cnode_array_t args;
        } builtin;
        struct
        {
            cvar_t *var;
            cnode_t *value;
        } set;
        struct
        {
            cvar_t *var;
        } ref;
        struct
        {
            cnode_t *cond;
        } ifelse;
        struct
        {
            cnode_t *value;
        } ret;
        struct
        {
            cnode_t *value;
        } copy;
        robject_t *constant;
        struct
        {
            cnode_array_t args;
        } phi;
    };
} cnode_t;
cfunction_t *cfunc_create(cfunction_t *parent);
void cfunc_free(cfunction_t *fn);
rtype_t *cfunc_type(cfunction_t *fn, rtype_t *ret_type);
cresult cfunc_map_children(cfunction_t *fn, cresult (*func)(cfunction_t *));
void cfunc_mapc_children(cfunction_t *fn, void (*func)(cfunction_t *));

cblock_t *cblock_create();
void cblock_free(cblock_t *block);
int index_of_block(cblock_array_t *arr, cblock_t *block);
void link_blocks(cblock_t *to, cblock_t *from);
void replace_link(cblock_array_t *arr, cblock_t *from, cblock_t *to);
void remove_link(cblock_array_t *arr, cblock_t *from);
cblock_t *split_block(cfunction_t *fn, cnode_t *site);

cnode_t *cnode_create(cblock_t *block, cnodetype type);
cnode_t *cnode_prepend(cblock_t *block, cnodetype type);
cnode_t *cnode_append(cblock_t *block, cnodetype type);
cnode_t *cnode_insert_before(cnode_t *other, cnodetype type);
cnode_t *cnode_insert_after(cnode_t *other, cnodetype type);
int index_of_node(cnode_array_t *arr, cnode_t *node);
void cnode_fini(cnode_t *node);
void cnode_free(cnode_t *node);
void cnode_reset(cnode_t *node);
void cnode_remove(cnode_t *node);

void cnode_unuse_all(cnode_t *node);
void cnode_replace_in_users(cnode_t *from, cnode_t *to);
void cnode_add_user(cnode_t *user, cnode_t *node);
void cnode_remove_user(cnode_t *user, cnode_t *node);
void cnode_map_used(cnode_t *node, void (*func)(cnode_t **, void *), void *data);
void cfunc_node_users(cfunction_t *fn);

bool cnode_is_pure(cnode_t *node, bool may_alias);

cnode_t *guard_decl(cnode_t *guard, cnode_t *node, rtype_t *decl, cnode_t *val);
cresult enforce_decl(cnode_t *node, rtype_t *decl, cnode_t **pval, bool boxing);
cresult enforce_val(cnode_t *node, rtype_t *decl);
robject_t *nil_init(rtype_t *decl);

cvar_t *cvar_create(rsymbol_t *name, cvartype type, rtype_t *decl);
void cvar_free(cvar_t *var);
int index_of_var(cvar_array_t *arr, cvar_t *block);

cresult cfunc_crit_edges(cfunction_t *fn); // FIXME can remove from here when we stop asserting with it
void cfunc_rdfo(cfunction_t *fn);
void cfunc_cleanup(cfunction_t *fn);
void cfunc_init_closure(cfunction_t *fn);
void cfunc_ssa_convert(cfunction_t *fn);

#include "ir_dom.h"

// XXX really somewhere in opt, because builtin_ops_t is private
bool builtin_is_void(const cbuiltin_t *bi, cnode_t *node);
bool builtin_is_pure(const cbuiltin_t *bi, cnode_t *node, bool may_alias);
void ir_dump(cfunction_t *fn); // DEBUG
static inline bool cfunc_has_closure(cfunction_t *fn)
    { return alen(&fn->closure) > 0; }
static inline bool cnode_is_used(cnode_t *node)
    { return !array_isempty(&node->users); }
static inline bool cnode_yields_value(cnode_t *node)
{
    switch(node->type)
    {
    case CN_IF:
    case CN_RETURN:
    case CN_SET:
        return false;
    case CN_BUILTIN:
        if(node->builtin.bi)
            return !builtin_is_void(node->builtin.bi, node);
        /* fallthrough */
    default:
        return true;
    }
}
static inline bool call_has_names(cnode_t *node)
    { return alen(&node->call.names) > 0; }
static inline bool cnode_only_used_by(cnode_t *node, cnode_t *user)
{
    cnode_t *chk;

    if(!cnode_is_used(node))
        return false;
    array_foreach_entry(&node->users, chk)
        if(chk != user)
            return false;
    return true;
}
static inline rtype_t *decl_type(rtype_t *decl)
    { return decl ? decl : r_type_object; }
static inline const char *decl_name(rtype_t *decl)
    { return rtype_name(decl_type(decl)); }
static inline compat cnode_compat(cnode_t *node, rtype_t *decl)
    { return r_type_compat(decl_type(node->decl), decl_type(decl), true); }
static inline bool cvar_is_global(cvar_t *var)
    { return var->type != LEXICAL; }
static inline bool cvar_is_celled(cvar_t *var)
    { return !cvar_is_global(var) && !var->is_const && var->local.is_closed; }
