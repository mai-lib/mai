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

// return true if the value is NA
#define boolean_na(v) ((v) == rboolean_na)
#define int_na(v) ((v) == rint_na)
#define double_na(v) (isnan(v))
// convert the value to the type
#define boolean_conv(v) ((v) != 0)
#define double_conv(v) (rdouble_t)(v)
#define int_conv(v) ((((v) > INT_MAX) | ((v) <= INT_MIN)) ? rint_na : (rint_t)(v))

// do not take `r`: compute it explicitly and hope that common
// subexpression elimination does its thing.
// return true if the operation over- or underflowed
#define add_xflow(x, y) ((x > 0) ^ (y < (x + y)))
#define signs(a, b) ((a < 0) ^ (b < 0))
#define sub_xflow(x, y) (signs(x, y) & signs(x, (x - y)))
#define mul_xflow(x, y) (((int64_t)x * (int64_t)y) != (x * y))
#define div_xflow(x, y) (y==0)
// XXX inefficient
#define pow_xflow(x, y) (fabs(pow(x,y)) > (INT_MAX-1))

// 
// 
// M4 prefer native double NA handling, thanks to C arithmetic conversion
// 

// 
  
  
    
    
  
  
    
    
  
// 
  
    
    

static inline rdouble_t arith_add_double_double(rdouble_t x, rdouble_t y)
{
    return (0 | 0 | 0)
        ? rdouble_na
        : x + y;
}
// 
// 
    
  
    
    

static inline rdouble_t arith_sub_double_double(rdouble_t x, rdouble_t y)
{
    return (0 | 0 | 0)
        ? rdouble_na
        : x - y;
}
// 
// 
    
  
    
    

static inline rdouble_t arith_mul_double_double(rdouble_t x, rdouble_t y)
{
    return (0 | 0 | 0)
        ? rdouble_na
        : x * y;
}
// 
// 
    
  
    
    

static inline rdouble_t arith_div_double_double(rdouble_t x, rdouble_t y)
{
    return (0 | 0 | 0)
        ? rdouble_na
        : x / y;
}
// 
// 
    
  
    
    

static inline rdouble_t arith_pow_double_double(rdouble_t x, rdouble_t y)
{
    return (0 | 0 | 0)
        ? rdouble_na
        : pow(x, y);
}
// 
// 
    
  

  

  

  

  

  

// 
  
    
  
    
    
  
// 
  
    
    

static inline rdouble_t arith_add_double_int(rdouble_t x, rint_t y)
{
    return (0 | int_na(y) | 0)
        ? rdouble_na
        : x + y;
}
// 
// 
    
  
    
    

static inline rdouble_t arith_sub_double_int(rdouble_t x, rint_t y)
{
    return (0 | int_na(y) | 0)
        ? rdouble_na
        : x - y;
}
// 
// 
    
  
    
    

static inline rdouble_t arith_mul_double_int(rdouble_t x, rint_t y)
{
    return (0 | int_na(y) | 0)
        ? rdouble_na
        : x * y;
}
// 
// 
    
  
    
    

static inline rdouble_t arith_div_double_int(rdouble_t x, rint_t y)
{
    return (0 | int_na(y) | 0)
        ? rdouble_na
        : x / y;
}
// 
// 
    
  
    
    

static inline rdouble_t arith_pow_double_int(rdouble_t x, rint_t y)
{
    return (0 | int_na(y) | 0)
        ? rdouble_na
        : pow(x, y);
}
// 
// 
    
  

  

  

  

  

  

// 
  
    
  

  

  

  
    
  
    
    
  
  
    
    
  
// 
  
    
    

static inline rdouble_t arith_add_int_double(rint_t x, rdouble_t y)
{
    return (int_na(x) | 0 | 0)
        ? rdouble_na
        : x + y;
}
// 
// 
    
  
    
    

static inline rdouble_t arith_sub_int_double(rint_t x, rdouble_t y)
{
    return (int_na(x) | 0 | 0)
        ? rdouble_na
        : x - y;
}
// 
// 
    
  
    
    

