const extern builtin_ops_t pure_ops, type_ops, typeof_ops;

typedef struct
{
    int arity;
    union
    {
        rtype_t *(*unfn)(const void *);
        rtype_t *(*binfn)(rtype_t *, rtype_t *);
    };
} type_op_fn_t;

#define type_unop(fn) (&(type_op_fn_t){ .arity = 1, .unfn = fn })
#define type_binop(fn) (&(type_op_fn_t){ .arity = 2, .binfn = fn })
