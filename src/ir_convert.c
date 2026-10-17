#include "global.h"
#include "ir.h"
#include "ast.h"
#include "grammar.h"
#include <errno.h>
#define ir_error(ir, fmt, args...) do {                         \
        if(ir->pfailed) *ir->pfailed = true;                    \
        c_message_va(C_ERROR, ir->file, fmt, ##args);           \
    } while (0)
#define ir_warning(ir, fmt, args...)                            \
    c_message_va(C_WARNING, ir->file, fmt, ##args)
typedef struct funenv
{
    cfunction_t *function;
    list_t cblock_head;
} funenv_t;
static funenv_t *fun_open(funenv_t *fun, funenv_t *parent)
{
    fun->function = cfunc_create(parent ? parent->function : NULL);
    list_init(&fun->cblock_head);
    return fun;
}
static cblock_t *fun_add_block(funenv_t *fun)
{
    cblock_t *block = cblock_create();
    list_add(&fun->cblock_head, &block->cblock_list);
    return block;
}
static cfunction_t *fun_close(funenv_t *fun, cblock_t *entry,
                              rtype_t *ret_type)
{
    cfunction_t *fn = fun->function;

    fn->entry = entry;
    fn->cl_type = cfunc_type(fn, ret_type);
    cfunc_rdfo(fn);
    return fn;
}
typedef struct lexenv lexenv_t;
typedef struct lexenv
{
    hashmap_t *vars;
    lexenv_t *parent;
    lexenv_t *global;
} lexenv_t;
static cvar_t *env_add(lexenv_t *env, cvar_t *var)
{
    if(!env->vars)
        env->vars = hashmap_create(rsym_hash, ptr_eq);
    hashmap_set(env->vars, var->name, var);
    return var;
}
static cvar_t *env_add_global(lexenv_t *env, rsymbol_t *name)
{
    cvar_t *var = cvar_create(name, GLOBAL_EXT, NULL);

    var->extl.global = NULL;
    env_add(env->global, var);
    return var;
}
static cvar_t *env_find(lexenv_t *env, rsymbol_t *name)
{
    bool found = false;
    cvar_t *var;

    if(env->vars)
    {
        var = hashmap_get(env->vars, name, &found);
        if(found)
            return var;
    }
    if(env->parent)
        return env_find(env->parent, name);
    return NULL;
}
static cvar_t *env_find_add(lexenv_t *env, rsymbol_t *name)
{
    cvar_t *var = env_find(env, name);

    if(var)
        return var;
    return env_add_global(env, name);
}
static lexenv_t *env_open(lexenv_t *env, lexenv_t *parent)
{
    *env = (lexenv_t) {
        .vars = NULL,
        .parent = parent,
        .global = parent ? parent->global : env,
    };
    return env;
}
static void env_close(lexenv_t *env)
{
    if(env->vars)
        hashmap_free(env->vars);
}
typedef struct
{
    cblock_t *brk, *cont;
} loopenv_t;
typedef struct ir_builder
{
    rsymbol_t *file;
    cblock_t *head, *tail;
    bool *pfailed;
    cnode_t *value;
    funenv_t *fun;
    lexenv_t *env;
    loopenv_t *loop;
} ir_builder_t;
static bool convert_expr(ir_builder_t *ir, ast_t *ast);
static bool convert_stmt(ir_builder_t *ir, ast_t *ast);
static inline bool is_top_level(ir_builder_t *ir)
    { return !ir->fun->function->parent; }
static ir_builder_t *ir_open(ir_builder_t *ir, ir_builder_t *other)
{
    if(other)
        *ir = *other;
    assert(ir->fun);
    ir->head = ir->tail = fun_add_block(ir->fun);
    ir->value = NULL;
    return ir;
}
static cnode_t *ir_build(ir_builder_t *ir, cnodetype type)
{
    cnode_t *node = cnode_append(ir->tail, type);
    node->file = ir->file;
    if(type != CN_BIND && cnode_yields_value(node))
        ir->value = node;
    else
        ir->value = NULL;
    return node;
}
static ir_builder_t *ir_extend(ir_builder_t *ir, ir_builder_t *other)
{
    ir->tail = other->tail;
    ir->value = other->value;
    return ir;
}
static cnode_t *build_constant(ir_builder_t *ir, robject_t *ptr)
{
    cnode_t *node = ir_build(ir, CN_CONST);
    robject_t *obj = c_intern(ptr);

    node->constant = obj;
    node->decl = r_typeof(obj);
    return node;
}
static cnode_t *build_ref(ir_builder_t *ir, rsymbol_t *name)
{
    cnode_t *node = ir_build(ir, CN_REF);

    node->ref.var = env_find_add(ir->env, name);
    return node;
}
static cnode_t *build_call(ir_builder_t *ir, cnode_t *target,
                           cnode_array_t *args, rsymbol_array_t *names)
{
    cnode_t *node = ir_build(ir, CN_CALL);

    node->call.target = target;
    node->call.args = *args;
    node->call.names = *names;
    return node;
}
static cnode_t *build_phi(ir_builder_t *ir, cnode_t *left, cnode_t *right)
{
    cnode_t *node = ir_build(ir, CN_PHI);
    cnode_array_t *args = &node->phi.args;

    assert(left && right);
    array_init(args, 2);
    array_push(args, left);
    array_push(args, right);
    return node;
}
static cnode_t *build_if(ir_builder_t *ir, cnode_t *cond)
{
    cnode_t *node = ir_build(ir, CN_IF);

    node->ifelse.cond = cond;
    return node;
}
static cnode_t *build_set(ir_builder_t *ir, cvar_t *var, cnode_t *val)
{
    cnode_t *node = ir_build(ir, CN_SET);

    node->set.var = var;
    node->set.value = ir->value = val;
    return node;
}
static cnode_t *build_return(ir_builder_t *ir)
{
    cnode_t *prev = ir->value ? ir->value : build_constant(ir, NULL),
            *node = ir_build(ir, CN_RETURN);

    node->ret.value = prev;
    return node;
}
static cnode_t *build_bind(ir_builder_t *ir, cvar_t *var,
                           rtype_t *decl, cnode_t *val)
{
    cnode_t *node = ir_build(ir, CN_BIND);

    node->set.value = val;
    node->set.var = var;
    node->decl = decl;
    return node;
}
static cnode_t *build_lambda(ir_builder_t *ir, cfunction_t *fn)
{
    cnode_t *node = ir_build(ir, CN_LAMBDA);

    array_init(&node->lambda.closure, 0);
    node->lambda.function = fn;
    fn->node = node;
    return node;
}
static cnode_t *build_builtin(ir_builder_t *ir, const cbuiltin_t *bi,
                              cnode_array_t *args, rtype_t *optype, rtype_t *decl)
{
    cnode_t *node = ir_build(ir, CN_BUILTIN);

    node->decl = decl;
    node->builtin.bi = bi;
    node->builtin.args = *args;
    node->builtin.optype = optype;
    return node;
}
static robject_t *constant_from_literal(ast_t *ast)
{
    switch(ast->type)
    {
    case AST_INT: return r_box(r_type_int, &ast->integer);
    case AST_DOUBLE: return r_box(r_type_double, &ast->dfloat);
    case AST_STRING: return (robject_t *)rstr_create(ast->string);
    case AST_TOKEN: return (robject_t *)r_intern(ast_str(ast));
    case AST_QUOTED:
    case AST_SYMBOL:
    case AST_NAME:
        return (robject_t *)ast->symbol;
    case AST_INVALID: return NULL;
    default: break;
    }
    return NULL; /* NOTREACHED */
}
typedef struct
{
    cvar_t *var;
    union
    {
        ast_t *expr;
        cnode_t *node;
    };
} varbind_t;
typedef ARRAY(varbind_t) varbind_array_t;
typedef cvar_t *(*varbind_fn)(rsymbol_t *, rtype_t *, void *);
static void bind_from_decl(ast_t *ast, varbind_t *bind, varbind_fn varfn,
                           void *data)
{
    rsymbol_t *name;
    ast_t *tspec = NULL, *expr = NULL;

    if(ast->type == AST_NAME)
    {
        name = ast->symbol;
    }
    else
    {
        assert(ast->type == AST_NODE);
        ast_array_t *arr = ast->children;
        assert(alen(arr) == 2 || alen(arr) == 3);
        ast_t *fst = aptr(arr, 0);
        ast_t *snd = aptr(arr, 1);

        if(snd->type == AST_NAME)
        {
            tspec = fst;
            name = snd->symbol;
            if(alen(arr) == 3)
                expr = aptr(arr, 2);
        }
        else
        {
            name = fst->symbol;
            expr = snd;
        }
    }
    *bind = (varbind_t) {
        .var = varfn(name, rtype_from_spec(tspec), data),
        .expr = expr
    };
}
static void binds_from_decls(ast_array_t *decls, varbind_array_t *binds,
                             varbind_fn varfn, void *data)
{
    int i;

    array_init(binds, alen(decls));
    array_resize(binds, alen(decls));
    array_foreach(decls, i)
        bind_from_decl(aptr(decls, i), aptr(binds, i), varfn, data);
}
static void binds_finalize(varbind_array_t *binds, bool open)
{
    varbind_t *bind;

    array_foreach_ptr(binds, bind)
    {
        if(!cvar_is_global(bind->var))
        {
            cfunction_t *fn = bind->var->local.binder;
            if(open)
                list_add_before(&fn->cvar_head, &bind->var->cvar_list);
            else
                cvar_free(bind->var);
        }
    }
    array_fini(binds);
}
static bool convert_initexpr(ir_builder_t *ir, varbind_t *bind)
{
    if(bind->expr)
        return convert_expr(ir, bind->expr);
    return build_constant(ir, nil_init(bind->var->decl));
}
static bool convert_let_binds(ir_builder_t *ir, varbind_array_t *binds)
{
    varbind_t *bind;

    array_foreach_ptr(binds, bind)
    {
        if(!convert_initexpr(ir, bind))
            return false;
        bind->node = ir->value;
    }
    array_foreach_ptr(binds, bind)
    {
        build_bind(ir, bind->var, bind->var->decl, bind->node);
        env_add(ir->env, bind->var);
    }
    return true;
}
static bool convert_var_binds(ir_builder_t *ir, varbind_array_t *binds)
{
    varbind_t *bind;

    array_foreach_ptr(binds, bind)
    {
        if(!convert_initexpr(ir, bind))
            return false;
        build_bind(ir, bind->var, bind->var->decl, ir->value);
        env_add(ir->env, bind->var);
    }
    return true;
}
static cvar_t *local_create(rsymbol_t *name, rtype_t *decl, void *data)
{
    ir_builder_t *ir = data;
    cvar_t *var = cvar_create(name, LEXICAL, decl);

    var->is_const = true;
    var->local.binder = ir->fun->function;
    return var;
}
static bool convert_let(ir_builder_t *ir, ast_t *decls, bool is_let)
{
    cnode_t *val = ir->value;
    varbind_array_t binds;
    bool open;

    binds_from_decls(decls->children, &binds, local_create, ir);
    open = is_let ?
        convert_let_binds(ir, &binds) :
        convert_var_binds(ir, &binds);
    binds_finalize(&binds, open);
    ir->value = val;
    return open;
}
static cvar_t *global_create(rsymbol_t *name, rtype_t *decl, void *data)
{
    ir_builder_t *ir = data;
    cvar_t *var = env_find(ir->env->global, name);

    if(!var)
        var = env_add_global(ir->env, name);
    else if(var->type == GLOBAL_INT)
        ir_error(ir, "duplicate global declaration `%s`",
                 r_symstr(var->name));
    var->type = GLOBAL_INT;
    var->decl = decl;
    return var;
}
static bool convert_global_binds(ir_builder_t *ir, varbind_array_t *binds,
                                 bool is_const)
{
    varbind_t *bind;

    array_foreach_ptr(binds, bind)
    {
        if(is_const && !bind->expr)
        {
            ir_error(ir, "`const` must have an initial value.");
            return false;
        }
        if(!convert_initexpr(ir, bind))
            return false;
        bind->var->is_const = is_const;
        bind->var->intl.set = build_set(ir, bind->var, ir->value);
    }
    return true;
}
static bool convert_global(ir_builder_t *ir, ast_t *decls, bool is_const)
{
    cnode_t *val = ir->value;
    varbind_array_t binds;
    bool open;

    if(!is_top_level(ir))
    {
        ir_error(ir, "`%s` only allowed at top level.",
                 is_const ? "const" : "global");
        return false;
    }

    binds_from_decls(decls->children, &binds, global_create, ir);
    open = convert_global_binds(ir, &binds, is_const);
    binds_finalize(&binds, open);
    ir->value = val;
    return open;
}
extern const cbuiltin_t missing;
static bool convert_default(ir_builder_t *ir, varbind_t *bind, argbits_t i,
                            cnode_t *bits)
{
    ir_builder_t tb, jn;
    robject_t *idx = c_intern(r_box(r_type_int, &i));
    cnode_array_t args;

    array_init(&args, 1);
    array_push(&args, bits);
    build_builtin(ir, &missing, &args, (rtype_t *)idx, r_type_boolean);
    build_if(ir, ir->value);

    ir_open(&tb, ir);
    link_blocks(tb.head, ir->tail);
    if(!convert_expr(&tb, bind->expr))
        return false;
    build_set(&tb, bind->var, tb.value);

    ir_open(&jn, ir);
    link_blocks(jn.head, tb.tail);
    link_blocks(jn.head, ir->tail);
    ir_extend(ir, &jn);
    return true;
}
static bool convert_arg_binds(ir_builder_t *ir, varbind_array_t *binds,
                              cnode_t *bits)
{
    int i;

    array_foreach(binds, i)
    {
        varbind_t *bind = aptr(binds, i);

        if(bind->var->name == r_sym_rest)
        {
            if(i != alen(binds)-1 || bind->var->decl || bind->expr)
            {
                ir_error(ir, "invalid '...' declaration.");
                return false;
            }
            bind->var->decl = r_type_vec_object;
            bind->var->local.is_optional = true;
        }
        else if(bind->expr)
        {
            bind->var->local.is_optional = true;
        }
        build_bind(ir, bind->var, bind->var->decl, NULL);
    }
    array_foreach(binds, i)
    {
        varbind_t *bind = aptr(binds, i);

        if(bind->expr && !convert_default(ir, bind, i, bits))
            return false;
        env_add(ir->env, bind->var);
    }
    return true;
}
static cvar_t *arg_create(rsymbol_t *name, rtype_t *decl, void *data)
{
    ir_builder_t *ir = data;
    cvar_t *var = cvar_create(name, LEXICAL, decl);

    var->is_const = true;
    var->local.binder = ir->fun->function;
    var->local.is_arg = true;
    var->local.is_optional = false;
    return var;
}
static bool convert_formals(ir_builder_t *ir, ast_array_t *decls)
{
    varbind_array_t binds;
    bool open = true;
    cnode_t *bits = build_bind(ir, NULL, r_type_int, NULL);

    binds_from_decls(decls, &binds, arg_create, ir);
    open = convert_arg_binds(ir, &binds, bits);
    binds_finalize(&binds, open);
    return open;
}
static bool convert_lambda(ir_builder_t *ir, rtype_t *ret_type,
                           ast_array_t *args, ast_t *body)
{
    funenv_t fun;
    lexenv_t env;
    ir_builder_t inner = {
        .fun = fun_open(&fun, ir->fun),
        .env = env_open(&env, ir->env),
        .pfailed = ir->pfailed,
        .file = ir->file
    };

    ir_open(&inner, NULL);
    if(alen(args) > sizeof(argbits_t)*8)
        ir_error(ir, "too many arguments.");
    else if(convert_formals(&inner, args) && convert_stmt(&inner, body))
        build_return(&inner);
    env_close(&env);
    build_lambda(ir, fun_close(&fun, inner.head, ret_type));
    assert(list_isempty(&fun.cblock_head) || *ir->pfailed);
    return true;
}
static bool convert_prog(ir_builder_t *ir, ast_array_t *arr)
{
    lexenv_t env;
    bool open = true;
    ir_builder_t inner = *ir;

    inner.env = env_open(&env, ir->env);
    for(int i = 1; i < alen(arr) && open; i++)
        open = convert_stmt(&inner, aptr(arr, i));
    env_close(&env);
    ir_extend(ir, &inner);
    return open;
}
static bool
convert_actuals(ir_builder_t *ir, cnode_array_t *args, rsymbol_array_t *names,
                ast_array_t *arr, bool *pnamed)
{
    rsymbol_t *name = NULL;

    array_init(args, alen(arr)-1);
    array_init(names, alen(arr)-1);
    for(int i = 1; i < alen(arr); i++)
    {
        ast_t *ast = aptr(arr, i);

        switch(ast->type)
        {
        case AST_NAME:
            *pnamed = true;
            name = ast->symbol;
            break;
        case AST_INVALID:
            array_push(args, NULL);
            array_push(names, NULL);
            break;
        default:
            if(!convert_expr(ir, ast))
                goto fail;
            if(ast_is_rest(ast))
            {
                *pnamed = true;
                name = r_sym_rest;
            }
            array_push(args, ir->value);
            array_push(names, name);
            name = NULL;
            break;
        }
    }

    if(alen(args) <= sizeof(argbits_t)*8)
        return true;
    ir_error(ir, "too many arguments.");
fail:
    array_fini(args);
    array_fini(names);
    return false;
}
static bool convert_call(ir_builder_t *ir, ast_t *fst, ast_array_t *arr)
{
    cnode_array_t args;
    rsymbol_array_t names;
    cnode_t *target;
    bool has_names = false;

    if(!convert_expr(ir, fst))
        return false;
    target = ir->value;

    if(!convert_actuals(ir, &args, &names, arr, &has_names))
        return false;

    if(!has_names)
        array_clear(&names);
    array_shrink(&args);
    array_shrink(&names);
    build_call(ir, target, &args, &names);
    return true;
}
static inline bool convert_branch(ir_builder_t *br, ir_builder_t *ir,
                                  ast_t *ast)
{
    bool open = true;

    ir_open(br, ir);
    if(ast)
        open = convert_stmt(br, ast);
    if(!br->value)
        build_constant(br, NULL);
    link_blocks(br->head, ir->tail);
    return open;
}
static inline void extend_join(ir_builder_t *ir, ir_builder_t *tb,
                               ir_builder_t *fb)
{
    ir_builder_t jn;

    ir_open(&jn, ir);
    build_phi(&jn, tb->value, fb->value);
    link_blocks(jn.head, tb->tail);
    link_blocks(jn.head, fb->tail);
    ir_extend(ir, &jn);
}
static bool convert_ifelse(ir_builder_t *ir, ast_t *cond,
                           ast_t *texpr, ast_t *fexpr)
{
    ir_builder_t tb, fb;
    bool topen, fopen;

    if(!convert_expr(ir, cond))
        return false;
    build_if(ir, ir->value);

    topen = convert_branch(&tb, ir, texpr);
    fopen = convert_branch(&fb, ir, fexpr);

    if(topen && fopen)
        extend_join(ir, &tb, &fb);
    else if(topen)
        ir_extend(ir, &tb);
    else if(fopen)
        ir_extend(ir, &fb);
    return topen || fopen;
}
static bool convert_or(ir_builder_t *ir, ast_t *lexpr, ast_t *rexpr)
{
    ast_t stub = {
        .type = AST_SYMBOL,
        .symbol = r_intern("true")
    };
    return convert_ifelse(ir, lexpr, &stub, rexpr);
}
static bool convert_and(ir_builder_t *ir, ast_t *lexpr, ast_t *rexpr)
{
    ast_t stub = {
        .type = AST_SYMBOL,
        .symbol = r_intern("false")
    };
    return convert_ifelse(ir, lexpr, rexpr, &stub);
}
static bool set_expandp(ast_t *ast)
{
    if(ast->type == AST_NODE && alen(ast->children) >= 2)
    {
        ast_t *fst = aptr(ast->children, 0);
        return fst->type == AST_SYMBOL || fst->type == AST_TOKEN;
    }
    return false;
}
static bool convert_set_expand(ir_builder_t *ir, ast_array_t *arr, ast_t *rhs)
{
    ast_t *fst = aptr(arr, 0), new;
    char *name;

    asprintf(&name, "%s=", ast_str(fst));
    new = (ast_t) {
        .type = AST_SYMBOL,
        .symbol = r_intern(name),
    };
    xfree(name);
    array_insert(arr, 2, *rhs);
    *rhs = (ast_t) { .type = AST_INVALID };
    return convert_call(ir, &new, arr);
}
static bool convert_set(ir_builder_t *ir, ast_t *lhs, ast_t *rhs)
{
    if(set_expandp(lhs))
        return convert_set_expand(ir, lhs->children, rhs);
    if(lhs->type != AST_SYMBOL)
    {
        ir_error(ir, "no set-expansion for expression.");
        return false;
    }

    cvar_t *var = env_find_add(ir->env, lhs->symbol);

    if(!cvar_is_global(var))
        var->is_const = false;
    if(!convert_expr(ir, rhs))
        return false;
    build_set(ir, var, ir->value);
    return true;
}
static bool convert_return(ir_builder_t *ir, ast_t *expr)
{
    if(convert_expr(ir, expr))
        build_return(ir);
    return false;
}
static bool convert_loop(ir_builder_t *ir, ast_t *decls, ast_t *cond,
                         ast_t *iter, ast_t *body)
{
    ir_builder_t prehead, header, inner, footer, after;
    lexenv_t env;
    cblock_t *next;
    bool open;
    ir_open(&prehead, ir);
    link_blocks(prehead.head, ir->tail);
    if(!ast_is_omitted(decls))
    {
        prehead.env = env_open(&env, ir->env);
        open = convert_let(&prehead, decls, true);
        if(!open)
            goto out;
    }
    ir_open(&header, &prehead);
    open = convert_expr(&header, cond);
    next = header.head;
    link_blocks(header.head, prehead.tail);
    if(!open)
        goto out;
    build_if(&header, header.value);
    ir_open(&after, ir);
    if(!ast_is_omitted(iter))
    {
        ir_open(&footer, &prehead);
        open = convert_stmt(&footer, iter);
        next = footer.head;
    }
    ir_open(&inner, &prehead);
    inner.loop = &(loopenv_t) { .brk = after.head, .cont = next };
    if(convert_stmt(&inner, body))
        link_blocks(next, inner.tail); // body tail -> around again
    else
        open = false; // control flow does not enter iter
    if(!ast_is_omitted(iter) && open)
        link_blocks(header.head, footer.tail);
    link_blocks(inner.head, header.tail);
    link_blocks(after.head, header.tail);
    ir_extend(ir, &after);
    open = true;
out:
    if(!ast_is_omitted(decls))
        env_close(&env);
    return open;
}
static bool convert_loop_exit(ir_builder_t *ir, ast_t *ast)
{
    bool is_break = (ast->token == BREAK);

    if(!ir->loop)
        ir_error(ir, "%s outside loop.",
                 is_break ? "break" : "continue");
    else
        link_blocks(is_break ? ir->loop->brk : ir->loop->cont,
                    ir->tail);
    return false;
}
static bool convert_literal_type(ir_builder_t *ir, ast_t *ast)
{
    rtype_t *type = rtype_from_spec(ast);

    if(!type)
    {
        ir_error(ir, "unknown type in `type` expression.");
        return false;
    }
    build_constant(ir, (robject_t *)type);
    return true;
}
static bool convert_file(ir_builder_t *ir, ast_t *ast)
{
    bool open = true;
    assert(ast->type == AST_NODE);
    for(int i = 0; i < alen(ast->children) && open; i++)
        open = convert_stmt(ir, aptr(ast->children, i));
    return open;
}
static bool convert_include(ir_builder_t *ir, ast_t *arg)
{
    assert(arg->type == AST_STRING);
    char *filename = arg->string;
    ast_t ast;
    ir_builder_t inner;
    bool open;
    int r;

    assert(filename);
    ir_open(&inner, ir);
    link_blocks(inner.head, ir->tail);
    inner.file = r_intern(filename);
    r = p_source(filename, &ast);
    if(r != 0)
    {
        *ir->pfailed = true;
        if(r < 0)
            ir_error(ir, "including %s - %s", filename,
                     strerror(errno));
        return false;
    }
    open = convert_file(&inner, &ast);
    ast_fini(&ast);
    ir_extend(ir, &inner);
    return open;
}
static bool convert_literal(ir_builder_t *ir, ast_t *ast)
{
    build_constant(ir, constant_from_literal(ast));
    return true;
}
static bool convert_symbol(ir_builder_t *ir, ast_t *ast)
{
    if(ast_is_rest(ast) && is_top_level(ir))
    {
        ir_error(ir, "invalid reference to `...`.");
        return false;
    }
    build_ref(ir, ast->symbol);
    return true;
}
static inline bool
convert_token(ir_builder_t *ir, const char *name, ast_array_t *arr)
{
    ast_t ast = {
        .type = AST_SYMBOL,
        .symbol = r_intern(name),
    };
    return convert_call(ir, &ast, arr);
}

static bool convert_node(ir_builder_t *ir, ast_t *ast)
{
    ast_array_t *arr = ast->children;
    ast_t *fst = aptr(arr, 0);

    if(fst->type != AST_TOKEN)
        return convert_call(ir, fst, arr);
    switch(fst->token)
    {
        case LET:
        case VAR: assert(alen(arr) == 2);
            return convert_let(ir, aptr(arr, 1), fst->token == LET);
        case GLOBAL:
        case CONST: assert(alen(arr) == 2);
            return convert_global(ir, aptr(arr, 1), fst->token == CONST);
        case FUNCTION: assert(alen(arr) == 4);
            return convert_lambda(ir, rtype_from_spec(aptr(arr, 1)),
                                  aptr(arr, 2)->children, aptr(arr, 3));
        case '{':
            return convert_prog(ir, arr);
        case IF: assert(alen(arr) == 3 || alen(arr) == 4);
            return convert_ifelse(ir, aptr(arr, 1), aptr(arr, 2),
                                  (alen(arr) == 4) ? aptr(arr, 3) : NULL);
        case OR: assert(alen(arr) == 3);
            return convert_or(ir, aptr(arr, 1), aptr(arr, 2));
        case AND: assert(alen(arr) == 3);
            return convert_and(ir, aptr(arr, 1), aptr(arr, 2));
        case '=': assert(alen(arr) == 3);
            return convert_set(ir, aptr(arr, 1), aptr(arr, 2));
        case RET: assert(alen(arr) == 2);
            return convert_return(ir, aptr(arr, 1));
        case WHILE: assert(alen(arr) == 3);
            return convert_loop(ir, &ast_null, aptr(arr, 1), &ast_null,
                                aptr(arr, 2));
        case FOR: assert(alen(arr) == 5);
            return convert_loop(ir, aptr(arr, 1), aptr(arr, 2), aptr(arr, 3),
                                aptr(arr, 4));
        case TYPE: assert(alen(arr) == 2);
            return convert_literal_type(ir, aptr(arr, 1));
        case INCLUDE: assert(alen(arr) == 2);
            return convert_include(ir, aptr(arr, 1));
        case '+':
            if(alen(arr) == 2)
                return convert_expr(ir, aptr(arr, 1));
            /* fallthrough */
        case '-':
            if(alen(arr) == 2)
                return convert_token(ir, "0-", arr);
            /* fallthrough */
        default:
            break;
    }
    return convert_token(ir, ast_str(fst), arr);
}
static bool convert_stmt(ir_builder_t *ir, ast_t *ast)
{
    switch(ast->type)
    {
    case AST_INT:
    case AST_DOUBLE:
    case AST_QUOTED:
    case AST_STRING:
        return convert_literal(ir, ast);
    case AST_SYMBOL:
        return convert_symbol(ir, ast);
    case AST_NODE:
        return convert_node(ir, ast);
    case AST_TOKEN:
        if(ast->token == BREAK || ast->token == CONTINUE)
            return convert_loop_exit(ir, ast);
        /* fallthrough */
    default:
        ir_error(ir, "invalid AST of type %d", ast->type);
    }
    return false; /* NOTREACHED, ideally */
}
static bool convert_expr(ir_builder_t *ir, ast_t *ast)
{
    if(convert_stmt(ir, ast) && ir->value)
        return true;
    if(ir->pfailed && !*ir->pfailed)
        ir_error(ir, "statement found where expression expected.");
    return false;
}
static inline bool check_resolve(cvar_t *var, rglobal_t *global)
{
    if(var->type == GLOBAL_INT)
    {
        c_error("duplicate %s definition `%s`",
                var->is_const ? "const" : "global", r_symstr(var->name));
        return false;
    }
    if(global->decl)
    {
        var->extl.global = global;
        var->decl = global->decl;
        var->is_const = global->is_const;
    }
    return true;
}
static void resolve_global(const void *key, void *value, void *ptr)
{
    ir_builder_t *ir = ptr;
    cvar_t *var = value;
    rglobal_t *global = r_get_global(var->name);
    cfunction_t *fn = ir->fun->function;

    if(global && !check_resolve(var, global))
        *ir->pfailed = true;

    assert(cvar_is_global(var));
    assert(list_isempty(&var->cvar_list));
    list_add(&fn->cvar_head, &var->cvar_list);
}
cfunction_t *ir_convert(ast_t *ast, char *filename)
{
    cfunction_t *function;
    funenv_t fun;
    lexenv_t global_env, env;
    bool failed = false;
    env_open(&global_env, NULL);
    ir_builder_t ir = {
        .fun = fun_open(&fun, NULL),
        .env = env_open(&env, &global_env),
        .file = filename ? r_intern(filename) : NULL,
        .pfailed = &failed
    };
    ir_open(&ir, NULL);
    if(convert_file(&ir, ast))
        build_return(&ir);
    function = fun_close(&fun, ir.head, NULL);
    env_close(&env);
    if(global_env.vars)
        hashmap_map(global_env.vars, resolve_global, &ir);
    env_close(&global_env);
    if(failed)
    {
        cfunc_free(function);
        return NULL;
    }
    return function;
}