static inline rdouble_t arith_mul_int_double(rint_t x, rdouble_t y)
{
    return (int_na(x) | 0 | 0)
        ? rdouble_na
        : x * y;
}
// 
// 
    
  
    
    

static inline rdouble_t arith_div_int_double(rint_t x, rdouble_t y)
{
    return (int_na(x) | 0 | 0)
        ? rdouble_na
        : x / y;
}
// 
// 
    
  
    
    

static inline rdouble_t arith_pow_int_double(rint_t x, rdouble_t y)
{
    return (int_na(x) | 0 | 0)
        ? rdouble_na
        : pow(x, y);
}
// 
// 
    
  

  

  

  

  

  

// 
  
    
  
    
    
  
// 
  
    
    

static inline rint_t arith_add_int_int(rint_t x, rint_t y)
{
    return (int_na(x) | int_na(y) | add_xflow(x, y))
        ? rint_na
        : x + y;
}
// 
// 
    
  
    
    

static inline rint_t arith_sub_int_int(rint_t x, rint_t y)
{
    return (int_na(x) | int_na(y) | sub_xflow(x, y))
        ? rint_na
        : x - y;
}
// 
// 
    
  
    
    

static inline rint_t arith_mul_int_int(rint_t x, rint_t y)
{
    return (int_na(x) | int_na(y) | mul_xflow(x, y))
        ? rint_na
        : x * y;
}
// 
// 
    
  
    
    

static inline rint_t arith_div_int_int(rint_t x, rint_t y)
{
    return (int_na(x) | int_na(y) | div_xflow(x, y))
        ? rint_na
        : x / y;
}
// 
// 
    
  
    
    

static inline rint_t arith_pow_int_int(rint_t x, rint_t y)
{
    return (int_na(x) | int_na(y) | pow_xflow(x, y))
        ? rint_na
        : pow(x, y);
}
// 
// 
    
  

  

  

  

  

  

// 
  
    
  

  

  

  
    
  

  

  



// 
  
    
    
// 
  
    
    

static inline rdouble_t arith_neg_double(rdouble_t x)
{
    rdouble_t r = - x;
    return 0 ? rdouble_na : r;
}
// 
// 
    
  

  

// 
    
  
    
    
// 
  
    
    

static inline rint_t arith_neg_int(rint_t x)
{
    rint_t r = - x;
    return int_na(x) ? rint_na : r;
}
// 
// 
    
  

  

// 
    
  

  

  


// M4 for other ops, we must explicitly check
// 
// M4 overflow doesn't occur outside arithmetic
// 
// M4 ops from now on return boolean
// 

// 
  
  
    
    
  
  
    
    
  
// 
  
    
    

static inline rboolean_t arith_lth_double_double(rdouble_t x, rdouble_t y)
{
    return (double_na(x) | double_na(y) | 0)
        ? rboolean_na
        : x < y;
}
// 
// 
    
  
    
    

static inline rboolean_t arith_lte_double_double(rdouble_t x, rdouble_t y)
{
    return (double_na(x) | double_na(y) | 0)
        ? rboolean_na
        : x <= y;
}
// 
// 
    
  
    
    

static inline rboolean_t arith_gth_double_double(rdouble_t x, rdouble_t y)
{
    return (double_na(x) | double_na(y) | 0)
        ? rboolean_na
        : x > y;
}
// 
// 
    
  
    
    

static inline rboolean_t arith_gte_double_double(rdouble_t x, rdouble_t y)
{
    return (double_na(x) | double_na(y) | 0)
        ? rboolean_na
        : x >= y;
}
// 
// 
    
  

  

  

  

  

// 
  
    
  
    
    
  
// 
  
    
    

static inline rboolean_t arith_lth_double_int(rdouble_t x, rint_t y)
{
    return (double_na(x) | int_na(y) | 0)
        ? rboolean_na
        : x < y;
}
// 
// 
    
  
    
    

