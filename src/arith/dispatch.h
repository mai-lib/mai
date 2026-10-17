// builtin data
typedef struct
{
    const void *fntab; // table of function pointers
    bool is_pred; // whether this builtin a predicate
    op_code_t op; // vm instruction opcode
} arith_builtin_t;

// binary operation dispatch
typedef struct
{
    rtype_t *typ, *optyp;
    size_t xsz, ysz, rsz;
    scalar_code xcode, ycode;
    bool xvec, yvec; // 'is array or vector', really
    bool xarr, yarr; // 'is array and not vector'
} binop_chk_t;

bool check_binary(binop_chk_t *chk, rtype_t *xt, rtype_t *yt, bool pred);

static inline binop_fn dispatch_binary(binop_chk_t *chk, binop_fntab fntab)
{
    return fntab[chk->xvec][chk->yvec][chk->xcode][chk->ycode];
}

// unary operation dispatch
typedef struct
{
    rtype_t *typ;
    scalar_code code;
    bool vec, arr; // same as for binop
} unop_chk_t;

bool check_unary(unop_chk_t *chk, rtype_t *xt, bool pred);

static inline unop_fn dispatch_unary(unop_chk_t *chk, unop_fntab fntab)
{
    return fntab[chk->vec][chk->code];
}

// reduction dispatch
typedef struct
{
    rtype_t *typ; // always a scalar
    scalar_code code;
    bool vec;
} reduce_chk_t;

bool check_reduce(reduce_chk_t *chk, rtype_t *xt);

static inline unop_fn dispatch_reduce(reduce_chk_t *chk, reduce_fntab fntab)
{
    return fntab[chk->code];
}

// conversion dispatch
typedef struct
{
    rtype_t *rt;
    bool vec, arr, generic;
    scalar_code fromcode, tocode;
} conv_chk_t;

// from could be anything; to is always a scalar
bool check_conv(conv_chk_t *chk, rtype_t *from, rtype_t *to);

static inline conv_fn dispatch_conv(conv_chk_t *chk)
{
    return conv_funcs[chk->vec][chk->fromcode][chk->tocode];
}

// collection fetch and store

// scalar, vector, boolean, or omitted index
typedef enum { SINGLE, INDEX, MASK, MISSING } idx_kind;

// vector
typedef struct
{
    idx_kind kind;
    bool takeptr; // use replacement/result value directly, do not indirect
    bool names; // fetch element names
    rtype_t *typ; // result type/replacement element type
} idx_chk_t;

bool check_fetch(idx_chk_t *chk, rtype_t *typ, rtype_t *ityp);
bool check_store(idx_chk_t *chk, rtype_t *typ, rtype_t *ityp, rtype_t *rtyp);

// array
typedef struct
{
    idx_kind *kinds; // one per index, _including_ the first
    bool takeptr;
    int rank; // rank of indexed value - 0: scalar, 1: vector, >1: array
    rtype_t *typ;  // element type of result/replacement
} arr_chk_t;

bool check_arr_fetch(arr_chk_t *achk, rtype_t *typ, rtype_t **ityps,
                     int ntyps);
bool check_arr_store(arr_chk_t *achk, rtype_t *typ, rtype_t **ityps,
                     int ntyps, rtype_t *rtyp);
