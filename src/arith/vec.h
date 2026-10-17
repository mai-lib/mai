/*
  


M4 bind the first argument (name) as an element of the second (quoted list)
M4 and expand the third argument (quotation) for each one in order





M4 usual list routines




M4 token-pasting


M4 diversion wrapper


M4 names of things







M4 static arithmetic promotion: expand to the argument found first in `types`
M4 FIXME should be ifelse(car($3), `$1', match one, car($3), `$2', match two, no match)



M4 convenient synonyms







M4 m4_define({{op_binary}}, {{$1 cadr(op_spec) $2}})


M4 m4_define({{op_unary}}, {{cadr(op_spec) $1}})



M4 double ops have no overflow





M4 loops




M4 configuration




M4 on numeric types, return promoted

M4 on numeric types, return same

M4 on numeric types, return same, no analogous symbol
M4 use cadr(op_spec) for identity instead


M4 on numeric types, return boolean


M4 on scalar types, return boolean

M4 on scalar types, return boolean
M4 none for now
M4 m4_define({{eql_unary_ops}}, {{{{something}}}})

M4 on booleans only

M4 on booleans only

M4 on booleans only, etc, etc


M4 on doubles only



*/
// 
typedef void (*binop_fn)(void *, void *, void *, int);
typedef void (*unop_fn)(void *, void *, int);
typedef void (*conv_fn)(void *, void *, int);
typedef void (*fill_na_fn)(void *, size_t );
typedef void (*idx_fn)(void *, void *, void *, int, int, int);
// 

// 
// 
// 
// 
// 
  
    
    

extern const binop_fn add_funcs[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR];
// 
// 
    
  
    
    

extern const binop_fn sub_funcs[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR];
// 
// 
    
  
    
    

extern const binop_fn mul_funcs[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR];
// 
// 
    
  
    
    

extern const binop_fn div_funcs[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR];
// 
// 
    
  
    
    

extern const binop_fn pow_funcs[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR];
// 
// 
    
  
    
    

extern const binop_fn lth_funcs[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR];
// 
// 
    
  
    
    

extern const binop_fn lte_funcs[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR];
// 
// 
    
  
    
    

extern const binop_fn gth_funcs[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR];
// 
// 
    
  
    
    

extern const binop_fn gte_funcs[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR];
// 
// 
    
  
    
    

extern const binop_fn eql_funcs[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR];
// 
// 
    
  
    
    

extern const binop_fn neq_funcs[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR];
// 
// 
    
  
    
    

extern const binop_fn and_funcs[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR];
// 
// 
    
  
    
    

extern const binop_fn or_funcs[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR];
// 
// 
    
  
    
    

extern const binop_fn atan2_funcs[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR];
// 
// 
    
  

  

  

  

  

  

  

  

  

  

  

  

  

  

  

// 
  
    
    

extern const unop_fn neg_funcs[2][CODE_MAX_SCALAR];
// 
// 
    
  
    
    

extern const unop_fn not_funcs[2][CODE_MAX_SCALAR];
// 
// 
    
  
    
    

extern const unop_fn sqrt_funcs[2][CODE_MAX_SCALAR];
// 
// 
    
  
    
    

extern const unop_fn sin_funcs[2][CODE_MAX_SCALAR];
// 
// 
    
  
    
    

extern const unop_fn cos_funcs[2][CODE_MAX_SCALAR];
// 
// 
    
  
    
    

extern const unop_fn tan_funcs[2][CODE_MAX_SCALAR];
// 
// 
    
  
    
    

extern const unop_fn asin_funcs[2][CODE_MAX_SCALAR];
// 
// 
    
  
    
    

extern const unop_fn acos_funcs[2][CODE_MAX_SCALAR];
// 
// 
    
  
    
    

extern const unop_fn atan_funcs[2][CODE_MAX_SCALAR];
// 
// 
    
  
    
    

extern const unop_fn log_funcs[2][CODE_MAX_SCALAR];
// 
// 
    
  
    
    

extern const unop_fn exp_funcs[2][CODE_MAX_SCALAR];
// 
// 
    
  
    
    

extern const unop_fn round_funcs[2][CODE_MAX_SCALAR];
// 
// 
    
  
    
    

extern const unop_fn is_na_funcs[2][CODE_MAX_SCALAR];
// 
// 
    
  

  

  

  

  

  

  

  

  

  

  

  

  

  

// 
  
    
    

extern const unop_fn reduce_add_funcs[CODE_MAX_SCALAR];
// 
// 
    
  
    
    

extern const unop_fn reduce_mul_funcs[CODE_MAX_SCALAR];
// 
// 
    
  
    
    

extern const unop_fn reduce_and_funcs[CODE_MAX_SCALAR];
// 
// 
    
  
    
    

extern const unop_fn reduce_or_funcs[CODE_MAX_SCALAR];
// 
// 
    
  

  

  

  

  


extern const conv_fn conv_funcs[2][CODE_MAX_SCALAR][CODE_MAX_SCALAR];
// 
// 
typedef const binop_fn binop_fntab[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR];
typedef const unop_fn unop_fntab[2][CODE_MAX_SCALAR];
typedef const unop_fn reduce_fntab[CODE_MAX_SCALAR];
// return the element type for an operand, given an aggregate or scalar type.
// returns NULL if typ isn't a scalar or an aggregate containing scalars.
static inline rtype_t *arith_elt_type(rtype_t *typ)
{
    if(rtype_is_container(typ))
        typ = typ->elt;
    return rtype_is_scalar(typ) ? typ : NULL;
}

// recycle src along dest
// slen/dlen lengths in elements, sz element size in bytes
static inline
void copy_recycle(void *dest, void *src, int dlen, int slen, size_t sz)
{
    if(slen == 0)
    {
        memset(dest, 0, sz * dlen);
        return;
    }
    if(slen >= dlen)
    {
        memcpy(dest, src, sz * dlen);
        return;
    }

    ptrdiff_t adv = slen * sz;
    uint8_t *end = (uint8_t *)dest + dlen * sz;
    for(uint8_t *ptr = dest; ptr < end; ptr += adv)
        memcpy(ptr, src, min(adv, end - ptr));
}

static inline rboolean_t scalar_is_na(rtype_t *type, void *src)
{
    assert(rtype_is_scalar(type));
    rboolean_t res;
    unop_fn is_na = is_na_funcs[CODE_sca][rscal_code(type)];
    is_na(&res, src, 1);
    return res;
}
robject_t *arith_binary(vm_ctx_t *vm, binop_fntab fntab, bool pred,
                        robject_t *xobj, robject_t *yobj);
robject_t *arith_unary(vm_ctx_t *vm, unop_fntab fntab, bool pred,
                       robject_t *xobj);
robject_t *arith_convert(vm_ctx_t *vm, robject_t *obj, rtype_t *to);
robject_t *arith_reduce(vm_ctx_t *vm, reduce_fntab fntab, robject_t *xobj);
bool scalar_convert(void *dest, robject_t *obj, rtype_t *to, rtype_t *from);

robject_t *vec_fetch(vm_ctx_t *vm, robject_t *obj, robject_t *idx);
robject_t *vec_store(vm_ctx_t *vm, robject_t *obj, robject_t *val,
                     robject_t *idx);
robject_t *arr_fetch(vm_ctx_t *vm, robject_t *obj, robject_t *idx,
                     robject_t **idxs, int nidxs);
robject_t *arr_store(vm_ctx_t *vm, robject_t *obj, robject_t *val,
                     robject_t *idx, robject_t **idxs, int nidxs);
void copy_names(robject_t *dest, robject_t *src);
void rt_install_vectors();
// 