static inline rboolean_t arith_lte_double_int(rdouble_t x, rint_t y)
{
    return (double_na(x) | int_na(y) | 0)
        ? rboolean_na
        : x <= y;
}
// 
// 
    
  
    
    

static inline rboolean_t arith_gth_double_int(rdouble_t x, rint_t y)
{
    return (double_na(x) | int_na(y) | 0)
        ? rboolean_na
        : x > y;
}
// 
// 
    
  
    
    

static inline rboolean_t arith_gte_double_int(rdouble_t x, rint_t y)
{
    return (double_na(x) | int_na(y) | 0)
        ? rboolean_na
        : x >= y;
}
// 
// 
    
  

  

  

  

  

// 
  
    
  

  

  

  
    
  
    
    
  
  
    
    
  
// 
  
    
    

static inline rboolean_t arith_lth_int_double(rint_t x, rdouble_t y)
{
    return (int_na(x) | double_na(y) | 0)
        ? rboolean_na
        : x < y;
}
// 
// 
    
  
    
    

static inline rboolean_t arith_lte_int_double(rint_t x, rdouble_t y)
{
    return (int_na(x) | double_na(y) | 0)
        ? rboolean_na
        : x <= y;
}
// 
// 
    
  
    
    

static inline rboolean_t arith_gth_int_double(rint_t x, rdouble_t y)
{
    return (int_na(x) | double_na(y) | 0)
        ? rboolean_na
        : x > y;
}
// 
// 
    
  
    
    

static inline rboolean_t arith_gte_int_double(rint_t x, rdouble_t y)
{
    return (int_na(x) | double_na(y) | 0)
        ? rboolean_na
        : x >= y;
}
// 
// 
    
  

  

  

  

  

// 
  
    
  
    
    
  
// 
  
    
    

static inline rboolean_t arith_lth_int_int(rint_t x, rint_t y)
{
    return (int_na(x) | int_na(y) | 0)
        ? rboolean_na
        : x < y;
}
// 
// 
    
  
    
    

static inline rboolean_t arith_lte_int_int(rint_t x, rint_t y)
{
    return (int_na(x) | int_na(y) | 0)
        ? rboolean_na
        : x <= y;
}
// 
// 
    
  
    
    

static inline rboolean_t arith_gth_int_int(rint_t x, rint_t y)
{
    return (int_na(x) | int_na(y) | 0)
        ? rboolean_na
        : x > y;
}
// 
// 
    
  
    
    

static inline rboolean_t arith_gte_int_int(rint_t x, rint_t y)
{
    return (int_na(x) | int_na(y) | 0)
        ? rboolean_na
        : x >= y;
}
// 
// 
    
  

  

  

  

  

// 
  
    
  

  

  

  
    
  

  

  


// 
  
  
    
    
  
  
    
    
  
// 
  
    
    

static inline rboolean_t arith_eql_double_double(rdouble_t x, rdouble_t y)
{
    return (double_na(x) | double_na(y) | 0)
        ? rboolean_na
        : x == y;
}
// 
// 
    
  
    
    

static inline rboolean_t arith_neq_double_double(rdouble_t x, rdouble_t y)
{
    return (double_na(x) | double_na(y) | 0)
        ? rboolean_na
        : x != y;
}
// 
// 
    
  

  

  

// 
  
    
  
    
    
  
// 
  
    
    

static inline rboolean_t arith_eql_double_int(rdouble_t x, rint_t y)
{
    return (double_na(x) | int_na(y) | 0)
        ? rboolean_na
        : x == y;
}
// 
// 
    
  
    
    

static inline rboolean_t arith_neq_double_int(rdouble_t x, rint_t y)
{
    return (double_na(x) | int_na(y) | 0)
        ? rboolean_na
        : x != y;
}
// 
// 
    
  

  

  

// 
  
    
  
    
    
  
// 
  
    
    

static inline rboolean_t arith_eql_double_boolean(rdouble_t x, rboolean_t y)
{
    return (double_na(x) | boolean_na(y) | 0)
        ? rboolean_na
        : x == y;
}
// 
// 
    
  
    
    

