#include "global.h"
#include "ast.h"
hashset_t *r_typetab;
bool sig_equal(funsig_t *a, funsig_t *b);
static bool rtype_equal(const void *xp, const void *yp)
{
    const rtype_t *x = xp, *y = yp;

    if(x->kind != y->kind)
        return false;
    switch(x->kind)
    {
    case RT_CALLABLE:
        return sig_equal(x->sig, y->sig);
    case RT_VECTOR:
    case RT_ARRAY:
    case RT_CELL:
        return r_equal(x->elt, y->elt);
    case RT_SCALAR:
        return x->scal == y->scal;
    case RT_TYPECONS:
        return x->cons == y->cons;
    case RT_OBJECT:
        return x->name == y->name;
    }
    return false;
}
static uint32_t rtype_hash(const void *ptr)
{
    return ((rtype_t *)ptr)->hash;
}
void sig_gc(funsig_t *sig);
static void rtype_gc(void *ptr)
{
    rtype_t *type = ptr;
    assert(r_typeof(type) == r_type_type);
    switch(type->kind)
    {
    case RT_CALLABLE:
        sig_gc(type->sig);
        break;
    case RT_VECTOR:
    case RT_ARRAY:
    case RT_CELL:
        gc_mark(type->elt);
        break;
    default:
        break;
    }
}
void sig_free(funsig_t *sig);
static void free_type_data(rtype_t *type)
{
    switch(type->kind)
    {
    case RT_CALLABLE:
        sig_free(type->sig);
        /* fallthrough */
    case RT_VECTOR:
    case RT_ARRAY:
    case RT_CELL:
        if(type->name)
            xfree(type->name);
        break;
    default:
        break;
    }
}
static void rtype_free(void *ptr)
{
    hashset_remove(r_typetab, ptr);
    free_type_data(ptr);
}
static inline void elt_print(FILE *fp, const char *label, const rtype_t *type)
{
    fprintf(fp, "%s(", label);
    r_print(fp, type->elt);
    fprintf(fp, ")");
}
void sig_print(FILE *fp, funsig_t *sig);
char *rtype_init_name(const rtype_t *type)
{
    char *buf = NULL;
    size_t sz;
    FILE *fp = open_memstream(&buf, &sz);
    assert(fp);
    switch(type->kind)
    {
    case RT_CALLABLE:
        sig_print(fp, type->sig);
        break;
    case RT_VECTOR:
        elt_print(fp, "vector", type);
        break;
    case RT_ARRAY:
        elt_print(fp, "array", type);
        break;
    case RT_CELL:
        elt_print(fp, "cell", type);
        break;
    default:
        break;
    }
    fclose(fp);
    return buf;
}
static void rtype_print(FILE *fp, const void *ptr)
{
    fprintf(fp, "%s", rtype_name((rtype_t *)ptr));
}
static const typeops_t type_ops = {
    .free = rtype_free,
    .gc = rtype_gc,
    .hash = rtype_hash,
    .equal = rtype_equal,
    .print = rtype_print
};
bool sig_types_equal(funsig_t *self, funsig_t *other);
compat r_type_compat(const rtype_t *self, const rtype_t *other, bool unbox)
{
    assert(self);
    assert(other);
    if(self == other)
        return YES;
    if(unbox && rtype_is_scalar(self) && rtype_is_scalar(other))
        return MAYBE;
    if(self == r_type_object)
        return MAYBE;
    if(other == r_type_object)
        return (unbox & rtype_is_scalar(self)) ? MAYBE : YES;
    if(self == r_type_nil)
        return rtype_is_scalar(other) ? NO : YES;
    if(other == r_type_nil)
        return rtype_is_scalar(self) ? NO : MAYBE;
    if(rtype_is_instance(self, other))
        return YES;
    if(rtype_is_instance(other, self))
        return MAYBE;
    if(self->kind == RT_CALLABLE && other->kind == RT_CALLABLE)
        return sig_types_equal(self->sig, other->sig) ? YES : NO;
    return NO;
}
rtype_t *r_common_type(rtype_t *self, rtype_t *other)
{
    assert(self);
    assert(other);
    if(r_subtypep(self, other))
        return other;
    if(r_subtypep(other, self))
        return self;
    if(self->kind != other->kind)
        return r_type_object;
    switch(self->kind)
    {
    case RT_SCALAR: return rscal_promote(self, other);
    case RT_CALLABLE: return r_type_callable;
    case RT_VECTOR: return r_type_vector;
    case RT_ARRAY: return r_type_array;
    default: break;
    }
    return r_type_object;
}
static uint32_t compute_hash(rtype_t *type)
{
    if(type->kind == RT_CALLABLE)
        return type->sig->hash;
    return hash_code(type, sizeof(rtype_t));
}
static void type_init(rtype_t *type, type_kind kind, void *data,
                      const typeops_t *ops, const char *name)
{
    *type = (rtype_t) {
        .base.type = r_type_type,
        .name = (char *)name,
        .kind = kind,
        .data = data,
        .ops = ops,
        .hash = 0,
    };
    type->hash = compute_hash(type);
}
rtype_t *rtype_create(type_kind kind, void *data,
                      const typeops_t *ops, const char *name)
{
    rtype_t *type, *otyp, tmp;

    type_init(&tmp, kind, data, ops, name);
    otyp = hashset_get(r_typetab, &tmp);
    if(!otyp)
    {
        type = gc_alloc(r_type_type, sizeof(rtype_t));
        *type = tmp;
        hashset_insert(r_typetab, type);
        return type;
    }
    free_type_data(&tmp);
    return otyp;
}
hashmap_t *r_global_types;
void rtype_install(rtype_t *type, const char *name)
{
    hashmap_set(r_global_types, r_intern(name), type);
}
rtype_t *rtype_init(type_kind kind, const typeops_t *ops, const char *name)
{
    rtype_t *type = rtype_create(kind, NULL, ops, name);
    rtype_install(type, name);
    return type;
}
rtype_t *rtype_cons_init(const consdesc_t *cons, const char *name)
{
    rtype_t *type = rtype_cons_create(cons, name);
    rtype_install(type, name);
    return type;
}
rtype_t *rtype_cons_create(const consdesc_t *cons, const char *name)
{
    return rtype_create(RT_TYPECONS, (void *)cons, NULL, name);
}
static rtype_t *rtype_from_name(rsymbol_t *name)
{
    rtype_t *type = hashmap_get(r_global_types, name, NULL);

    if(!type)
        c_warning("unknown type `%s`.", r_symstr(name));
    return type;
}
static rtype_t *rtype_from_node(ast_array_t *arr)
{
    int nargs = alen(arr)-1;
    ast_t *car = aptr(arr, 0);
    ast_t *args = (nargs>0) ? aptr(arr,1) : NULL;
    rtype_t *type = rtype_from_spec(car);

    if(!type)
        return NULL;
    if(!rtype_is_constructor(type))
    {
        c_warning("type `%s` not a type constructor.", ast_str(car));
        return NULL;
    }
    assert(type->cons && type->cons->from_spec);
    return type->cons->from_spec(type->cons, nargs, args);
}
rtype_t *rtype_from_spec(ast_t *ast)
{
    if(!ast)
        return NULL;
    switch(ast->type)
    {
    case AST_INVALID:
        return NULL;
    case AST_NAME:
    case AST_SYMBOL:
        return rtype_from_name(ast->symbol);
    case AST_TOKEN:
        return rtype_from_name(r_intern(ast_str(ast)));
    case AST_NODE:
        assert(alen(ast->children) >= 1);
        return rtype_from_node(ast->children);
    default:
        break;
    }
    c_warning("malformed type declaration.");
    return NULL;
}
rtype_t *r_type_type;
void rt_bootstrap()
{
    r_global_types = hashmap_create(rsym_hash, ptr_eq);
    r_typetab = hashset_create(rtype_hash, rtype_equal);
    r_type_type = rtype_create(RT_OBJECT, NULL, &type_ops, "type");
    r_type_type->base.type = r_type_type;
}