static inline rboolean_t arith_neq_double_boolean(rdouble_t x, rboolean_t y)
{
    return (double_na(x) | boolean_na(y) | 0)
        ? rboolean_na
        : x != y;
}
// 
// 
    
  

  

  

// 
  
    
  

  

  

  

  
    
  
    
    
  
  
    
    
  
// 
  
    
    

static inline rboolean_t arith_eql_int_double(rint_t x, rdouble_t y)
{
    return (int_na(x) | double_na(y) | 0)
        ? rboolean_na
        : x == y;
}
// 
// 
    
  
    
    

static inline rboolean_t arith_neq_int_double(rint_t x, rdouble_t y)
{
    return (int_na(x) | double_na(y) | 0)
        ? rboolean_na
        : x != y;
}
// 
// 
    
  

  

  

// 
  
    
  
    
    
  
// 
  
    
    

static inline rboolean_t arith_eql_int_int(rint_t x, rint_t y)
{
    return (int_na(x) | int_na(y) | 0)
        ? rboolean_na
        : x == y;
}
// 
// 
    
  
    
    

static inline rboolean_t arith_neq_int_int(rint_t x, rint_t y)
{
    return (int_na(x) | int_na(y) | 0)
        ? rboolean_na
        : x != y;
}
// 
// 
    
  

  

  

// 
  
    
  
    
    
  
// 
  
    
    

static inline rboolean_t arith_eql_int_boolean(rint_t x, rboolean_t y)
{
    return (int_na(x) | boolean_na(y) | 0)
        ? rboolean_na
        : x == y;
}
// 
// 
    
  
    
    

static inline rboolean_t arith_neq_int_boolean(rint_t x, rboolean_t y)
{
    return (int_na(x) | boolean_na(y) | 0)
        ? rboolean_na
        : x != y;
}
// 
// 
    
  

  

  

// 
  
    
  

  

  

  

  
    
  
    
    
  
  
    
    
  
// 
  
    
    

static inline rboolean_t arith_eql_boolean_double(rboolean_t x, rdouble_t y)
{
    return (boolean_na(x) | double_na(y) | 0)
        ? rboolean_na
        : x == y;
}
// 
// 
    
  
    
    

static inline rboolean_t arith_neq_boolean_double(rboolean_t x, rdouble_t y)
{
    return (boolean_na(x) | double_na(y) | 0)
        ? rboolean_na
        : x != y;
}
// 
// 
    
  

  

  

// 
  
    
  
    
    
  
// 
  
    
    

static inline rboolean_t arith_eql_boolean_int(rboolean_t x, rint_t y)
{
    return (boolean_na(x) | int_na(y) | 0)
        ? rboolean_na
        : x == y;
}
// 
// 
    
  
    
    

static inline rboolean_t arith_neq_boolean_int(rboolean_t x, rint_t y)
{
    return (boolean_na(x) | int_na(y) | 0)
        ? rboolean_na
        : x != y;
}
// 
// 
    
  

  

  

// 
  
    
  
    
    
  
// 
  
    
    

static inline rboolean_t arith_eql_boolean_boolean(rboolean_t x, rboolean_t y)
{
    return (boolean_na(x) | boolean_na(y) | 0)
        ? rboolean_na
        : x == y;
}
// 
// 
    
  
    
    

static inline rboolean_t arith_neq_boolean_boolean(rboolean_t x, rboolean_t y)
{
    return (boolean_na(x) | boolean_na(y) | 0)
        ? rboolean_na
        : x != y;
}
// 
// 
    
  

  

  

// 
  
    
  

  

  

  

  
    
  

  

  

  



//  
// 
  
    
    

static inline rboolean_t arith_and_boolean_boolean(rboolean_t x, rboolean_t y)
{
    return (boolean_na(x) | boolean_na(y) | 0)
        ? rboolean_na
        : x & y;
}
// 
// 
    
  
    
    

static inline rboolean_t arith_or_boolean_boolean(rboolean_t x, rboolean_t y)
{
    return (boolean_na(x) | boolean_na(y) | 0)
        ? rboolean_na
        : x | y;
}
// 
// 
    
  

  

  

// 
  
    
    

static inline rboolean_t arith_not_boolean(rboolean_t x)
{
    rboolean_t r = ! x;
    return boolean_na(x) ? rboolean_na : r;
}
// 
// 
    
  

  

//  
// 
  
    
    
static inline rboolean_t arith_is_na_double(rdouble_t x)
{
    return double_na(x);
}
// 
    
  
    
    
static inline rboolean_t arith_is_na_int(rint_t x)
{
    return int_na(x);
}
// 
    
  
    
    
static inline rboolean_t arith_is_na_boolean(rboolean_t x)
{
    return boolean_na(x);
}
// 
    
  

  

  

  


// M4 reset defs to default
// 
// 
// 
  
  
    
    
  
  
    
    
  
// 
// 
  
    
  
    
    
  
// 
static inline rint_t arith_conv_double_to_int(rdouble_t x)
{
    return (double_na(x)) ? rint_na : int_conv(x);
}
// 
// 
  
    
  
    
    
  
// 
static inline rboolean_t arith_conv_double_to_boolean(rdouble_t x)
{
    return (double_na(x)) ? rboolean_na : boolean_conv(x);
}
// 
// 
  
    
  

  

  

  

  
    
  
    
    
  
  
    
    
  
// 
static inline rdouble_t arith_conv_int_to_double(rint_t x)
{
    return (int_na(x)) ? rdouble_na : double_conv(x);
}
// 
// 
  
    
  
    
    
  
// 
// 
  
    
  
    
    
  
// 
static inline rboolean_t arith_conv_int_to_boolean(rint_t x)
{
    return (int_na(x)) ? rboolean_na : boolean_conv(x);
}
// 
// 
  
    
  

  

  

  

  
    
  
    
    
  
  
    
    
  
// 
static inline rdouble_t arith_conv_boolean_to_double(rboolean_t x)
{
    return (boolean_na(x)) ? rdouble_na : double_conv(x);
}
// 
// 
  
    
  
    
    
  
// 
static inline rint_t arith_conv_boolean_to_int(rboolean_t x)
{
    return (boolean_na(x)) ? rint_na : int_conv(x);
}
// 
// 
  
    
  
    
    
  
// 
// 
  
    
  

  

  

  

  
    
  

  

  

  


// 
  
    
    
static inline rdouble_t arith_conv_ptr_to_double(robject_t *x)
{
    if(r_typeof(x) != r_type_double)
        return rdouble_na;
    return UNBOX(rdouble_t, x);
}

static inline robject_t *arith_conv_double_to_ptr(rdouble_t x)
{
    robject_t *box = r_box_create(r_type_double);
    UNBOX(rdouble_t, box) = x;
    return box;
}
// 
    
  
    
    
static inline rint_t arith_conv_ptr_to_int(robject_t *x)
{
    if(r_typeof(x) != r_type_int)
        return rint_na;
    return UNBOX(rint_t, x);
}

static inline robject_t *arith_conv_int_to_ptr(rint_t x)
{
    robject_t *box = r_box_create(r_type_int);
    UNBOX(rint_t, box) = x;
    return box;
}
// 
    
  
    
    
static inline rboolean_t arith_conv_ptr_to_boolean(robject_t *x)
{
    if(r_typeof(x) != r_type_boolean)
        return rboolean_na;
    return UNBOX(rboolean_t, x);
}

static inline robject_t *arith_conv_boolean_to_ptr(rboolean_t x)
{
    robject_t *box = r_box_create(r_type_boolean);
    UNBOX(rboolean_t, box) = x;
    return box;
}
// 
    
  

  

  

  

enum {
    CODE_boolean = SC_BOOLEAN,
    CODE_int = SC_INT,
    CODE_double = SC_DOUBLE,
    CODE_MAX_SCALAR,
    CODE_ptr = CODE_MAX_SCALAR,
    CODE_MAX
};
enum {
    CODE_sca, CODE_vec
};
