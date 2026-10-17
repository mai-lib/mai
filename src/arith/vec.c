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
#include "global.h"
#include "arith/scalar.h"
#include "arith/vec.h"
#include "arith/dispatch.h"
/*
  
  
  
  
*/
// 
// 
// 
// 

// 

// 

// 
// 
  
    
    
// 
  
  
    
    
  
  
    
    
  

static void vec_add_double_v_double_v(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_add_double_double(vx[i], vy[i]);
}

static void vec_add_double_v_double_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t vy = *(rdouble_t *)y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_add_double_double(vx[i], vy);
}

static void vec_add_double_s_double_v(void *r, void *x, void *y, int len)
{
    rdouble_t vx = *(rdouble_t *)x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_add_double_double(vx, vy[i]);
}

static void vec_add_double_s_double_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    *vr = arith_add_double_double(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_add_double_v_int_v(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_add_double_int(vx[i], vy[i]);
}

static void vec_add_double_v_int_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t vy = *(rint_t *)y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_add_double_int(vx[i], vy);
}

static void vec_add_double_s_int_v(void *r, void *x, void *y, int len)
{
    rdouble_t vx = *(rdouble_t *)x;
    rint_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_add_double_int(vx, vy[i]);
}

static void vec_add_double_s_int_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t *vy = y;
    rdouble_t *vr = r;
    *vr = arith_add_double_int(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  

  

  

  
    
  
    
    
  
  
    
    
  

static void vec_add_int_v_double_v(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_add_int_double(vx[i], vy[i]);
}

static void vec_add_int_v_double_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t vy = *(rdouble_t *)y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_add_int_double(vx[i], vy);
}

static void vec_add_int_s_double_v(void *r, void *x, void *y, int len)
{
    rint_t vx = *(rint_t *)x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_add_int_double(vx, vy[i]);
}

static void vec_add_int_s_double_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    *vr = arith_add_int_double(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_add_int_v_int_v(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t *vy = y;
    rint_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_add_int_int(vx[i], vy[i]);
}

static void vec_add_int_v_int_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t vy = *(rint_t *)y;
    rint_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_add_int_int(vx[i], vy);
}

static void vec_add_int_s_int_v(void *r, void *x, void *y, int len)
{
    rint_t vx = *(rint_t *)x;
    rint_t *vy = y;
    rint_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_add_int_int(vx, vy[i]);
}

static void vec_add_int_s_int_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t *vy = y;
    rint_t *vr = r;
    *vr = arith_add_int_int(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  

  

  

  
    
  

  

  



const binop_fn add_funcs[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR] = {
    
  [CODE_sca][CODE_sca][CODE_double][CODE_double] = vec_add_double_s_double_s,
  [CODE_sca][CODE_vec][CODE_double][CODE_double] = vec_add_double_s_double_v,
  [CODE_vec][CODE_sca][CODE_double][CODE_double] = vec_add_double_v_double_s,
  [CODE_vec][CODE_vec][CODE_double][CODE_double] = vec_add_double_v_double_v,
  
  [CODE_sca][CODE_sca][CODE_double][CODE_int] = vec_add_double_s_int_s,
  [CODE_sca][CODE_vec][CODE_double][CODE_int] = vec_add_double_s_int_v,
  [CODE_vec][CODE_sca][CODE_double][CODE_int] = vec_add_double_v_int_s,
  [CODE_vec][CODE_vec][CODE_double][CODE_int] = vec_add_double_v_int_v,
  
  [CODE_sca][CODE_sca][CODE_int][CODE_double] = vec_add_int_s_double_s,
  [CODE_sca][CODE_vec][CODE_int][CODE_double] = vec_add_int_s_double_v,
  [CODE_vec][CODE_sca][CODE_int][CODE_double] = vec_add_int_v_double_s,
  [CODE_vec][CODE_vec][CODE_int][CODE_double] = vec_add_int_v_double_v,
  
  [CODE_sca][CODE_sca][CODE_int][CODE_int] = vec_add_int_s_int_s,
  [CODE_sca][CODE_vec][CODE_int][CODE_int] = vec_add_int_s_int_v,
  [CODE_vec][CODE_sca][CODE_int][CODE_int] = vec_add_int_v_int_s,
  [CODE_vec][CODE_vec][CODE_int][CODE_int] = vec_add_int_v_int_v,
  
};
// 
// 
    
  
    
    
// 
  
  
    
    
  
  
    
    
  

static void vec_sub_double_v_double_v(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_sub_double_double(vx[i], vy[i]);
}

static void vec_sub_double_v_double_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t vy = *(rdouble_t *)y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_sub_double_double(vx[i], vy);
}

static void vec_sub_double_s_double_v(void *r, void *x, void *y, int len)
{
    rdouble_t vx = *(rdouble_t *)x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_sub_double_double(vx, vy[i]);
}

static void vec_sub_double_s_double_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    *vr = arith_sub_double_double(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_sub_double_v_int_v(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_sub_double_int(vx[i], vy[i]);
}

static void vec_sub_double_v_int_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t vy = *(rint_t *)y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_sub_double_int(vx[i], vy);
}

static void vec_sub_double_s_int_v(void *r, void *x, void *y, int len)
{
    rdouble_t vx = *(rdouble_t *)x;
    rint_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_sub_double_int(vx, vy[i]);
}

static void vec_sub_double_s_int_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t *vy = y;
    rdouble_t *vr = r;
    *vr = arith_sub_double_int(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  

  

  

  
    
  
    
    
  
  
    
    
  

static void vec_sub_int_v_double_v(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_sub_int_double(vx[i], vy[i]);
}

static void vec_sub_int_v_double_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t vy = *(rdouble_t *)y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_sub_int_double(vx[i], vy);
}

static void vec_sub_int_s_double_v(void *r, void *x, void *y, int len)
{
    rint_t vx = *(rint_t *)x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_sub_int_double(vx, vy[i]);
}

static void vec_sub_int_s_double_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    *vr = arith_sub_int_double(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_sub_int_v_int_v(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t *vy = y;
    rint_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_sub_int_int(vx[i], vy[i]);
}

static void vec_sub_int_v_int_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t vy = *(rint_t *)y;
    rint_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_sub_int_int(vx[i], vy);
}

static void vec_sub_int_s_int_v(void *r, void *x, void *y, int len)
{
    rint_t vx = *(rint_t *)x;
    rint_t *vy = y;
    rint_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_sub_int_int(vx, vy[i]);
}

static void vec_sub_int_s_int_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t *vy = y;
    rint_t *vr = r;
    *vr = arith_sub_int_int(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  

  

  

  
    
  

  

  



const binop_fn sub_funcs[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR] = {
    
  [CODE_sca][CODE_sca][CODE_double][CODE_double] = vec_sub_double_s_double_s,
  [CODE_sca][CODE_vec][CODE_double][CODE_double] = vec_sub_double_s_double_v,
  [CODE_vec][CODE_sca][CODE_double][CODE_double] = vec_sub_double_v_double_s,
  [CODE_vec][CODE_vec][CODE_double][CODE_double] = vec_sub_double_v_double_v,
  
  [CODE_sca][CODE_sca][CODE_double][CODE_int] = vec_sub_double_s_int_s,
  [CODE_sca][CODE_vec][CODE_double][CODE_int] = vec_sub_double_s_int_v,
  [CODE_vec][CODE_sca][CODE_double][CODE_int] = vec_sub_double_v_int_s,
  [CODE_vec][CODE_vec][CODE_double][CODE_int] = vec_sub_double_v_int_v,
  
  [CODE_sca][CODE_sca][CODE_int][CODE_double] = vec_sub_int_s_double_s,
  [CODE_sca][CODE_vec][CODE_int][CODE_double] = vec_sub_int_s_double_v,
  [CODE_vec][CODE_sca][CODE_int][CODE_double] = vec_sub_int_v_double_s,
  [CODE_vec][CODE_vec][CODE_int][CODE_double] = vec_sub_int_v_double_v,
  
  [CODE_sca][CODE_sca][CODE_int][CODE_int] = vec_sub_int_s_int_s,
  [CODE_sca][CODE_vec][CODE_int][CODE_int] = vec_sub_int_s_int_v,
  [CODE_vec][CODE_sca][CODE_int][CODE_int] = vec_sub_int_v_int_s,
  [CODE_vec][CODE_vec][CODE_int][CODE_int] = vec_sub_int_v_int_v,
  
};
// 
// 
    
  
    
    
// 
  
  
    
    
  
  
    
    
  

static void vec_mul_double_v_double_v(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_mul_double_double(vx[i], vy[i]);
}

static void vec_mul_double_v_double_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t vy = *(rdouble_t *)y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_mul_double_double(vx[i], vy);
}

static void vec_mul_double_s_double_v(void *r, void *x, void *y, int len)
{
    rdouble_t vx = *(rdouble_t *)x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_mul_double_double(vx, vy[i]);
}

static void vec_mul_double_s_double_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    *vr = arith_mul_double_double(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_mul_double_v_int_v(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_mul_double_int(vx[i], vy[i]);
}

static void vec_mul_double_v_int_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t vy = *(rint_t *)y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_mul_double_int(vx[i], vy);
}

static void vec_mul_double_s_int_v(void *r, void *x, void *y, int len)
{
    rdouble_t vx = *(rdouble_t *)x;
    rint_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_mul_double_int(vx, vy[i]);
}

static void vec_mul_double_s_int_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t *vy = y;
    rdouble_t *vr = r;
    *vr = arith_mul_double_int(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  

  

  

  
    
  
    
    
  
  
    
    
  

static void vec_mul_int_v_double_v(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_mul_int_double(vx[i], vy[i]);
}

static void vec_mul_int_v_double_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t vy = *(rdouble_t *)y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_mul_int_double(vx[i], vy);
}

static void vec_mul_int_s_double_v(void *r, void *x, void *y, int len)
{
    rint_t vx = *(rint_t *)x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_mul_int_double(vx, vy[i]);
}

static void vec_mul_int_s_double_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    *vr = arith_mul_int_double(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_mul_int_v_int_v(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t *vy = y;
    rint_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_mul_int_int(vx[i], vy[i]);
}

static void vec_mul_int_v_int_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t vy = *(rint_t *)y;
    rint_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_mul_int_int(vx[i], vy);
}

static void vec_mul_int_s_int_v(void *r, void *x, void *y, int len)
{
    rint_t vx = *(rint_t *)x;
    rint_t *vy = y;
    rint_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_mul_int_int(vx, vy[i]);
}

static void vec_mul_int_s_int_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t *vy = y;
    rint_t *vr = r;
    *vr = arith_mul_int_int(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  

  

  

  
    
  

  

  



const binop_fn mul_funcs[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR] = {
    
  [CODE_sca][CODE_sca][CODE_double][CODE_double] = vec_mul_double_s_double_s,
  [CODE_sca][CODE_vec][CODE_double][CODE_double] = vec_mul_double_s_double_v,
  [CODE_vec][CODE_sca][CODE_double][CODE_double] = vec_mul_double_v_double_s,
  [CODE_vec][CODE_vec][CODE_double][CODE_double] = vec_mul_double_v_double_v,
  
  [CODE_sca][CODE_sca][CODE_double][CODE_int] = vec_mul_double_s_int_s,
  [CODE_sca][CODE_vec][CODE_double][CODE_int] = vec_mul_double_s_int_v,
  [CODE_vec][CODE_sca][CODE_double][CODE_int] = vec_mul_double_v_int_s,
  [CODE_vec][CODE_vec][CODE_double][CODE_int] = vec_mul_double_v_int_v,
  
  [CODE_sca][CODE_sca][CODE_int][CODE_double] = vec_mul_int_s_double_s,
  [CODE_sca][CODE_vec][CODE_int][CODE_double] = vec_mul_int_s_double_v,
  [CODE_vec][CODE_sca][CODE_int][CODE_double] = vec_mul_int_v_double_s,
  [CODE_vec][CODE_vec][CODE_int][CODE_double] = vec_mul_int_v_double_v,
  
  [CODE_sca][CODE_sca][CODE_int][CODE_int] = vec_mul_int_s_int_s,
  [CODE_sca][CODE_vec][CODE_int][CODE_int] = vec_mul_int_s_int_v,
  [CODE_vec][CODE_sca][CODE_int][CODE_int] = vec_mul_int_v_int_s,
  [CODE_vec][CODE_vec][CODE_int][CODE_int] = vec_mul_int_v_int_v,
  
};
// 
// 
    
  
    
    
// 
  
  
    
    
  
  
    
    
  

static void vec_div_double_v_double_v(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_div_double_double(vx[i], vy[i]);
}

static void vec_div_double_v_double_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t vy = *(rdouble_t *)y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_div_double_double(vx[i], vy);
}

static void vec_div_double_s_double_v(void *r, void *x, void *y, int len)
{
    rdouble_t vx = *(rdouble_t *)x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_div_double_double(vx, vy[i]);
}

static void vec_div_double_s_double_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    *vr = arith_div_double_double(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_div_double_v_int_v(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_div_double_int(vx[i], vy[i]);
}

static void vec_div_double_v_int_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t vy = *(rint_t *)y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_div_double_int(vx[i], vy);
}

static void vec_div_double_s_int_v(void *r, void *x, void *y, int len)
{
    rdouble_t vx = *(rdouble_t *)x;
    rint_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_div_double_int(vx, vy[i]);
}

static void vec_div_double_s_int_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t *vy = y;
    rdouble_t *vr = r;
    *vr = arith_div_double_int(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  

  

  

  
    
  
    
    
  
  
    
    
  

static void vec_div_int_v_double_v(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_div_int_double(vx[i], vy[i]);
}

static void vec_div_int_v_double_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t vy = *(rdouble_t *)y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_div_int_double(vx[i], vy);
}

static void vec_div_int_s_double_v(void *r, void *x, void *y, int len)
{
    rint_t vx = *(rint_t *)x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_div_int_double(vx, vy[i]);
}

static void vec_div_int_s_double_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    *vr = arith_div_int_double(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_div_int_v_int_v(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t *vy = y;
    rint_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_div_int_int(vx[i], vy[i]);
}

static void vec_div_int_v_int_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t vy = *(rint_t *)y;
    rint_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_div_int_int(vx[i], vy);
}

static void vec_div_int_s_int_v(void *r, void *x, void *y, int len)
{
    rint_t vx = *(rint_t *)x;
    rint_t *vy = y;
    rint_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_div_int_int(vx, vy[i]);
}

static void vec_div_int_s_int_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t *vy = y;
    rint_t *vr = r;
    *vr = arith_div_int_int(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  

  

  

  
    
  

  

  



const binop_fn div_funcs[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR] = {
    
  [CODE_sca][CODE_sca][CODE_double][CODE_double] = vec_div_double_s_double_s,
  [CODE_sca][CODE_vec][CODE_double][CODE_double] = vec_div_double_s_double_v,
  [CODE_vec][CODE_sca][CODE_double][CODE_double] = vec_div_double_v_double_s,
  [CODE_vec][CODE_vec][CODE_double][CODE_double] = vec_div_double_v_double_v,
  
  [CODE_sca][CODE_sca][CODE_double][CODE_int] = vec_div_double_s_int_s,
  [CODE_sca][CODE_vec][CODE_double][CODE_int] = vec_div_double_s_int_v,
  [CODE_vec][CODE_sca][CODE_double][CODE_int] = vec_div_double_v_int_s,
  [CODE_vec][CODE_vec][CODE_double][CODE_int] = vec_div_double_v_int_v,
  
  [CODE_sca][CODE_sca][CODE_int][CODE_double] = vec_div_int_s_double_s,
  [CODE_sca][CODE_vec][CODE_int][CODE_double] = vec_div_int_s_double_v,
  [CODE_vec][CODE_sca][CODE_int][CODE_double] = vec_div_int_v_double_s,
  [CODE_vec][CODE_vec][CODE_int][CODE_double] = vec_div_int_v_double_v,
  
  [CODE_sca][CODE_sca][CODE_int][CODE_int] = vec_div_int_s_int_s,
  [CODE_sca][CODE_vec][CODE_int][CODE_int] = vec_div_int_s_int_v,
  [CODE_vec][CODE_sca][CODE_int][CODE_int] = vec_div_int_v_int_s,
  [CODE_vec][CODE_vec][CODE_int][CODE_int] = vec_div_int_v_int_v,
  
};
// 
// 
    
  
    
    
// 
  
  
    
    
  
  
    
    
  

static void vec_pow_double_v_double_v(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_pow_double_double(vx[i], vy[i]);
}

static void vec_pow_double_v_double_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t vy = *(rdouble_t *)y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_pow_double_double(vx[i], vy);
}

static void vec_pow_double_s_double_v(void *r, void *x, void *y, int len)
{
    rdouble_t vx = *(rdouble_t *)x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_pow_double_double(vx, vy[i]);
}

static void vec_pow_double_s_double_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    *vr = arith_pow_double_double(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_pow_double_v_int_v(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_pow_double_int(vx[i], vy[i]);
}

static void vec_pow_double_v_int_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t vy = *(rint_t *)y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_pow_double_int(vx[i], vy);
}

static void vec_pow_double_s_int_v(void *r, void *x, void *y, int len)
{
    rdouble_t vx = *(rdouble_t *)x;
    rint_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_pow_double_int(vx, vy[i]);
}

static void vec_pow_double_s_int_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t *vy = y;
    rdouble_t *vr = r;
    *vr = arith_pow_double_int(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  

  

  

  
    
  
    
    
  
  
    
    
  

static void vec_pow_int_v_double_v(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_pow_int_double(vx[i], vy[i]);
}

static void vec_pow_int_v_double_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t vy = *(rdouble_t *)y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_pow_int_double(vx[i], vy);
}

static void vec_pow_int_s_double_v(void *r, void *x, void *y, int len)
{
    rint_t vx = *(rint_t *)x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_pow_int_double(vx, vy[i]);
}

static void vec_pow_int_s_double_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    *vr = arith_pow_int_double(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_pow_int_v_int_v(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t *vy = y;
    rint_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_pow_int_int(vx[i], vy[i]);
}

static void vec_pow_int_v_int_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t vy = *(rint_t *)y;
    rint_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_pow_int_int(vx[i], vy);
}

static void vec_pow_int_s_int_v(void *r, void *x, void *y, int len)
{
    rint_t vx = *(rint_t *)x;
    rint_t *vy = y;
    rint_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_pow_int_int(vx, vy[i]);
}

static void vec_pow_int_s_int_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t *vy = y;
    rint_t *vr = r;
    *vr = arith_pow_int_int(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  

  

  

  
    
  

  

  



const binop_fn pow_funcs[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR] = {
    
  [CODE_sca][CODE_sca][CODE_double][CODE_double] = vec_pow_double_s_double_s,
  [CODE_sca][CODE_vec][CODE_double][CODE_double] = vec_pow_double_s_double_v,
  [CODE_vec][CODE_sca][CODE_double][CODE_double] = vec_pow_double_v_double_s,
  [CODE_vec][CODE_vec][CODE_double][CODE_double] = vec_pow_double_v_double_v,
  
  [CODE_sca][CODE_sca][CODE_double][CODE_int] = vec_pow_double_s_int_s,
  [CODE_sca][CODE_vec][CODE_double][CODE_int] = vec_pow_double_s_int_v,
  [CODE_vec][CODE_sca][CODE_double][CODE_int] = vec_pow_double_v_int_s,
  [CODE_vec][CODE_vec][CODE_double][CODE_int] = vec_pow_double_v_int_v,
  
  [CODE_sca][CODE_sca][CODE_int][CODE_double] = vec_pow_int_s_double_s,
  [CODE_sca][CODE_vec][CODE_int][CODE_double] = vec_pow_int_s_double_v,
  [CODE_vec][CODE_sca][CODE_int][CODE_double] = vec_pow_int_v_double_s,
  [CODE_vec][CODE_vec][CODE_int][CODE_double] = vec_pow_int_v_double_v,
  
  [CODE_sca][CODE_sca][CODE_int][CODE_int] = vec_pow_int_s_int_s,
  [CODE_sca][CODE_vec][CODE_int][CODE_int] = vec_pow_int_s_int_v,
  [CODE_vec][CODE_sca][CODE_int][CODE_int] = vec_pow_int_v_int_s,
  [CODE_vec][CODE_vec][CODE_int][CODE_int] = vec_pow_int_v_int_v,
  
};
// 
// 
    
  

  

  

  

  

  


// 
  
    
    
// 
  
    
    

static void vec_neg_double_v(void *r, void *x, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neg_double(vx[i]);
}

static void vec_neg_double_s(void *r, void *x, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vr = r;
    *vr = arith_neg_double(*vx);
}

/*
  
*/
// 
// 
    
  
    
    

static void vec_neg_int_v(void *r, void *x, int len)
{
    rint_t *vx = x;
    rint_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neg_int(vx[i]);
}

static void vec_neg_int_s(void *r, void *x, int len)
{
    rint_t *vx = x;
    rint_t *vr = r;
    *vr = arith_neg_int(*vx);
}

/*
  
*/
// 
// 
    
  

  

  


const unop_fn neg_funcs[2][CODE_MAX_SCALAR] = {
    
  [CODE_sca][CODE_double] = vec_neg_double_s,
  [CODE_vec][CODE_double] = vec_neg_double_v,
  
  [CODE_sca][CODE_int] = vec_neg_int_s,
  [CODE_vec][CODE_int] = vec_neg_int_v,
  
};
// 
// 
    
  

  


// 
  
    
    
// 
  
    
    

static void vec_reduce_add_double(void *r, void *x, int len)
{
    rdouble_t vr = 0;
    rdouble_t *vx = x;
    for(int i=0; i < len; i++)
        vr = arith_add_double_double(vr, vx[i]);
    *(rdouble_t *)r = vr;
}

/*
  
*/
// 
// 
    
  
    
    

static void vec_reduce_add_int(void *r, void *x, int len)
{
    rint_t vr = 0;
    rint_t *vx = x;
    for(int i=0; i < len; i++)
        vr = arith_add_int_int(vr, vx[i]);
    *(rint_t *)r = vr;
}

/*
  
*/
// 
// 
    
  

  

  


const unop_fn reduce_add_funcs[CODE_MAX_SCALAR] = {
    
  [CODE_double] = vec_reduce_add_double,
  
  [CODE_int] = vec_reduce_add_int,
  
};
// 
// 
    
  
    
    
// 
  
    
    

static void vec_reduce_mul_double(void *r, void *x, int len)
{
    rdouble_t vr = 1;
    rdouble_t *vx = x;
    for(int i=0; i < len; i++)
        vr = arith_mul_double_double(vr, vx[i]);
    *(rdouble_t *)r = vr;
}

/*
  
*/
// 
// 
    
  
    
    

static void vec_reduce_mul_int(void *r, void *x, int len)
{
    rint_t vr = 1;
    rint_t *vx = x;
    for(int i=0; i < len; i++)
        vr = arith_mul_int_int(vr, vx[i]);
    *(rint_t *)r = vr;
}

/*
  
*/
// 
// 
    
  

  

  


const unop_fn reduce_mul_funcs[CODE_MAX_SCALAR] = {
    
  [CODE_double] = vec_reduce_mul_double,
  
  [CODE_int] = vec_reduce_mul_int,
  
};
// 
// 
    
  

  

  

// 

// 
  
    
    
// 
  
  
    
    
  
  
    
    
  

static void vec_lth_double_v_double_v(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_lth_double_double(vx[i], vy[i]);
}

static void vec_lth_double_v_double_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t vy = *(rdouble_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_lth_double_double(vx[i], vy);
}

static void vec_lth_double_s_double_v(void *r, void *x, void *y, int len)
{
    rdouble_t vx = *(rdouble_t *)x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_lth_double_double(vx, vy[i]);
}

static void vec_lth_double_s_double_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_lth_double_double(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_lth_double_v_int_v(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_lth_double_int(vx[i], vy[i]);
}

static void vec_lth_double_v_int_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t vy = *(rint_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_lth_double_int(vx[i], vy);
}

static void vec_lth_double_s_int_v(void *r, void *x, void *y, int len)
{
    rdouble_t vx = *(rdouble_t *)x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_lth_double_int(vx, vy[i]);
}

static void vec_lth_double_s_int_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_lth_double_int(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  

  

  

  
    
  
    
    
  
  
    
    
  

static void vec_lth_int_v_double_v(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_lth_int_double(vx[i], vy[i]);
}

static void vec_lth_int_v_double_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t vy = *(rdouble_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_lth_int_double(vx[i], vy);
}

static void vec_lth_int_s_double_v(void *r, void *x, void *y, int len)
{
    rint_t vx = *(rint_t *)x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_lth_int_double(vx, vy[i]);
}

static void vec_lth_int_s_double_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_lth_int_double(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_lth_int_v_int_v(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_lth_int_int(vx[i], vy[i]);
}

static void vec_lth_int_v_int_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t vy = *(rint_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_lth_int_int(vx[i], vy);
}

static void vec_lth_int_s_int_v(void *r, void *x, void *y, int len)
{
    rint_t vx = *(rint_t *)x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_lth_int_int(vx, vy[i]);
}

static void vec_lth_int_s_int_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_lth_int_int(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  

  

  

  
    
  

  

  



const binop_fn lth_funcs[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR] = {
    
  [CODE_sca][CODE_sca][CODE_double][CODE_double] = vec_lth_double_s_double_s,
  [CODE_sca][CODE_vec][CODE_double][CODE_double] = vec_lth_double_s_double_v,
  [CODE_vec][CODE_sca][CODE_double][CODE_double] = vec_lth_double_v_double_s,
  [CODE_vec][CODE_vec][CODE_double][CODE_double] = vec_lth_double_v_double_v,
  
  [CODE_sca][CODE_sca][CODE_double][CODE_int] = vec_lth_double_s_int_s,
  [CODE_sca][CODE_vec][CODE_double][CODE_int] = vec_lth_double_s_int_v,
  [CODE_vec][CODE_sca][CODE_double][CODE_int] = vec_lth_double_v_int_s,
  [CODE_vec][CODE_vec][CODE_double][CODE_int] = vec_lth_double_v_int_v,
  
  [CODE_sca][CODE_sca][CODE_int][CODE_double] = vec_lth_int_s_double_s,
  [CODE_sca][CODE_vec][CODE_int][CODE_double] = vec_lth_int_s_double_v,
  [CODE_vec][CODE_sca][CODE_int][CODE_double] = vec_lth_int_v_double_s,
  [CODE_vec][CODE_vec][CODE_int][CODE_double] = vec_lth_int_v_double_v,
  
  [CODE_sca][CODE_sca][CODE_int][CODE_int] = vec_lth_int_s_int_s,
  [CODE_sca][CODE_vec][CODE_int][CODE_int] = vec_lth_int_s_int_v,
  [CODE_vec][CODE_sca][CODE_int][CODE_int] = vec_lth_int_v_int_s,
  [CODE_vec][CODE_vec][CODE_int][CODE_int] = vec_lth_int_v_int_v,
  
};
// 
// 
    
  
    
    
// 
  
  
    
    
  
  
    
    
  

static void vec_lte_double_v_double_v(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_lte_double_double(vx[i], vy[i]);
}

static void vec_lte_double_v_double_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t vy = *(rdouble_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_lte_double_double(vx[i], vy);
}

static void vec_lte_double_s_double_v(void *r, void *x, void *y, int len)
{
    rdouble_t vx = *(rdouble_t *)x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_lte_double_double(vx, vy[i]);
}

static void vec_lte_double_s_double_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_lte_double_double(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_lte_double_v_int_v(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_lte_double_int(vx[i], vy[i]);
}

static void vec_lte_double_v_int_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t vy = *(rint_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_lte_double_int(vx[i], vy);
}

static void vec_lte_double_s_int_v(void *r, void *x, void *y, int len)
{
    rdouble_t vx = *(rdouble_t *)x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_lte_double_int(vx, vy[i]);
}

static void vec_lte_double_s_int_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_lte_double_int(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  

  

  

  
    
  
    
    
  
  
    
    
  

static void vec_lte_int_v_double_v(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_lte_int_double(vx[i], vy[i]);
}

static void vec_lte_int_v_double_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t vy = *(rdouble_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_lte_int_double(vx[i], vy);
}

static void vec_lte_int_s_double_v(void *r, void *x, void *y, int len)
{
    rint_t vx = *(rint_t *)x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_lte_int_double(vx, vy[i]);
}

static void vec_lte_int_s_double_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_lte_int_double(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_lte_int_v_int_v(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_lte_int_int(vx[i], vy[i]);
}

static void vec_lte_int_v_int_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t vy = *(rint_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_lte_int_int(vx[i], vy);
}

static void vec_lte_int_s_int_v(void *r, void *x, void *y, int len)
{
    rint_t vx = *(rint_t *)x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_lte_int_int(vx, vy[i]);
}

static void vec_lte_int_s_int_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_lte_int_int(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  

  

  

  
    
  

  

  



const binop_fn lte_funcs[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR] = {
    
  [CODE_sca][CODE_sca][CODE_double][CODE_double] = vec_lte_double_s_double_s,
  [CODE_sca][CODE_vec][CODE_double][CODE_double] = vec_lte_double_s_double_v,
  [CODE_vec][CODE_sca][CODE_double][CODE_double] = vec_lte_double_v_double_s,
  [CODE_vec][CODE_vec][CODE_double][CODE_double] = vec_lte_double_v_double_v,
  
  [CODE_sca][CODE_sca][CODE_double][CODE_int] = vec_lte_double_s_int_s,
  [CODE_sca][CODE_vec][CODE_double][CODE_int] = vec_lte_double_s_int_v,
  [CODE_vec][CODE_sca][CODE_double][CODE_int] = vec_lte_double_v_int_s,
  [CODE_vec][CODE_vec][CODE_double][CODE_int] = vec_lte_double_v_int_v,
  
  [CODE_sca][CODE_sca][CODE_int][CODE_double] = vec_lte_int_s_double_s,
  [CODE_sca][CODE_vec][CODE_int][CODE_double] = vec_lte_int_s_double_v,
  [CODE_vec][CODE_sca][CODE_int][CODE_double] = vec_lte_int_v_double_s,
  [CODE_vec][CODE_vec][CODE_int][CODE_double] = vec_lte_int_v_double_v,
  
  [CODE_sca][CODE_sca][CODE_int][CODE_int] = vec_lte_int_s_int_s,
  [CODE_sca][CODE_vec][CODE_int][CODE_int] = vec_lte_int_s_int_v,
  [CODE_vec][CODE_sca][CODE_int][CODE_int] = vec_lte_int_v_int_s,
  [CODE_vec][CODE_vec][CODE_int][CODE_int] = vec_lte_int_v_int_v,
  
};
// 
// 
    
  
    
    
// 
  
  
    
    
  
  
    
    
  

static void vec_gth_double_v_double_v(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_gth_double_double(vx[i], vy[i]);
}

static void vec_gth_double_v_double_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t vy = *(rdouble_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_gth_double_double(vx[i], vy);
}

static void vec_gth_double_s_double_v(void *r, void *x, void *y, int len)
{
    rdouble_t vx = *(rdouble_t *)x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_gth_double_double(vx, vy[i]);
}

static void vec_gth_double_s_double_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_gth_double_double(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_gth_double_v_int_v(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_gth_double_int(vx[i], vy[i]);
}

static void vec_gth_double_v_int_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t vy = *(rint_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_gth_double_int(vx[i], vy);
}

static void vec_gth_double_s_int_v(void *r, void *x, void *y, int len)
{
    rdouble_t vx = *(rdouble_t *)x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_gth_double_int(vx, vy[i]);
}

static void vec_gth_double_s_int_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_gth_double_int(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  

  

  

  
    
  
    
    
  
  
    
    
  

static void vec_gth_int_v_double_v(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_gth_int_double(vx[i], vy[i]);
}

static void vec_gth_int_v_double_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t vy = *(rdouble_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_gth_int_double(vx[i], vy);
}

static void vec_gth_int_s_double_v(void *r, void *x, void *y, int len)
{
    rint_t vx = *(rint_t *)x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_gth_int_double(vx, vy[i]);
}

static void vec_gth_int_s_double_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_gth_int_double(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_gth_int_v_int_v(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_gth_int_int(vx[i], vy[i]);
}

static void vec_gth_int_v_int_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t vy = *(rint_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_gth_int_int(vx[i], vy);
}

static void vec_gth_int_s_int_v(void *r, void *x, void *y, int len)
{
    rint_t vx = *(rint_t *)x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_gth_int_int(vx, vy[i]);
}

static void vec_gth_int_s_int_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_gth_int_int(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  

  

  

  
    
  

  

  



const binop_fn gth_funcs[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR] = {
    
  [CODE_sca][CODE_sca][CODE_double][CODE_double] = vec_gth_double_s_double_s,
  [CODE_sca][CODE_vec][CODE_double][CODE_double] = vec_gth_double_s_double_v,
  [CODE_vec][CODE_sca][CODE_double][CODE_double] = vec_gth_double_v_double_s,
  [CODE_vec][CODE_vec][CODE_double][CODE_double] = vec_gth_double_v_double_v,
  
  [CODE_sca][CODE_sca][CODE_double][CODE_int] = vec_gth_double_s_int_s,
  [CODE_sca][CODE_vec][CODE_double][CODE_int] = vec_gth_double_s_int_v,
  [CODE_vec][CODE_sca][CODE_double][CODE_int] = vec_gth_double_v_int_s,
  [CODE_vec][CODE_vec][CODE_double][CODE_int] = vec_gth_double_v_int_v,
  
  [CODE_sca][CODE_sca][CODE_int][CODE_double] = vec_gth_int_s_double_s,
  [CODE_sca][CODE_vec][CODE_int][CODE_double] = vec_gth_int_s_double_v,
  [CODE_vec][CODE_sca][CODE_int][CODE_double] = vec_gth_int_v_double_s,
  [CODE_vec][CODE_vec][CODE_int][CODE_double] = vec_gth_int_v_double_v,
  
  [CODE_sca][CODE_sca][CODE_int][CODE_int] = vec_gth_int_s_int_s,
  [CODE_sca][CODE_vec][CODE_int][CODE_int] = vec_gth_int_s_int_v,
  [CODE_vec][CODE_sca][CODE_int][CODE_int] = vec_gth_int_v_int_s,
  [CODE_vec][CODE_vec][CODE_int][CODE_int] = vec_gth_int_v_int_v,
  
};
// 
// 
    
  
    
    
// 
  
  
    
    
  
  
    
    
  

static void vec_gte_double_v_double_v(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_gte_double_double(vx[i], vy[i]);
}

static void vec_gte_double_v_double_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t vy = *(rdouble_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_gte_double_double(vx[i], vy);
}

static void vec_gte_double_s_double_v(void *r, void *x, void *y, int len)
{
    rdouble_t vx = *(rdouble_t *)x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_gte_double_double(vx, vy[i]);
}

static void vec_gte_double_s_double_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_gte_double_double(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_gte_double_v_int_v(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_gte_double_int(vx[i], vy[i]);
}

static void vec_gte_double_v_int_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t vy = *(rint_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_gte_double_int(vx[i], vy);
}

static void vec_gte_double_s_int_v(void *r, void *x, void *y, int len)
{
    rdouble_t vx = *(rdouble_t *)x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_gte_double_int(vx, vy[i]);
}

static void vec_gte_double_s_int_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_gte_double_int(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  

  

  

  
    
  
    
    
  
  
    
    
  

static void vec_gte_int_v_double_v(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_gte_int_double(vx[i], vy[i]);
}

static void vec_gte_int_v_double_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t vy = *(rdouble_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_gte_int_double(vx[i], vy);
}

static void vec_gte_int_s_double_v(void *r, void *x, void *y, int len)
{
    rint_t vx = *(rint_t *)x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_gte_int_double(vx, vy[i]);
}

static void vec_gte_int_s_double_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_gte_int_double(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_gte_int_v_int_v(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_gte_int_int(vx[i], vy[i]);
}

static void vec_gte_int_v_int_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t vy = *(rint_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_gte_int_int(vx[i], vy);
}

static void vec_gte_int_s_int_v(void *r, void *x, void *y, int len)
{
    rint_t vx = *(rint_t *)x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_gte_int_int(vx, vy[i]);
}

static void vec_gte_int_s_int_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_gte_int_int(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  

  

  

  
    
  

  

  



const binop_fn gte_funcs[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR] = {
    
  [CODE_sca][CODE_sca][CODE_double][CODE_double] = vec_gte_double_s_double_s,
  [CODE_sca][CODE_vec][CODE_double][CODE_double] = vec_gte_double_s_double_v,
  [CODE_vec][CODE_sca][CODE_double][CODE_double] = vec_gte_double_v_double_s,
  [CODE_vec][CODE_vec][CODE_double][CODE_double] = vec_gte_double_v_double_v,
  
  [CODE_sca][CODE_sca][CODE_double][CODE_int] = vec_gte_double_s_int_s,
  [CODE_sca][CODE_vec][CODE_double][CODE_int] = vec_gte_double_s_int_v,
  [CODE_vec][CODE_sca][CODE_double][CODE_int] = vec_gte_double_v_int_s,
  [CODE_vec][CODE_vec][CODE_double][CODE_int] = vec_gte_double_v_int_v,
  
  [CODE_sca][CODE_sca][CODE_int][CODE_double] = vec_gte_int_s_double_s,
  [CODE_sca][CODE_vec][CODE_int][CODE_double] = vec_gte_int_s_double_v,
  [CODE_vec][CODE_sca][CODE_int][CODE_double] = vec_gte_int_v_double_s,
  [CODE_vec][CODE_vec][CODE_int][CODE_double] = vec_gte_int_v_double_v,
  
  [CODE_sca][CODE_sca][CODE_int][CODE_int] = vec_gte_int_s_int_s,
  [CODE_sca][CODE_vec][CODE_int][CODE_int] = vec_gte_int_s_int_v,
  [CODE_vec][CODE_sca][CODE_int][CODE_int] = vec_gte_int_v_int_s,
  [CODE_vec][CODE_vec][CODE_int][CODE_int] = vec_gte_int_v_int_v,
  
};
// 
// 
    
  

  

  

  

  

// 
  
    
    
// 
  
  
    
    
  
  
    
    
  

static void vec_eql_double_v_double_v(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_eql_double_double(vx[i], vy[i]);
}

static void vec_eql_double_v_double_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t vy = *(rdouble_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_eql_double_double(vx[i], vy);
}

static void vec_eql_double_s_double_v(void *r, void *x, void *y, int len)
{
    rdouble_t vx = *(rdouble_t *)x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_eql_double_double(vx, vy[i]);
}

static void vec_eql_double_s_double_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_eql_double_double(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_eql_double_v_int_v(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_eql_double_int(vx[i], vy[i]);
}

static void vec_eql_double_v_int_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t vy = *(rint_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_eql_double_int(vx[i], vy);
}

static void vec_eql_double_s_int_v(void *r, void *x, void *y, int len)
{
    rdouble_t vx = *(rdouble_t *)x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_eql_double_int(vx, vy[i]);
}

static void vec_eql_double_s_int_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_eql_double_int(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_eql_double_v_boolean_v(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rboolean_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_eql_double_boolean(vx[i], vy[i]);
}

static void vec_eql_double_v_boolean_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rboolean_t vy = *(rboolean_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_eql_double_boolean(vx[i], vy);
}

static void vec_eql_double_s_boolean_v(void *r, void *x, void *y, int len)
{
    rdouble_t vx = *(rdouble_t *)x;
    rboolean_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_eql_double_boolean(vx, vy[i]);
}

static void vec_eql_double_s_boolean_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rboolean_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_eql_double_boolean(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  

  

  

  

  
    
  
    
    
  
  
    
    
  

static void vec_eql_int_v_double_v(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_eql_int_double(vx[i], vy[i]);
}

static void vec_eql_int_v_double_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t vy = *(rdouble_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_eql_int_double(vx[i], vy);
}

static void vec_eql_int_s_double_v(void *r, void *x, void *y, int len)
{
    rint_t vx = *(rint_t *)x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_eql_int_double(vx, vy[i]);
}

static void vec_eql_int_s_double_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_eql_int_double(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_eql_int_v_int_v(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_eql_int_int(vx[i], vy[i]);
}

static void vec_eql_int_v_int_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t vy = *(rint_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_eql_int_int(vx[i], vy);
}

static void vec_eql_int_s_int_v(void *r, void *x, void *y, int len)
{
    rint_t vx = *(rint_t *)x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_eql_int_int(vx, vy[i]);
}

static void vec_eql_int_s_int_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_eql_int_int(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_eql_int_v_boolean_v(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rboolean_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_eql_int_boolean(vx[i], vy[i]);
}

static void vec_eql_int_v_boolean_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rboolean_t vy = *(rboolean_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_eql_int_boolean(vx[i], vy);
}

static void vec_eql_int_s_boolean_v(void *r, void *x, void *y, int len)
{
    rint_t vx = *(rint_t *)x;
    rboolean_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_eql_int_boolean(vx, vy[i]);
}

static void vec_eql_int_s_boolean_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rboolean_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_eql_int_boolean(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  

  

  

  

  
    
  
    
    
  
  
    
    
  

static void vec_eql_boolean_v_double_v(void *r, void *x, void *y, int len)
{
    rboolean_t *vx = x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_eql_boolean_double(vx[i], vy[i]);
}

static void vec_eql_boolean_v_double_s(void *r, void *x, void *y, int len)
{
    rboolean_t *vx = x;
    rdouble_t vy = *(rdouble_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_eql_boolean_double(vx[i], vy);
}

static void vec_eql_boolean_s_double_v(void *r, void *x, void *y, int len)
{
    rboolean_t vx = *(rboolean_t *)x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_eql_boolean_double(vx, vy[i]);
}

static void vec_eql_boolean_s_double_s(void *r, void *x, void *y, int len)
{
    rboolean_t *vx = x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_eql_boolean_double(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_eql_boolean_v_int_v(void *r, void *x, void *y, int len)
{
    rboolean_t *vx = x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_eql_boolean_int(vx[i], vy[i]);
}

static void vec_eql_boolean_v_int_s(void *r, void *x, void *y, int len)
{
    rboolean_t *vx = x;
    rint_t vy = *(rint_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_eql_boolean_int(vx[i], vy);
}

static void vec_eql_boolean_s_int_v(void *r, void *x, void *y, int len)
{
    rboolean_t vx = *(rboolean_t *)x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_eql_boolean_int(vx, vy[i]);
}

static void vec_eql_boolean_s_int_s(void *r, void *x, void *y, int len)
{
    rboolean_t *vx = x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_eql_boolean_int(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_eql_boolean_v_boolean_v(void *r, void *x, void *y, int len)
{
    rboolean_t *vx = x;
    rboolean_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_eql_boolean_boolean(vx[i], vy[i]);
}

static void vec_eql_boolean_v_boolean_s(void *r, void *x, void *y, int len)
{
    rboolean_t *vx = x;
    rboolean_t vy = *(rboolean_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_eql_boolean_boolean(vx[i], vy);
}

static void vec_eql_boolean_s_boolean_v(void *r, void *x, void *y, int len)
{
    rboolean_t vx = *(rboolean_t *)x;
    rboolean_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_eql_boolean_boolean(vx, vy[i]);
}

static void vec_eql_boolean_s_boolean_s(void *r, void *x, void *y, int len)
{
    rboolean_t *vx = x;
    rboolean_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_eql_boolean_boolean(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  

  

  

  

  
    
  

  

  

  



const binop_fn eql_funcs[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR] = {
    
  [CODE_sca][CODE_sca][CODE_double][CODE_double] = vec_eql_double_s_double_s,
  [CODE_sca][CODE_vec][CODE_double][CODE_double] = vec_eql_double_s_double_v,
  [CODE_vec][CODE_sca][CODE_double][CODE_double] = vec_eql_double_v_double_s,
  [CODE_vec][CODE_vec][CODE_double][CODE_double] = vec_eql_double_v_double_v,
  
  [CODE_sca][CODE_sca][CODE_double][CODE_int] = vec_eql_double_s_int_s,
  [CODE_sca][CODE_vec][CODE_double][CODE_int] = vec_eql_double_s_int_v,
  [CODE_vec][CODE_sca][CODE_double][CODE_int] = vec_eql_double_v_int_s,
  [CODE_vec][CODE_vec][CODE_double][CODE_int] = vec_eql_double_v_int_v,
  
  [CODE_sca][CODE_sca][CODE_double][CODE_boolean] = vec_eql_double_s_boolean_s,
  [CODE_sca][CODE_vec][CODE_double][CODE_boolean] = vec_eql_double_s_boolean_v,
  [CODE_vec][CODE_sca][CODE_double][CODE_boolean] = vec_eql_double_v_boolean_s,
  [CODE_vec][CODE_vec][CODE_double][CODE_boolean] = vec_eql_double_v_boolean_v,
  
  [CODE_sca][CODE_sca][CODE_int][CODE_double] = vec_eql_int_s_double_s,
  [CODE_sca][CODE_vec][CODE_int][CODE_double] = vec_eql_int_s_double_v,
  [CODE_vec][CODE_sca][CODE_int][CODE_double] = vec_eql_int_v_double_s,
  [CODE_vec][CODE_vec][CODE_int][CODE_double] = vec_eql_int_v_double_v,
  
  [CODE_sca][CODE_sca][CODE_int][CODE_int] = vec_eql_int_s_int_s,
  [CODE_sca][CODE_vec][CODE_int][CODE_int] = vec_eql_int_s_int_v,
  [CODE_vec][CODE_sca][CODE_int][CODE_int] = vec_eql_int_v_int_s,
  [CODE_vec][CODE_vec][CODE_int][CODE_int] = vec_eql_int_v_int_v,
  
  [CODE_sca][CODE_sca][CODE_int][CODE_boolean] = vec_eql_int_s_boolean_s,
  [CODE_sca][CODE_vec][CODE_int][CODE_boolean] = vec_eql_int_s_boolean_v,
  [CODE_vec][CODE_sca][CODE_int][CODE_boolean] = vec_eql_int_v_boolean_s,
  [CODE_vec][CODE_vec][CODE_int][CODE_boolean] = vec_eql_int_v_boolean_v,
  
  [CODE_sca][CODE_sca][CODE_boolean][CODE_double] = vec_eql_boolean_s_double_s,
  [CODE_sca][CODE_vec][CODE_boolean][CODE_double] = vec_eql_boolean_s_double_v,
  [CODE_vec][CODE_sca][CODE_boolean][CODE_double] = vec_eql_boolean_v_double_s,
  [CODE_vec][CODE_vec][CODE_boolean][CODE_double] = vec_eql_boolean_v_double_v,
  
  [CODE_sca][CODE_sca][CODE_boolean][CODE_int] = vec_eql_boolean_s_int_s,
  [CODE_sca][CODE_vec][CODE_boolean][CODE_int] = vec_eql_boolean_s_int_v,
  [CODE_vec][CODE_sca][CODE_boolean][CODE_int] = vec_eql_boolean_v_int_s,
  [CODE_vec][CODE_vec][CODE_boolean][CODE_int] = vec_eql_boolean_v_int_v,
  
  [CODE_sca][CODE_sca][CODE_boolean][CODE_boolean] = vec_eql_boolean_s_boolean_s,
  [CODE_sca][CODE_vec][CODE_boolean][CODE_boolean] = vec_eql_boolean_s_boolean_v,
  [CODE_vec][CODE_sca][CODE_boolean][CODE_boolean] = vec_eql_boolean_v_boolean_s,
  [CODE_vec][CODE_vec][CODE_boolean][CODE_boolean] = vec_eql_boolean_v_boolean_v,
  
};
// 
// 
    
  
    
    
// 
  
  
    
    
  
  
    
    
  

static void vec_neq_double_v_double_v(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neq_double_double(vx[i], vy[i]);
}

static void vec_neq_double_v_double_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t vy = *(rdouble_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neq_double_double(vx[i], vy);
}

static void vec_neq_double_s_double_v(void *r, void *x, void *y, int len)
{
    rdouble_t vx = *(rdouble_t *)x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neq_double_double(vx, vy[i]);
}

static void vec_neq_double_s_double_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_neq_double_double(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_neq_double_v_int_v(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neq_double_int(vx[i], vy[i]);
}

static void vec_neq_double_v_int_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t vy = *(rint_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neq_double_int(vx[i], vy);
}

static void vec_neq_double_s_int_v(void *r, void *x, void *y, int len)
{
    rdouble_t vx = *(rdouble_t *)x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neq_double_int(vx, vy[i]);
}

static void vec_neq_double_s_int_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_neq_double_int(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_neq_double_v_boolean_v(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rboolean_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neq_double_boolean(vx[i], vy[i]);
}

static void vec_neq_double_v_boolean_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rboolean_t vy = *(rboolean_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neq_double_boolean(vx[i], vy);
}

static void vec_neq_double_s_boolean_v(void *r, void *x, void *y, int len)
{
    rdouble_t vx = *(rdouble_t *)x;
    rboolean_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neq_double_boolean(vx, vy[i]);
}

static void vec_neq_double_s_boolean_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rboolean_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_neq_double_boolean(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  

  

  

  

  
    
  
    
    
  
  
    
    
  

static void vec_neq_int_v_double_v(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neq_int_double(vx[i], vy[i]);
}

static void vec_neq_int_v_double_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t vy = *(rdouble_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neq_int_double(vx[i], vy);
}

static void vec_neq_int_s_double_v(void *r, void *x, void *y, int len)
{
    rint_t vx = *(rint_t *)x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neq_int_double(vx, vy[i]);
}

static void vec_neq_int_s_double_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_neq_int_double(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_neq_int_v_int_v(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neq_int_int(vx[i], vy[i]);
}

static void vec_neq_int_v_int_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t vy = *(rint_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neq_int_int(vx[i], vy);
}

static void vec_neq_int_s_int_v(void *r, void *x, void *y, int len)
{
    rint_t vx = *(rint_t *)x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neq_int_int(vx, vy[i]);
}

static void vec_neq_int_s_int_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_neq_int_int(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_neq_int_v_boolean_v(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rboolean_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neq_int_boolean(vx[i], vy[i]);
}

static void vec_neq_int_v_boolean_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rboolean_t vy = *(rboolean_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neq_int_boolean(vx[i], vy);
}

static void vec_neq_int_s_boolean_v(void *r, void *x, void *y, int len)
{
    rint_t vx = *(rint_t *)x;
    rboolean_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neq_int_boolean(vx, vy[i]);
}

static void vec_neq_int_s_boolean_s(void *r, void *x, void *y, int len)
{
    rint_t *vx = x;
    rboolean_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_neq_int_boolean(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  

  

  

  

  
    
  
    
    
  
  
    
    
  

static void vec_neq_boolean_v_double_v(void *r, void *x, void *y, int len)
{
    rboolean_t *vx = x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neq_boolean_double(vx[i], vy[i]);
}

static void vec_neq_boolean_v_double_s(void *r, void *x, void *y, int len)
{
    rboolean_t *vx = x;
    rdouble_t vy = *(rdouble_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neq_boolean_double(vx[i], vy);
}

static void vec_neq_boolean_s_double_v(void *r, void *x, void *y, int len)
{
    rboolean_t vx = *(rboolean_t *)x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neq_boolean_double(vx, vy[i]);
}

static void vec_neq_boolean_s_double_s(void *r, void *x, void *y, int len)
{
    rboolean_t *vx = x;
    rdouble_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_neq_boolean_double(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_neq_boolean_v_int_v(void *r, void *x, void *y, int len)
{
    rboolean_t *vx = x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neq_boolean_int(vx[i], vy[i]);
}

static void vec_neq_boolean_v_int_s(void *r, void *x, void *y, int len)
{
    rboolean_t *vx = x;
    rint_t vy = *(rint_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neq_boolean_int(vx[i], vy);
}

static void vec_neq_boolean_s_int_v(void *r, void *x, void *y, int len)
{
    rboolean_t vx = *(rboolean_t *)x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neq_boolean_int(vx, vy[i]);
}

static void vec_neq_boolean_s_int_s(void *r, void *x, void *y, int len)
{
    rboolean_t *vx = x;
    rint_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_neq_boolean_int(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  
    
    
  

static void vec_neq_boolean_v_boolean_v(void *r, void *x, void *y, int len)
{
    rboolean_t *vx = x;
    rboolean_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neq_boolean_boolean(vx[i], vy[i]);
}

static void vec_neq_boolean_v_boolean_s(void *r, void *x, void *y, int len)
{
    rboolean_t *vx = x;
    rboolean_t vy = *(rboolean_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neq_boolean_boolean(vx[i], vy);
}

static void vec_neq_boolean_s_boolean_v(void *r, void *x, void *y, int len)
{
    rboolean_t vx = *(rboolean_t *)x;
    rboolean_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_neq_boolean_boolean(vx, vy[i]);
}

static void vec_neq_boolean_s_boolean_s(void *r, void *x, void *y, int len)
{
    rboolean_t *vx = x;
    rboolean_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_neq_boolean_boolean(*vx, *vy);
}

/*
  
*/
// 
// 
  
    
  

  

  

  

  
    
  

  

  

  



const binop_fn neq_funcs[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR] = {
    
  [CODE_sca][CODE_sca][CODE_double][CODE_double] = vec_neq_double_s_double_s,
  [CODE_sca][CODE_vec][CODE_double][CODE_double] = vec_neq_double_s_double_v,
  [CODE_vec][CODE_sca][CODE_double][CODE_double] = vec_neq_double_v_double_s,
  [CODE_vec][CODE_vec][CODE_double][CODE_double] = vec_neq_double_v_double_v,
  
  [CODE_sca][CODE_sca][CODE_double][CODE_int] = vec_neq_double_s_int_s,
  [CODE_sca][CODE_vec][CODE_double][CODE_int] = vec_neq_double_s_int_v,
  [CODE_vec][CODE_sca][CODE_double][CODE_int] = vec_neq_double_v_int_s,
  [CODE_vec][CODE_vec][CODE_double][CODE_int] = vec_neq_double_v_int_v,
  
  [CODE_sca][CODE_sca][CODE_double][CODE_boolean] = vec_neq_double_s_boolean_s,
  [CODE_sca][CODE_vec][CODE_double][CODE_boolean] = vec_neq_double_s_boolean_v,
  [CODE_vec][CODE_sca][CODE_double][CODE_boolean] = vec_neq_double_v_boolean_s,
  [CODE_vec][CODE_vec][CODE_double][CODE_boolean] = vec_neq_double_v_boolean_v,
  
  [CODE_sca][CODE_sca][CODE_int][CODE_double] = vec_neq_int_s_double_s,
  [CODE_sca][CODE_vec][CODE_int][CODE_double] = vec_neq_int_s_double_v,
  [CODE_vec][CODE_sca][CODE_int][CODE_double] = vec_neq_int_v_double_s,
  [CODE_vec][CODE_vec][CODE_int][CODE_double] = vec_neq_int_v_double_v,
  
  [CODE_sca][CODE_sca][CODE_int][CODE_int] = vec_neq_int_s_int_s,
  [CODE_sca][CODE_vec][CODE_int][CODE_int] = vec_neq_int_s_int_v,
  [CODE_vec][CODE_sca][CODE_int][CODE_int] = vec_neq_int_v_int_s,
  [CODE_vec][CODE_vec][CODE_int][CODE_int] = vec_neq_int_v_int_v,
  
  [CODE_sca][CODE_sca][CODE_int][CODE_boolean] = vec_neq_int_s_boolean_s,
  [CODE_sca][CODE_vec][CODE_int][CODE_boolean] = vec_neq_int_s_boolean_v,
  [CODE_vec][CODE_sca][CODE_int][CODE_boolean] = vec_neq_int_v_boolean_s,
  [CODE_vec][CODE_vec][CODE_int][CODE_boolean] = vec_neq_int_v_boolean_v,
  
  [CODE_sca][CODE_sca][CODE_boolean][CODE_double] = vec_neq_boolean_s_double_s,
  [CODE_sca][CODE_vec][CODE_boolean][CODE_double] = vec_neq_boolean_s_double_v,
  [CODE_vec][CODE_sca][CODE_boolean][CODE_double] = vec_neq_boolean_v_double_s,
  [CODE_vec][CODE_vec][CODE_boolean][CODE_double] = vec_neq_boolean_v_double_v,
  
  [CODE_sca][CODE_sca][CODE_boolean][CODE_int] = vec_neq_boolean_s_int_s,
  [CODE_sca][CODE_vec][CODE_boolean][CODE_int] = vec_neq_boolean_s_int_v,
  [CODE_vec][CODE_sca][CODE_boolean][CODE_int] = vec_neq_boolean_v_int_s,
  [CODE_vec][CODE_vec][CODE_boolean][CODE_int] = vec_neq_boolean_v_int_v,
  
  [CODE_sca][CODE_sca][CODE_boolean][CODE_boolean] = vec_neq_boolean_s_boolean_s,
  [CODE_sca][CODE_vec][CODE_boolean][CODE_boolean] = vec_neq_boolean_s_boolean_v,
  [CODE_vec][CODE_sca][CODE_boolean][CODE_boolean] = vec_neq_boolean_v_boolean_s,
  [CODE_vec][CODE_vec][CODE_boolean][CODE_boolean] = vec_neq_boolean_v_boolean_v,
  
};
// 
// 
    
  

  

  

// 
  
    
    
// 
  
    
    

static void vec_is_na_double_v(void *r, void *x, int len)
{
    rdouble_t *vx = x;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_is_na_double(vx[i]);
}

static void vec_is_na_double_s(void *r, void *x, int len)
{
    rdouble_t *vx = x;
    rboolean_t *vr = r;
    *vr = arith_is_na_double(*vx);
}

/*
  
*/
// 
// 
    
  
    
    

static void vec_is_na_int_v(void *r, void *x, int len)
{
    rint_t *vx = x;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_is_na_int(vx[i]);
}

static void vec_is_na_int_s(void *r, void *x, int len)
{
    rint_t *vx = x;
    rboolean_t *vr = r;
    *vr = arith_is_na_int(*vx);
}

/*
  
*/
// 
// 
    
  
    
    

static void vec_is_na_boolean_v(void *r, void *x, int len)
{
    rboolean_t *vx = x;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_is_na_boolean(vx[i]);
}

static void vec_is_na_boolean_s(void *r, void *x, int len)
{
    rboolean_t *vx = x;
    rboolean_t *vr = r;
    *vr = arith_is_na_boolean(*vx);
}

/*
  
*/
// 
// 
    
  

  

  

  


const unop_fn is_na_funcs[2][CODE_MAX_SCALAR] = {
    
  [CODE_sca][CODE_double] = vec_is_na_double_s,
  [CODE_vec][CODE_double] = vec_is_na_double_v,
  
  [CODE_sca][CODE_int] = vec_is_na_int_s,
  [CODE_vec][CODE_int] = vec_is_na_int_v,
  
  [CODE_sca][CODE_boolean] = vec_is_na_boolean_s,
  [CODE_vec][CODE_boolean] = vec_is_na_boolean_v,
  
};
// 
// 
    
  

  

// 
// 
// 
  
    
    

static void vec_and_boolean_v_boolean_v(void *r, void *x, void *y, int len)
{
    rboolean_t *vx = x;
    rboolean_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_and_boolean_boolean(vx[i], vy[i]);
}

static void vec_and_boolean_v_boolean_s(void *r, void *x, void *y, int len)
{
    rboolean_t *vx = x;
    rboolean_t vy = *(rboolean_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_and_boolean_boolean(vx[i], vy);
}

static void vec_and_boolean_s_boolean_v(void *r, void *x, void *y, int len)
{
    rboolean_t vx = *(rboolean_t *)x;
    rboolean_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_and_boolean_boolean(vx, vy[i]);
}

static void vec_and_boolean_s_boolean_s(void *r, void *x, void *y, int len)
{
    rboolean_t *vx = x;
    rboolean_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_and_boolean_boolean(*vx, *vy);
}

/*
  
*/
// 

const binop_fn and_funcs[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR] = {
    
  [CODE_sca][CODE_sca][CODE_boolean][CODE_boolean] = vec_and_boolean_s_boolean_s,
  [CODE_sca][CODE_vec][CODE_boolean][CODE_boolean] = vec_and_boolean_s_boolean_v,
  [CODE_vec][CODE_sca][CODE_boolean][CODE_boolean] = vec_and_boolean_v_boolean_s,
  [CODE_vec][CODE_vec][CODE_boolean][CODE_boolean] = vec_and_boolean_v_boolean_v,
  
};
// 
// 
    
  
    
    

static void vec_or_boolean_v_boolean_v(void *r, void *x, void *y, int len)
{
    rboolean_t *vx = x;
    rboolean_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_or_boolean_boolean(vx[i], vy[i]);
}

static void vec_or_boolean_v_boolean_s(void *r, void *x, void *y, int len)
{
    rboolean_t *vx = x;
    rboolean_t vy = *(rboolean_t *)y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_or_boolean_boolean(vx[i], vy);
}

static void vec_or_boolean_s_boolean_v(void *r, void *x, void *y, int len)
{
    rboolean_t vx = *(rboolean_t *)x;
    rboolean_t *vy = y;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_or_boolean_boolean(vx, vy[i]);
}

static void vec_or_boolean_s_boolean_s(void *r, void *x, void *y, int len)
{
    rboolean_t *vx = x;
    rboolean_t *vy = y;
    rboolean_t *vr = r;
    *vr = arith_or_boolean_boolean(*vx, *vy);
}

/*
  
*/
// 

const binop_fn or_funcs[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR] = {
    
  [CODE_sca][CODE_sca][CODE_boolean][CODE_boolean] = vec_or_boolean_s_boolean_s,
  [CODE_sca][CODE_vec][CODE_boolean][CODE_boolean] = vec_or_boolean_s_boolean_v,
  [CODE_vec][CODE_sca][CODE_boolean][CODE_boolean] = vec_or_boolean_v_boolean_s,
  [CODE_vec][CODE_vec][CODE_boolean][CODE_boolean] = vec_or_boolean_v_boolean_v,
  
};
// 
// 
    
  

  

  

// 
  
    
    

static void vec_not_boolean_v(void *r, void *x, int len)
{
    rboolean_t *vx = x;
    rboolean_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = arith_not_boolean(vx[i]);
}

static void vec_not_boolean_s(void *r, void *x, int len)
{
    rboolean_t *vx = x;
    rboolean_t *vr = r;
    *vr = arith_not_boolean(*vx);
}

/*
  
*/
// 

const unop_fn not_funcs[2][CODE_MAX_SCALAR] = {
    
  [CODE_sca][CODE_boolean] = vec_not_boolean_s,
  [CODE_vec][CODE_boolean] = vec_not_boolean_v,
  
};
// 
// 
    
  

  

// 
  
    
    

static void vec_reduce_and_boolean(void *r, void *x, int len)
{
    rboolean_t vr = true;
    rboolean_t *vx = x;
    for(int i=0; i < len; i++)
        vr = arith_and_boolean_boolean(vr, vx[i]);
    *(rboolean_t *)r = vr;
}

/*
  
*/
// 

const unop_fn reduce_and_funcs[CODE_MAX_SCALAR] = {
    
  [CODE_boolean] = vec_reduce_and_boolean,
  
};
// 
// 
    
  
    
    

static void vec_reduce_or_boolean(void *r, void *x, int len)
{
    rboolean_t vr = false;
    rboolean_t *vx = x;
    for(int i=0; i < len; i++)
        vr = arith_or_boolean_boolean(vr, vx[i]);
    *(rboolean_t *)r = vr;
}

/*
  
*/
// 

const unop_fn reduce_or_funcs[CODE_MAX_SCALAR] = {
    
  [CODE_boolean] = vec_reduce_or_boolean,
  
};
// 
// 
    
  

  

  

//   
// 
// 
// 
// 
// 
// 
  
    
    

static void vec_atan2_double_v_double_v(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = atan2(vx[i], vy[i]);
}

static void vec_atan2_double_v_double_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t vy = *(rdouble_t *)y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = atan2(vx[i], vy);
}

static void vec_atan2_double_s_double_v(void *r, void *x, void *y, int len)
{
    rdouble_t vx = *(rdouble_t *)x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = atan2(vx, vy[i]);
}

static void vec_atan2_double_s_double_s(void *r, void *x, void *y, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vy = y;
    rdouble_t *vr = r;
    *vr = atan2(*vx, *vy);
}

/*
  
*/
// 

const binop_fn atan2_funcs[2][2][CODE_MAX_SCALAR][CODE_MAX_SCALAR] = {
    
  [CODE_sca][CODE_sca][CODE_double][CODE_double] = vec_atan2_double_s_double_s,
  [CODE_sca][CODE_vec][CODE_double][CODE_double] = vec_atan2_double_s_double_v,
  [CODE_vec][CODE_sca][CODE_double][CODE_double] = vec_atan2_double_v_double_s,
  [CODE_vec][CODE_vec][CODE_double][CODE_double] = vec_atan2_double_v_double_v,
  
};
// 
// 
    
  

  

// 
  
    
    

static void vec_sqrt_double_v(void *r, void *x, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = sqrt(vx[i]);
}

static void vec_sqrt_double_s(void *r, void *x, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vr = r;
    *vr = sqrt(*vx);
}

/*
  
*/
// 

const unop_fn sqrt_funcs[2][CODE_MAX_SCALAR] = {
    
  [CODE_sca][CODE_double] = vec_sqrt_double_s,
  [CODE_vec][CODE_double] = vec_sqrt_double_v,
  
};
// 
// 
    
  
    
    

static void vec_sin_double_v(void *r, void *x, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = sin(vx[i]);
}

static void vec_sin_double_s(void *r, void *x, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vr = r;
    *vr = sin(*vx);
}

/*
  
*/
// 

const unop_fn sin_funcs[2][CODE_MAX_SCALAR] = {
    
  [CODE_sca][CODE_double] = vec_sin_double_s,
  [CODE_vec][CODE_double] = vec_sin_double_v,
  
};
// 
// 
    
  
    
    

static void vec_cos_double_v(void *r, void *x, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = cos(vx[i]);
}

static void vec_cos_double_s(void *r, void *x, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vr = r;
    *vr = cos(*vx);
}

/*
  
*/
// 

const unop_fn cos_funcs[2][CODE_MAX_SCALAR] = {
    
  [CODE_sca][CODE_double] = vec_cos_double_s,
  [CODE_vec][CODE_double] = vec_cos_double_v,
  
};
// 
// 
    
  
    
    

static void vec_tan_double_v(void *r, void *x, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = tan(vx[i]);
}

static void vec_tan_double_s(void *r, void *x, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vr = r;
    *vr = tan(*vx);
}

/*
  
*/
// 

const unop_fn tan_funcs[2][CODE_MAX_SCALAR] = {
    
  [CODE_sca][CODE_double] = vec_tan_double_s,
  [CODE_vec][CODE_double] = vec_tan_double_v,
  
};
// 
// 
    
  
    
    

static void vec_asin_double_v(void *r, void *x, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = asin(vx[i]);
}

static void vec_asin_double_s(void *r, void *x, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vr = r;
    *vr = asin(*vx);
}

/*
  
*/
// 

const unop_fn asin_funcs[2][CODE_MAX_SCALAR] = {
    
  [CODE_sca][CODE_double] = vec_asin_double_s,
  [CODE_vec][CODE_double] = vec_asin_double_v,
  
};
// 
// 
    
  
    
    

static void vec_acos_double_v(void *r, void *x, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = acos(vx[i]);
}

static void vec_acos_double_s(void *r, void *x, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vr = r;
    *vr = acos(*vx);
}

/*
  
*/
// 

const unop_fn acos_funcs[2][CODE_MAX_SCALAR] = {
    
  [CODE_sca][CODE_double] = vec_acos_double_s,
  [CODE_vec][CODE_double] = vec_acos_double_v,
  
};
// 
// 
    
  
    
    

static void vec_atan_double_v(void *r, void *x, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = atan(vx[i]);
}

static void vec_atan_double_s(void *r, void *x, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vr = r;
    *vr = atan(*vx);
}

/*
  
*/
// 

const unop_fn atan_funcs[2][CODE_MAX_SCALAR] = {
    
  [CODE_sca][CODE_double] = vec_atan_double_s,
  [CODE_vec][CODE_double] = vec_atan_double_v,
  
};
// 
// 
    
  
    
    

static void vec_log_double_v(void *r, void *x, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = log(vx[i]);
}

static void vec_log_double_s(void *r, void *x, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vr = r;
    *vr = log(*vx);
}

/*
  
*/
// 

const unop_fn log_funcs[2][CODE_MAX_SCALAR] = {
    
  [CODE_sca][CODE_double] = vec_log_double_s,
  [CODE_vec][CODE_double] = vec_log_double_v,
  
};
// 
// 
    
  
    
    

static void vec_exp_double_v(void *r, void *x, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = exp(vx[i]);
}

static void vec_exp_double_s(void *r, void *x, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vr = r;
    *vr = exp(*vx);
}

/*
  
*/
// 

const unop_fn exp_funcs[2][CODE_MAX_SCALAR] = {
    
  [CODE_sca][CODE_double] = vec_exp_double_s,
  [CODE_vec][CODE_double] = vec_exp_double_v,
  
};
// 
// 
    
  
    
    

static void vec_round_double_v(void *r, void *x, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vr = r;
    for(int i=0; i < len; i++)
        vr[i] = round(vx[i]);
}

static void vec_round_double_s(void *r, void *x, int len)
{
    rdouble_t *vx = x;
    rdouble_t *vr = r;
    *vr = round(*vx);
}

/*
  
*/
// 

const unop_fn round_funcs[2][CODE_MAX_SCALAR] = {
    
  [CODE_sca][CODE_double] = vec_round_double_s,
  [CODE_vec][CODE_double] = vec_round_double_v,
  
};
// 
// 
    
  

  

  

  

  

  

  

  

  

  

  

//  
//   
// 
  
  
    
    
  
  
    
    
  
// 
// 
  
    
  
    
    
  
// 
static void vec_conv_v_double_to_int(void *dest, void *src, int len)
{
    rdouble_t *vs = src;
    rint_t *vd = dest;
    for(int i=0; i < len; i++)
        vd[i] = arith_conv_double_to_int(vs[i]);
}
static void vec_conv_s_double_to_int(void *dest, void *src, int len)
{
    *(rint_t *)dest = arith_conv_double_to_int(*(rdouble_t *)src);
}
/*
  
*/
// 
// 
  
    
  
    
    
  
// 
static void vec_conv_v_double_to_boolean(void *dest, void *src, int len)
{
    rdouble_t *vs = src;
    rboolean_t *vd = dest;
    for(int i=0; i < len; i++)
        vd[i] = arith_conv_double_to_boolean(vs[i]);
}
static void vec_conv_s_double_to_boolean(void *dest, void *src, int len)
{
    *(rboolean_t *)dest = arith_conv_double_to_boolean(*(rdouble_t *)src);
}
/*
  
*/
// 
// 
  
    
  

  

  

  

  
    
  
    
    
  
  
    
    
  
// 
static void vec_conv_v_int_to_double(void *dest, void *src, int len)
{
    rint_t *vs = src;
    rdouble_t *vd = dest;
    for(int i=0; i < len; i++)
        vd[i] = arith_conv_int_to_double(vs[i]);
}
static void vec_conv_s_int_to_double(void *dest, void *src, int len)
{
    *(rdouble_t *)dest = arith_conv_int_to_double(*(rint_t *)src);
}
/*
  
*/
// 
// 
  
    
  
    
    
  
// 
// 
  
    
  
    
    
  
// 
static void vec_conv_v_int_to_boolean(void *dest, void *src, int len)
{
    rint_t *vs = src;
    rboolean_t *vd = dest;
    for(int i=0; i < len; i++)
        vd[i] = arith_conv_int_to_boolean(vs[i]);
}
static void vec_conv_s_int_to_boolean(void *dest, void *src, int len)
{
    *(rboolean_t *)dest = arith_conv_int_to_boolean(*(rint_t *)src);
}
/*
  
*/
// 
// 
  
    
  

  

  

  

  
    
  
    
    
  
  
    
    
  
// 
static void vec_conv_v_boolean_to_double(void *dest, void *src, int len)
{
    rboolean_t *vs = src;
    rdouble_t *vd = dest;
    for(int i=0; i < len; i++)
        vd[i] = arith_conv_boolean_to_double(vs[i]);
}
static void vec_conv_s_boolean_to_double(void *dest, void *src, int len)
{
    *(rdouble_t *)dest = arith_conv_boolean_to_double(*(rboolean_t *)src);
}
/*
  
*/
// 
// 
  
    
  
    
    
  
// 
static void vec_conv_v_boolean_to_int(void *dest, void *src, int len)
{
    rboolean_t *vs = src;
    rint_t *vd = dest;
    for(int i=0; i < len; i++)
        vd[i] = arith_conv_boolean_to_int(vs[i]);
}
static void vec_conv_s_boolean_to_int(void *dest, void *src, int len)
{
    *(rint_t *)dest = arith_conv_boolean_to_int(*(rboolean_t *)src);
}
/*
  
*/
// 
// 
  
    
  
    
    
  
// 
// 
  
    
  

  

  

  

  
    
  

  

  

  



const conv_fn conv_funcs[2][CODE_MAX_SCALAR][CODE_MAX_SCALAR] = {
    
  [CODE_sca][CODE_double][CODE_int] = vec_conv_s_double_to_int,
  [CODE_vec][CODE_double][CODE_int] = vec_conv_v_double_to_int,
  
  [CODE_sca][CODE_double][CODE_boolean] = vec_conv_s_double_to_boolean,
  [CODE_vec][CODE_double][CODE_boolean] = vec_conv_v_double_to_boolean,
  
  [CODE_sca][CODE_int][CODE_double] = vec_conv_s_int_to_double,
  [CODE_vec][CODE_int][CODE_double] = vec_conv_v_int_to_double,
  
  [CODE_sca][CODE_int][CODE_boolean] = vec_conv_s_int_to_boolean,
  [CODE_vec][CODE_int][CODE_boolean] = vec_conv_v_int_to_boolean,
  
  [CODE_sca][CODE_boolean][CODE_double] = vec_conv_s_boolean_to_double,
  [CODE_vec][CODE_boolean][CODE_double] = vec_conv_v_boolean_to_double,
  
  [CODE_sca][CODE_boolean][CODE_int] = vec_conv_s_boolean_to_int,
  [CODE_vec][CODE_boolean][CODE_int] = vec_conv_v_boolean_to_int,
  
};
// 
/*
  
*/
#define NEXT1(p, start, end) ((p+1 >= end) ? start : p+1)
// 
  
  
    
    
  
  
    
    
  

// v[ints] = r
// recycle r along idx
// idxs which are out of range are ignored
static void
int_set_double_from_double(void *r, void *v, void *idx,
                                    int rlen, int vlen, int ilen)
{
    rint_t *vi = idx;
    rdouble_t *vv = v;
    rdouble_t *vr = r;
    rdouble_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<ilen; i++)
    {
        rint_t j = vi[i];
        if(!(int_na(j) || j > vlen || j <= 0))
        {
            vv[j-1] = *vr;
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[bools] = r
// recycles r along idx, and idx along v
static void
bool_set_double_from_double(void *r, void *v, void *idx,
                                     int rlen, int vlen, int ilen)
{
    rboolean_t *vi = idx;
    rboolean_t *istart = vi, *iend = vi + ilen;
    rdouble_t *vv = v;
    rdouble_t *vr = r;
    rdouble_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        rboolean_t b = *vi;
        vi = NEXT1(vi, istart, iend);
        if(b == true)
        {
            vv[i] = *vr;
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[] = r
static void
missing_set_double_from_double(void *r, void *v, void *idx,
                                        int rlen, int vlen, int ilen)
{
    rdouble_t *vv = v;
    rdouble_t *vr = r;
    rdouble_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        vv[i] = *vr;
        vr = NEXT1(vr, rstart, rend);
    }
}

/*
  
  
  
*/
// 
  
    
  
    
    
  

// v[ints] = r
// recycle r along idx
// idxs which are out of range are ignored
static void
int_set_double_from_int(void *r, void *v, void *idx,
                                    int rlen, int vlen, int ilen)
{
    rint_t *vi = idx;
    rdouble_t *vv = v;
    rint_t *vr = r;
    rint_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<ilen; i++)
    {
        rint_t j = vi[i];
        if(!(int_na(j) || j > vlen || j <= 0))
        {
            vv[j-1] = arith_conv_int_to_double(*vr);
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[bools] = r
// recycles r along idx, and idx along v
static void
bool_set_double_from_int(void *r, void *v, void *idx,
                                     int rlen, int vlen, int ilen)
{
    rboolean_t *vi = idx;
    rboolean_t *istart = vi, *iend = vi + ilen;
    rdouble_t *vv = v;
    rint_t *vr = r;
    rint_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        rboolean_t b = *vi;
        vi = NEXT1(vi, istart, iend);
        if(b == true)
        {
            vv[i] = arith_conv_int_to_double(*vr);
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[] = r
static void
missing_set_double_from_int(void *r, void *v, void *idx,
                                        int rlen, int vlen, int ilen)
{
    rdouble_t *vv = v;
    rint_t *vr = r;
    rint_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        vv[i] = arith_conv_int_to_double(*vr);
        vr = NEXT1(vr, rstart, rend);
    }
}

/*
  
  
  
*/
// 
  
    
  
    
    
  

// v[ints] = r
// recycle r along idx
// idxs which are out of range are ignored
static void
int_set_double_from_boolean(void *r, void *v, void *idx,
                                    int rlen, int vlen, int ilen)
{
    rint_t *vi = idx;
    rdouble_t *vv = v;
    rboolean_t *vr = r;
    rboolean_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<ilen; i++)
    {
        rint_t j = vi[i];
        if(!(int_na(j) || j > vlen || j <= 0))
        {
            vv[j-1] = arith_conv_boolean_to_double(*vr);
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[bools] = r
// recycles r along idx, and idx along v
static void
bool_set_double_from_boolean(void *r, void *v, void *idx,
                                     int rlen, int vlen, int ilen)
{
    rboolean_t *vi = idx;
    rboolean_t *istart = vi, *iend = vi + ilen;
    rdouble_t *vv = v;
    rboolean_t *vr = r;
    rboolean_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        rboolean_t b = *vi;
        vi = NEXT1(vi, istart, iend);
        if(b == true)
        {
            vv[i] = arith_conv_boolean_to_double(*vr);
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[] = r
static void
missing_set_double_from_boolean(void *r, void *v, void *idx,
                                        int rlen, int vlen, int ilen)
{
    rdouble_t *vv = v;
    rboolean_t *vr = r;
    rboolean_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        vv[i] = arith_conv_boolean_to_double(*vr);
        vr = NEXT1(vr, rstart, rend);
    }
}

/*
  
  
  
*/
// 
  
    
  
    
    
  

// v[ints] = r
// recycle r along idx
// idxs which are out of range are ignored
static void
int_set_double_from_ptr(void *r, void *v, void *idx,
                                    int rlen, int vlen, int ilen)
{
    rint_t *vi = idx;
    rdouble_t *vv = v;
    rptr_t *vr = r;
    rptr_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<ilen; i++)
    {
        rint_t j = vi[i];
        if(!(int_na(j) || j > vlen || j <= 0))
        {
            vv[j-1] = arith_conv_ptr_to_double(*vr);
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[bools] = r
// recycles r along idx, and idx along v
static void
bool_set_double_from_ptr(void *r, void *v, void *idx,
                                     int rlen, int vlen, int ilen)
{
    rboolean_t *vi = idx;
    rboolean_t *istart = vi, *iend = vi + ilen;
    rdouble_t *vv = v;
    rptr_t *vr = r;
    rptr_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        rboolean_t b = *vi;
        vi = NEXT1(vi, istart, iend);
        if(b == true)
        {
            vv[i] = arith_conv_ptr_to_double(*vr);
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[] = r
static void
missing_set_double_from_ptr(void *r, void *v, void *idx,
                                        int rlen, int vlen, int ilen)
{
    rdouble_t *vv = v;
    rptr_t *vr = r;
    rptr_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        vv[i] = arith_conv_ptr_to_double(*vr);
        vr = NEXT1(vr, rstart, rend);
    }
}

/*
  
  
  
*/
// 
  
    
  

  

  

  

  

  
    
  
    
    
  
  
    
    
  

// v[ints] = r
// recycle r along idx
// idxs which are out of range are ignored
static void
int_set_int_from_double(void *r, void *v, void *idx,
                                    int rlen, int vlen, int ilen)
{
    rint_t *vi = idx;
    rint_t *vv = v;
    rdouble_t *vr = r;
    rdouble_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<ilen; i++)
    {
        rint_t j = vi[i];
        if(!(int_na(j) || j > vlen || j <= 0))
        {
            vv[j-1] = arith_conv_double_to_int(*vr);
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[bools] = r
// recycles r along idx, and idx along v
static void
bool_set_int_from_double(void *r, void *v, void *idx,
                                     int rlen, int vlen, int ilen)
{
    rboolean_t *vi = idx;
    rboolean_t *istart = vi, *iend = vi + ilen;
    rint_t *vv = v;
    rdouble_t *vr = r;
    rdouble_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        rboolean_t b = *vi;
        vi = NEXT1(vi, istart, iend);
        if(b == true)
        {
            vv[i] = arith_conv_double_to_int(*vr);
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[] = r
static void
missing_set_int_from_double(void *r, void *v, void *idx,
                                        int rlen, int vlen, int ilen)
{
    rint_t *vv = v;
    rdouble_t *vr = r;
    rdouble_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        vv[i] = arith_conv_double_to_int(*vr);
        vr = NEXT1(vr, rstart, rend);
    }
}

/*
  
  
  
*/
// 
  
    
  
    
    
  

// v[ints] = r
// recycle r along idx
// idxs which are out of range are ignored
static void
int_set_int_from_int(void *r, void *v, void *idx,
                                    int rlen, int vlen, int ilen)
{
    rint_t *vi = idx;
    rint_t *vv = v;
    rint_t *vr = r;
    rint_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<ilen; i++)
    {
        rint_t j = vi[i];
        if(!(int_na(j) || j > vlen || j <= 0))
        {
            vv[j-1] = *vr;
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[bools] = r
// recycles r along idx, and idx along v
static void
bool_set_int_from_int(void *r, void *v, void *idx,
                                     int rlen, int vlen, int ilen)
{
    rboolean_t *vi = idx;
    rboolean_t *istart = vi, *iend = vi + ilen;
    rint_t *vv = v;
    rint_t *vr = r;
    rint_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        rboolean_t b = *vi;
        vi = NEXT1(vi, istart, iend);
        if(b == true)
        {
            vv[i] = *vr;
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[] = r
static void
missing_set_int_from_int(void *r, void *v, void *idx,
                                        int rlen, int vlen, int ilen)
{
    rint_t *vv = v;
    rint_t *vr = r;
    rint_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        vv[i] = *vr;
        vr = NEXT1(vr, rstart, rend);
    }
}

/*
  
  
  
*/
// 
  
    
  
    
    
  

// v[ints] = r
// recycle r along idx
// idxs which are out of range are ignored
static void
int_set_int_from_boolean(void *r, void *v, void *idx,
                                    int rlen, int vlen, int ilen)
{
    rint_t *vi = idx;
    rint_t *vv = v;
    rboolean_t *vr = r;
    rboolean_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<ilen; i++)
    {
        rint_t j = vi[i];
        if(!(int_na(j) || j > vlen || j <= 0))
        {
            vv[j-1] = arith_conv_boolean_to_int(*vr);
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[bools] = r
// recycles r along idx, and idx along v
static void
bool_set_int_from_boolean(void *r, void *v, void *idx,
                                     int rlen, int vlen, int ilen)
{
    rboolean_t *vi = idx;
    rboolean_t *istart = vi, *iend = vi + ilen;
    rint_t *vv = v;
    rboolean_t *vr = r;
    rboolean_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        rboolean_t b = *vi;
        vi = NEXT1(vi, istart, iend);
        if(b == true)
        {
            vv[i] = arith_conv_boolean_to_int(*vr);
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[] = r
static void
missing_set_int_from_boolean(void *r, void *v, void *idx,
                                        int rlen, int vlen, int ilen)
{
    rint_t *vv = v;
    rboolean_t *vr = r;
    rboolean_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        vv[i] = arith_conv_boolean_to_int(*vr);
        vr = NEXT1(vr, rstart, rend);
    }
}

/*
  
  
  
*/
// 
  
    
  
    
    
  

// v[ints] = r
// recycle r along idx
// idxs which are out of range are ignored
static void
int_set_int_from_ptr(void *r, void *v, void *idx,
                                    int rlen, int vlen, int ilen)
{
    rint_t *vi = idx;
    rint_t *vv = v;
    rptr_t *vr = r;
    rptr_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<ilen; i++)
    {
        rint_t j = vi[i];
        if(!(int_na(j) || j > vlen || j <= 0))
        {
            vv[j-1] = arith_conv_ptr_to_int(*vr);
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[bools] = r
// recycles r along idx, and idx along v
static void
bool_set_int_from_ptr(void *r, void *v, void *idx,
                                     int rlen, int vlen, int ilen)
{
    rboolean_t *vi = idx;
    rboolean_t *istart = vi, *iend = vi + ilen;
    rint_t *vv = v;
    rptr_t *vr = r;
    rptr_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        rboolean_t b = *vi;
        vi = NEXT1(vi, istart, iend);
        if(b == true)
        {
            vv[i] = arith_conv_ptr_to_int(*vr);
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[] = r
static void
missing_set_int_from_ptr(void *r, void *v, void *idx,
                                        int rlen, int vlen, int ilen)
{
    rint_t *vv = v;
    rptr_t *vr = r;
    rptr_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        vv[i] = arith_conv_ptr_to_int(*vr);
        vr = NEXT1(vr, rstart, rend);
    }
}

/*
  
  
  
*/
// 
  
    
  

  

  

  

  

  
    
  
    
    
  
  
    
    
  

// v[ints] = r
// recycle r along idx
// idxs which are out of range are ignored
static void
int_set_boolean_from_double(void *r, void *v, void *idx,
                                    int rlen, int vlen, int ilen)
{
    rint_t *vi = idx;
    rboolean_t *vv = v;
    rdouble_t *vr = r;
    rdouble_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<ilen; i++)
    {
        rint_t j = vi[i];
        if(!(int_na(j) || j > vlen || j <= 0))
        {
            vv[j-1] = arith_conv_double_to_boolean(*vr);
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[bools] = r
// recycles r along idx, and idx along v
static void
bool_set_boolean_from_double(void *r, void *v, void *idx,
                                     int rlen, int vlen, int ilen)
{
    rboolean_t *vi = idx;
    rboolean_t *istart = vi, *iend = vi + ilen;
    rboolean_t *vv = v;
    rdouble_t *vr = r;
    rdouble_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        rboolean_t b = *vi;
        vi = NEXT1(vi, istart, iend);
        if(b == true)
        {
            vv[i] = arith_conv_double_to_boolean(*vr);
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[] = r
static void
missing_set_boolean_from_double(void *r, void *v, void *idx,
                                        int rlen, int vlen, int ilen)
{
    rboolean_t *vv = v;
    rdouble_t *vr = r;
    rdouble_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        vv[i] = arith_conv_double_to_boolean(*vr);
        vr = NEXT1(vr, rstart, rend);
    }
}

/*
  
  
  
*/
// 
  
    
  
    
    
  

// v[ints] = r
// recycle r along idx
// idxs which are out of range are ignored
static void
int_set_boolean_from_int(void *r, void *v, void *idx,
                                    int rlen, int vlen, int ilen)
{
    rint_t *vi = idx;
    rboolean_t *vv = v;
    rint_t *vr = r;
    rint_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<ilen; i++)
    {
        rint_t j = vi[i];
        if(!(int_na(j) || j > vlen || j <= 0))
        {
            vv[j-1] = arith_conv_int_to_boolean(*vr);
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[bools] = r
// recycles r along idx, and idx along v
static void
bool_set_boolean_from_int(void *r, void *v, void *idx,
                                     int rlen, int vlen, int ilen)
{
    rboolean_t *vi = idx;
    rboolean_t *istart = vi, *iend = vi + ilen;
    rboolean_t *vv = v;
    rint_t *vr = r;
    rint_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        rboolean_t b = *vi;
        vi = NEXT1(vi, istart, iend);
        if(b == true)
        {
            vv[i] = arith_conv_int_to_boolean(*vr);
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[] = r
static void
missing_set_boolean_from_int(void *r, void *v, void *idx,
                                        int rlen, int vlen, int ilen)
{
    rboolean_t *vv = v;
    rint_t *vr = r;
    rint_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        vv[i] = arith_conv_int_to_boolean(*vr);
        vr = NEXT1(vr, rstart, rend);
    }
}

/*
  
  
  
*/
// 
  
    
  
    
    
  

// v[ints] = r
// recycle r along idx
// idxs which are out of range are ignored
static void
int_set_boolean_from_boolean(void *r, void *v, void *idx,
                                    int rlen, int vlen, int ilen)
{
    rint_t *vi = idx;
    rboolean_t *vv = v;
    rboolean_t *vr = r;
    rboolean_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<ilen; i++)
    {
        rint_t j = vi[i];
        if(!(int_na(j) || j > vlen || j <= 0))
        {
            vv[j-1] = *vr;
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[bools] = r
// recycles r along idx, and idx along v
static void
bool_set_boolean_from_boolean(void *r, void *v, void *idx,
                                     int rlen, int vlen, int ilen)
{
    rboolean_t *vi = idx;
    rboolean_t *istart = vi, *iend = vi + ilen;
    rboolean_t *vv = v;
    rboolean_t *vr = r;
    rboolean_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        rboolean_t b = *vi;
        vi = NEXT1(vi, istart, iend);
        if(b == true)
        {
            vv[i] = *vr;
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[] = r
static void
missing_set_boolean_from_boolean(void *r, void *v, void *idx,
                                        int rlen, int vlen, int ilen)
{
    rboolean_t *vv = v;
    rboolean_t *vr = r;
    rboolean_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        vv[i] = *vr;
        vr = NEXT1(vr, rstart, rend);
    }
}

/*
  
  
  
*/
// 
  
    
  
    
    
  

// v[ints] = r
// recycle r along idx
// idxs which are out of range are ignored
static void
int_set_boolean_from_ptr(void *r, void *v, void *idx,
                                    int rlen, int vlen, int ilen)
{
    rint_t *vi = idx;
    rboolean_t *vv = v;
    rptr_t *vr = r;
    rptr_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<ilen; i++)
    {
        rint_t j = vi[i];
        if(!(int_na(j) || j > vlen || j <= 0))
        {
            vv[j-1] = arith_conv_ptr_to_boolean(*vr);
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[bools] = r
// recycles r along idx, and idx along v
static void
bool_set_boolean_from_ptr(void *r, void *v, void *idx,
                                     int rlen, int vlen, int ilen)
{
    rboolean_t *vi = idx;
    rboolean_t *istart = vi, *iend = vi + ilen;
    rboolean_t *vv = v;
    rptr_t *vr = r;
    rptr_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        rboolean_t b = *vi;
        vi = NEXT1(vi, istart, iend);
        if(b == true)
        {
            vv[i] = arith_conv_ptr_to_boolean(*vr);
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[] = r
static void
missing_set_boolean_from_ptr(void *r, void *v, void *idx,
                                        int rlen, int vlen, int ilen)
{
    rboolean_t *vv = v;
    rptr_t *vr = r;
    rptr_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        vv[i] = arith_conv_ptr_to_boolean(*vr);
        vr = NEXT1(vr, rstart, rend);
    }
}

/*
  
  
  
*/
// 
  
    
  

  

  

  

  

  
    
  
    
    
  
  
    
    
  

// v[ints] = r
// recycle r along idx
// idxs which are out of range are ignored
static void
int_set_ptr_from_double(void *r, void *v, void *idx,
                                    int rlen, int vlen, int ilen)
{
    rint_t *vi = idx;
    rptr_t *vv = v;
    rdouble_t *vr = r;
    rdouble_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<ilen; i++)
    {
        rint_t j = vi[i];
        if(!(int_na(j) || j > vlen || j <= 0))
        {
            vv[j-1] = arith_conv_double_to_ptr(*vr);
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[bools] = r
// recycles r along idx, and idx along v
static void
bool_set_ptr_from_double(void *r, void *v, void *idx,
                                     int rlen, int vlen, int ilen)
{
    rboolean_t *vi = idx;
    rboolean_t *istart = vi, *iend = vi + ilen;
    rptr_t *vv = v;
    rdouble_t *vr = r;
    rdouble_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        rboolean_t b = *vi;
        vi = NEXT1(vi, istart, iend);
        if(b == true)
        {
            vv[i] = arith_conv_double_to_ptr(*vr);
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[] = r
static void
missing_set_ptr_from_double(void *r, void *v, void *idx,
                                        int rlen, int vlen, int ilen)
{
    rptr_t *vv = v;
    rdouble_t *vr = r;
    rdouble_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        vv[i] = arith_conv_double_to_ptr(*vr);
        vr = NEXT1(vr, rstart, rend);
    }
}

/*
  
  
  
*/
// 
  
    
  
    
    
  

// v[ints] = r
// recycle r along idx
// idxs which are out of range are ignored
static void
int_set_ptr_from_int(void *r, void *v, void *idx,
                                    int rlen, int vlen, int ilen)
{
    rint_t *vi = idx;
    rptr_t *vv = v;
    rint_t *vr = r;
    rint_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<ilen; i++)
    {
        rint_t j = vi[i];
        if(!(int_na(j) || j > vlen || j <= 0))
        {
            vv[j-1] = arith_conv_int_to_ptr(*vr);
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[bools] = r
// recycles r along idx, and idx along v
static void
bool_set_ptr_from_int(void *r, void *v, void *idx,
                                     int rlen, int vlen, int ilen)
{
    rboolean_t *vi = idx;
    rboolean_t *istart = vi, *iend = vi + ilen;
    rptr_t *vv = v;
    rint_t *vr = r;
    rint_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        rboolean_t b = *vi;
        vi = NEXT1(vi, istart, iend);
        if(b == true)
        {
            vv[i] = arith_conv_int_to_ptr(*vr);
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[] = r
static void
missing_set_ptr_from_int(void *r, void *v, void *idx,
                                        int rlen, int vlen, int ilen)
{
    rptr_t *vv = v;
    rint_t *vr = r;
    rint_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        vv[i] = arith_conv_int_to_ptr(*vr);
        vr = NEXT1(vr, rstart, rend);
    }
}

/*
  
  
  
*/
// 
  
    
  
    
    
  

// v[ints] = r
// recycle r along idx
// idxs which are out of range are ignored
static void
int_set_ptr_from_boolean(void *r, void *v, void *idx,
                                    int rlen, int vlen, int ilen)
{
    rint_t *vi = idx;
    rptr_t *vv = v;
    rboolean_t *vr = r;
    rboolean_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<ilen; i++)
    {
        rint_t j = vi[i];
        if(!(int_na(j) || j > vlen || j <= 0))
        {
            vv[j-1] = arith_conv_boolean_to_ptr(*vr);
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[bools] = r
// recycles r along idx, and idx along v
static void
bool_set_ptr_from_boolean(void *r, void *v, void *idx,
                                     int rlen, int vlen, int ilen)
{
    rboolean_t *vi = idx;
    rboolean_t *istart = vi, *iend = vi + ilen;
    rptr_t *vv = v;
    rboolean_t *vr = r;
    rboolean_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        rboolean_t b = *vi;
        vi = NEXT1(vi, istart, iend);
        if(b == true)
        {
            vv[i] = arith_conv_boolean_to_ptr(*vr);
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[] = r
static void
missing_set_ptr_from_boolean(void *r, void *v, void *idx,
                                        int rlen, int vlen, int ilen)
{
    rptr_t *vv = v;
    rboolean_t *vr = r;
    rboolean_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        vv[i] = arith_conv_boolean_to_ptr(*vr);
        vr = NEXT1(vr, rstart, rend);
    }
}

/*
  
  
  
*/
// 
  
    
  
    
    
  

// v[ints] = r
// recycle r along idx
// idxs which are out of range are ignored
static void
int_set_ptr_from_ptr(void *r, void *v, void *idx,
                                    int rlen, int vlen, int ilen)
{
    rint_t *vi = idx;
    rptr_t *vv = v;
    rptr_t *vr = r;
    rptr_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<ilen; i++)
    {
        rint_t j = vi[i];
        if(!(int_na(j) || j > vlen || j <= 0))
        {
            vv[j-1] = *vr;
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[bools] = r
// recycles r along idx, and idx along v
static void
bool_set_ptr_from_ptr(void *r, void *v, void *idx,
                                     int rlen, int vlen, int ilen)
{
    rboolean_t *vi = idx;
    rboolean_t *istart = vi, *iend = vi + ilen;
    rptr_t *vv = v;
    rptr_t *vr = r;
    rptr_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        rboolean_t b = *vi;
        vi = NEXT1(vi, istart, iend);
        if(b == true)
        {
            vv[i] = *vr;
            vr = NEXT1(vr, rstart, rend);
        }
    }
}

// v[] = r
static void
missing_set_ptr_from_ptr(void *r, void *v, void *idx,
                                        int rlen, int vlen, int ilen)
{
    rptr_t *vv = v;
    rptr_t *vr = r;
    rptr_t *rstart = vr, *rend = vr + rlen;

    for(int i=0; i<vlen; i++)
    {
        vv[i] = *vr;
        vr = NEXT1(vr, rstart, rend);
    }
}

/*
  
  
  
*/
// 
  
    
  

  

  

  

  

  
    
  

  

  

  

  



const idx_fn int_set_funcs[CODE_MAX][CODE_MAX] = {
    //
  [CODE_double][CODE_double] = int_set_double_from_double,
  
  [CODE_double][CODE_int] = int_set_double_from_int,
  
  [CODE_double][CODE_boolean] = int_set_double_from_boolean,
  
  [CODE_double][CODE_ptr] = int_set_double_from_ptr,
  
  [CODE_int][CODE_double] = int_set_int_from_double,
  
  [CODE_int][CODE_int] = int_set_int_from_int,
  
  [CODE_int][CODE_boolean] = int_set_int_from_boolean,
  
  [CODE_int][CODE_ptr] = int_set_int_from_ptr,
  
  [CODE_boolean][CODE_double] = int_set_boolean_from_double,
  
  [CODE_boolean][CODE_int] = int_set_boolean_from_int,
  
  [CODE_boolean][CODE_boolean] = int_set_boolean_from_boolean,
  
  [CODE_boolean][CODE_ptr] = int_set_boolean_from_ptr,
  
  [CODE_ptr][CODE_double] = int_set_ptr_from_double,
  
  [CODE_ptr][CODE_int] = int_set_ptr_from_int,
  
  [CODE_ptr][CODE_boolean] = int_set_ptr_from_boolean,
  
  [CODE_ptr][CODE_ptr] = int_set_ptr_from_ptr,
  
};
const idx_fn bool_set_funcs[CODE_MAX][CODE_MAX] = {
    //
  [CODE_double][CODE_double] = bool_set_double_from_double,
  
  [CODE_double][CODE_int] = bool_set_double_from_int,
  
  [CODE_double][CODE_boolean] = bool_set_double_from_boolean,
  
  [CODE_double][CODE_ptr] = bool_set_double_from_ptr,
  
  [CODE_int][CODE_double] = bool_set_int_from_double,
  
  [CODE_int][CODE_int] = bool_set_int_from_int,
  
  [CODE_int][CODE_boolean] = bool_set_int_from_boolean,
  
  [CODE_int][CODE_ptr] = bool_set_int_from_ptr,
  
  [CODE_boolean][CODE_double] = bool_set_boolean_from_double,
  
  [CODE_boolean][CODE_int] = bool_set_boolean_from_int,
  
  [CODE_boolean][CODE_boolean] = bool_set_boolean_from_boolean,
  
  [CODE_boolean][CODE_ptr] = bool_set_boolean_from_ptr,
  
  [CODE_ptr][CODE_double] = bool_set_ptr_from_double,
  
  [CODE_ptr][CODE_int] = bool_set_ptr_from_int,
  
  [CODE_ptr][CODE_boolean] = bool_set_ptr_from_boolean,
  
  [CODE_ptr][CODE_ptr] = bool_set_ptr_from_ptr,
  
};
const idx_fn missing_set_funcs[CODE_MAX][CODE_MAX] = {
    //
  [CODE_double][CODE_double] = missing_set_double_from_double,
  
  [CODE_double][CODE_int] = missing_set_double_from_int,
  
  [CODE_double][CODE_boolean] = missing_set_double_from_boolean,
  
  [CODE_double][CODE_ptr] = missing_set_double_from_ptr,
  
  [CODE_int][CODE_double] = missing_set_int_from_double,
  
  [CODE_int][CODE_int] = missing_set_int_from_int,
  
  [CODE_int][CODE_boolean] = missing_set_int_from_boolean,
  
  [CODE_int][CODE_ptr] = missing_set_int_from_ptr,
  
  [CODE_boolean][CODE_double] = missing_set_boolean_from_double,
  
  [CODE_boolean][CODE_int] = missing_set_boolean_from_int,
  
  [CODE_boolean][CODE_boolean] = missing_set_boolean_from_boolean,
  
  [CODE_boolean][CODE_ptr] = missing_set_boolean_from_ptr,
  
  [CODE_ptr][CODE_double] = missing_set_ptr_from_double,
  
  [CODE_ptr][CODE_int] = missing_set_ptr_from_int,
  
  [CODE_ptr][CODE_boolean] = missing_set_ptr_from_boolean,
  
  [CODE_ptr][CODE_ptr] = missing_set_ptr_from_ptr,
  
};
// 
  
    
    
static void fill_na_double(void *r, size_t sz)
{
    rdouble_t *vr = r;
    int len = sz / sizeof(*vr);
    for(int i=0; i < len; i++)
        vr[i] = rdouble_na;
}

// ... = v[ints]
// r is sized correctly
static void int_get_double(void *r, void *v, void *idx,
                                   int rlen, int vlen, int ilen)
{
    rint_t *vi = idx;
    rdouble_t *vv = v;
    rdouble_t *vr = r;

    // for each element in the index
    for(int i=0; i < ilen; i++)
    {
        rint_t j = vi[i];
        // if the index element is out of range or NA, the result is NA,
        // else it's the indexed element from the vector
        if(int_na(j) || j > vlen || j <= 0)
            vr[i] = rdouble_na;
        else
            vr[i] = vv[j-1];
    }
}

// ... = v[bools]
// recycle idx along v
// r is sized correctly, but may need padding with NAs if v is shorter than idx
static void bool_get_double(void *r, void *v, void *idx,
                                    int rlen, int vlen, int ilen)
{
    rboolean_t *vi = idx;
    rboolean_t *istart = vi, *iend = vi + ilen;
    rdouble_t *vv = v;
    rdouble_t *vr = r;

    for(int i=0; i<vlen; i++)
    {
        rboolean_t b = *vi;
        vi = NEXT1(vi, istart, iend);
        // if it's not false
        if(b != false)
        {
            // set the current result element - to NA, if the index elt is;
            // or to the corresponding vector element
            *vr++ = boolean_na(b) ? rdouble_na : vv[i];
        }
    }

    // if the index is longer than the vector, fill the rest of the result with NAs
    if(vlen < ilen)
    {
        uint8_t *end = (uint8_t *)((rdouble_t *)r + rlen);
        fill_na_double(vr, end - (uint8_t *)vr);
    }
}

// ... = v[]
static void missing_get_double(void *r, void *v, void *idx,
                                       int rlen, int vlen, int ilen)
{
    rdouble_t *vv = v;
    rdouble_t *vr = r;

    for(int i=0; i<vlen; i++)
        vr[i] = vv[i];
}

/*
  
  
  
  
*/
// 
    
  
    
    
static void fill_na_int(void *r, size_t sz)
{
    rint_t *vr = r;
    int len = sz / sizeof(*vr);
    for(int i=0; i < len; i++)
        vr[i] = rint_na;
}

// ... = v[ints]
// r is sized correctly
static void int_get_int(void *r, void *v, void *idx,
                                   int rlen, int vlen, int ilen)
{
    rint_t *vi = idx;
    rint_t *vv = v;
    rint_t *vr = r;

    // for each element in the index
    for(int i=0; i < ilen; i++)
    {
        rint_t j = vi[i];
        // if the index element is out of range or NA, the result is NA,
        // else it's the indexed element from the vector
        if(int_na(j) || j > vlen || j <= 0)
            vr[i] = rint_na;
        else
            vr[i] = vv[j-1];
    }
}

// ... = v[bools]
// recycle idx along v
// r is sized correctly, but may need padding with NAs if v is shorter than idx
static void bool_get_int(void *r, void *v, void *idx,
                                    int rlen, int vlen, int ilen)
{
    rboolean_t *vi = idx;
    rboolean_t *istart = vi, *iend = vi + ilen;
    rint_t *vv = v;
    rint_t *vr = r;

    for(int i=0; i<vlen; i++)
    {
        rboolean_t b = *vi;
        vi = NEXT1(vi, istart, iend);
        // if it's not false
        if(b != false)
        {
            // set the current result element - to NA, if the index elt is;
            // or to the corresponding vector element
            *vr++ = boolean_na(b) ? rint_na : vv[i];
        }
    }

    // if the index is longer than the vector, fill the rest of the result with NAs
    if(vlen < ilen)
    {
        uint8_t *end = (uint8_t *)((rint_t *)r + rlen);
        fill_na_int(vr, end - (uint8_t *)vr);
    }
}

// ... = v[]
static void missing_get_int(void *r, void *v, void *idx,
                                       int rlen, int vlen, int ilen)
{
    rint_t *vv = v;
    rint_t *vr = r;

    for(int i=0; i<vlen; i++)
        vr[i] = vv[i];
}

/*
  
  
  
  
*/
// 
    
  
    
    
static void fill_na_boolean(void *r, size_t sz)
{
    rboolean_t *vr = r;
    int len = sz / sizeof(*vr);
    for(int i=0; i < len; i++)
        vr[i] = rboolean_na;
}

// ... = v[ints]
// r is sized correctly
static void int_get_boolean(void *r, void *v, void *idx,
                                   int rlen, int vlen, int ilen)
{
    rint_t *vi = idx;
    rboolean_t *vv = v;
    rboolean_t *vr = r;

    // for each element in the index
    for(int i=0; i < ilen; i++)
    {
        rint_t j = vi[i];
        // if the index element is out of range or NA, the result is NA,
        // else it's the indexed element from the vector
        if(int_na(j) || j > vlen || j <= 0)
            vr[i] = rboolean_na;
        else
            vr[i] = vv[j-1];
    }
}

// ... = v[bools]
// recycle idx along v
// r is sized correctly, but may need padding with NAs if v is shorter than idx
static void bool_get_boolean(void *r, void *v, void *idx,
                                    int rlen, int vlen, int ilen)
{
    rboolean_t *vi = idx;
    rboolean_t *istart = vi, *iend = vi + ilen;
    rboolean_t *vv = v;
    rboolean_t *vr = r;

    for(int i=0; i<vlen; i++)
    {
        rboolean_t b = *vi;
        vi = NEXT1(vi, istart, iend);
        // if it's not false
        if(b != false)
        {
            // set the current result element - to NA, if the index elt is;
            // or to the corresponding vector element
            *vr++ = boolean_na(b) ? rboolean_na : vv[i];
        }
    }

    // if the index is longer than the vector, fill the rest of the result with NAs
    if(vlen < ilen)
    {
        uint8_t *end = (uint8_t *)((rboolean_t *)r + rlen);
        fill_na_boolean(vr, end - (uint8_t *)vr);
    }
}

// ... = v[]
static void missing_get_boolean(void *r, void *v, void *idx,
                                       int rlen, int vlen, int ilen)
{
    rboolean_t *vv = v;
    rboolean_t *vr = r;

    for(int i=0; i<vlen; i++)
        vr[i] = vv[i];
}

/*
  
  
  
  
*/
// 
    
  
    
    
static void fill_na_ptr(void *r, size_t sz)
{
    rptr_t *vr = r;
    int len = sz / sizeof(*vr);
    for(int i=0; i < len; i++)
        vr[i] = rptr_na;
}

// ... = v[ints]
// r is sized correctly
static void int_get_ptr(void *r, void *v, void *idx,
                                   int rlen, int vlen, int ilen)
{
    rint_t *vi = idx;
    rptr_t *vv = v;
    rptr_t *vr = r;

    // for each element in the index
    for(int i=0; i < ilen; i++)
    {
        rint_t j = vi[i];
        // if the index element is out of range or NA, the result is NA,
        // else it's the indexed element from the vector
        if(int_na(j) || j > vlen || j <= 0)
            vr[i] = rptr_na;
        else
            vr[i] = vv[j-1];
    }
}

// ... = v[bools]
// recycle idx along v
// r is sized correctly, but may need padding with NAs if v is shorter than idx
static void bool_get_ptr(void *r, void *v, void *idx,
                                    int rlen, int vlen, int ilen)
{
    rboolean_t *vi = idx;
    rboolean_t *istart = vi, *iend = vi + ilen;
    rptr_t *vv = v;
    rptr_t *vr = r;

    for(int i=0; i<vlen; i++)
    {
        rboolean_t b = *vi;
        vi = NEXT1(vi, istart, iend);
        // if it's not false
        if(b != false)
        {
            // set the current result element - to NA, if the index elt is;
            // or to the corresponding vector element
            *vr++ = boolean_na(b) ? rptr_na : vv[i];
        }
    }

    // if the index is longer than the vector, fill the rest of the result with NAs
    if(vlen < ilen)
    {
        uint8_t *end = (uint8_t *)((rptr_t *)r + rlen);
        fill_na_ptr(vr, end - (uint8_t *)vr);
    }
}

// ... = v[]
static void missing_get_ptr(void *r, void *v, void *idx,
                                       int rlen, int vlen, int ilen)
{
    rptr_t *vv = v;
    rptr_t *vr = r;

    for(int i=0; i<vlen; i++)
        vr[i] = vv[i];
}

/*
  
  
  
  
*/
// 
    
  

  

  

  

  


const idx_fn int_get_funcs[CODE_MAX] = {
    //
  [CODE_double] = int_get_double,
  
  [CODE_int] = int_get_int,
  
  [CODE_boolean] = int_get_boolean,
  
  [CODE_ptr] = int_get_ptr,
  
};
const idx_fn bool_get_funcs[CODE_MAX] = {
    //
  [CODE_double] = bool_get_double,
  
  [CODE_int] = bool_get_int,
  
  [CODE_boolean] = bool_get_boolean,
  
  [CODE_ptr] = bool_get_ptr,
  
};
const idx_fn missing_get_funcs[CODE_MAX] = {
    //
  [CODE_double] = missing_get_double,
  
  [CODE_int] = missing_get_int,
  
  [CODE_boolean] = missing_get_boolean,
  
  [CODE_ptr] = missing_get_ptr,
  
};
const fill_na_fn fill_na_funcs[CODE_MAX] = {
    //
  [CODE_double] = fill_na_double,
  
  [CODE_int] = fill_na_int,
  
  [CODE_boolean] = fill_na_boolean,
  
  [CODE_ptr] = fill_na_ptr,
  
};
//

// helpers
// FIXME use rvec_len/rvec_elts now they exist
static inline int arith_length(robject_t *obj)
{
    if(!obj)
        return 0;
    if(!rtype_is_container(r_typeof(obj)))
        return 1;
    return ((rbuf_t *)obj)->length;
}

static inline void *arith_storage(robject_t *obj)
{
    if(!obj)
        return NULL;
    if(rtype_is_scalar(r_typeof(obj)))
        return BOXPTR(obj);
    assert(rtype_is_container(r_typeof(obj)));
    return ((rbuf_t *)obj)->elts;
}

// FIXME some generalised new() kind of thing, plx
static inline robject_t *arith_create(rtype_t *typ, int len)
{
    if(rtype_is_scalar(typ))
        return r_box_create(typ);
    else if(rtype_is_vector(typ))
        return (robject_t *)rvec_create(typ, len);
    else if(rtype_is_array(typ))
        return (robject_t *)rarr_create_buf(typ, len);
    assert(!"can't arith_create that type");
    return NULL;
}

// dest and src are the same (container) type
// may allocate; retain unrooted inputs.
// do not invoke before initialising freshly allocated vectors of reference type
// despite the name, will share .names/.dimnames pointer if at all possible
void copy_names(robject_t *dest, robject_t *src)
{
    rtype_t *dtype = r_typeof(dest);
    if(rtype_is_vector(dtype))
    {
        rvector_t *dvec = (rvector_t *)dest, *svec = (rvector_t *)src;

        if(rvec_len(dvec) == rvec_len(svec))
        {
            // copy the reference if they're the same length
            dvec->names = svec->names;
        }
        else
        {
            rvec_add_names(dvec);
            copy_recycle(rvec_elts(dvec->names), rvec_elts(svec->names),
                         rvec_len(dvec), rvec_len(svec),
                         sizeof(rsymbol_t *));
        }
    }
    else if(rtype_is_array(dtype))
    {
        rarray_t *darr = (rarray_t *)dest, *sarr = (rarray_t *)src;
        assert(rarr_conform(darr, sarr));
        // arrays always conform, so just copy the reference
        darr->dimnames = sarr->dimnames;
    }
}

static inline void copy_shape(robject_t *robj, robject_t *obj)
{
    rarray_t *arr = (rarray_t *)obj;
    rarr_set_shape((rarray_t *)robj, arr->rank, arr->shape);
}

bool check_binary(binop_chk_t *chk, rtype_t *xt, rtype_t *yt, bool pred)
{
    *chk = (binop_chk_t) {
        .xvec = rtype_is_container(xt), .yvec = rtype_is_container(yt),
        .xarr = rtype_is_array(xt), .yarr = rtype_is_array(yt)
    };

    // take the element type if arg is a container
    rtype_t *xet = chk->xvec ? xt->elt : xt,
            *yet = chk->yvec ? yt->elt : yt;

    // only work on scalars
    if(!rtype_is_scalar(xet) || !rtype_is_scalar(yet))
        return false;

    // dispatch at the higher of the arguments
    chk->optyp = rscal_promote(xet, yet);

    // result element type is dispatch type, or boolean if op is a predicate
    rtype_t *etyp = pred ? r_type_boolean : chk->optyp;

    // result is an array if either arg is, else a vector if either arg is, else a scalar
    if(chk->xarr || chk->yarr)
        chk->typ = rarr_type_create(etyp);
    else if(chk->xvec || chk->yvec)
        chk->typ = rvec_type_create(etyp);
    else
        chk->typ = etyp;

    // scalar attributes cached for later use
    chk->xsz = rscal_size(xet);
    chk->ysz = rscal_size(yet);
    chk->rsz = rscal_size(etyp);
    chk->xcode = rscal_code(xet);
    chk->ycode = rscal_code(yet);
    return true;
}

// XXX ancillae, surely?
static void copy_ancilla_binary(binop_chk_t *chk, robject_t *robj, robject_t *xobj,
                                robject_t *yobj, bool y_long)
{
    bool xn = is_named(xobj), yn = is_named(yobj);

    // if an array is involved
    if(chk->xarr || chk->yarr)
    {
        // take its shape
        if(chk->xarr)
            copy_shape(robj, xobj);
        else if(chk->yarr)
            copy_shape(robj, yobj);
        // take dimnames if present
        if(chk->xarr && xn)
            copy_names(robj, xobj);
        else if(chk->yarr && yn)
            copy_names(robj, yobj);
    }
    // neither x nor y are arrays
    else if(chk->xvec && xn)
    {
        // x is a named vector
        // if y is a named vector longer than x, use its names
        if(chk->yvec && yn && y_long)
            copy_names(robj, yobj);
        else // else use x's
            copy_names(robj, xobj);
    }
    // x is not a named vector, and y is
    else if(chk->yvec && yn)
        copy_names(robj, yobj);
}

robject_t *arith_binary(vm_ctx_t *vm, binop_fntab fntab, bool pred,
                        robject_t *xobj, robject_t *yobj)
{
    rtype_t *xt = r_typeof(xobj), *yt = r_typeof(yobj);
    binop_chk_t chk = { 0 };

    if(!check_binary(&chk, xt, yt, pred))
        vm_error(vm, "arith_binary", "incompatible argument types.");

    rtype_t *rt = vm_retain(vm, chk.typ);
    binop_fn fn = dispatch_binary(&chk, fntab);

    if(!fn)
        vm_error(vm, "arith_binary", "operation not supported.");
    if(chk.xarr & chk.yarr && !rarr_conform((rarray_t *)xobj, (rarray_t *)yobj))
        vm_error(vm, "arith_binary", "non-conforming arrays.");

    int xlen = arith_length(xobj), ylen = arith_length(yobj);
    int rlen = (xlen == 0 || ylen == 0) ? 0 : max(xlen, ylen);
    robject_t *robj = vm_retain(vm, arith_create(rt, rlen));
    void *x = arith_storage(xobj), *y = arith_storage(yobj),
         *r = arith_storage(robj);

    // copy shape and dim/names
    copy_ancilla_binary(&chk, robj, xobj, yobj, ylen > xlen);
    // if either are length 0, the result is length 0, an R-ism
    if(chk.xvec && chk.yvec && rlen > 0)
    {
        bool xlong = xlen >= ylen;
        int roundlen = xlong ? ylen : xlen;
        int rounds = rlen / roundlen;
        int extra = rlen - rounds * roundlen;

        ptrdiff_t xadv = xlong ? (chk.xsz * ylen) : 0;
        ptrdiff_t yadv = xlong ? 0 : (chk.ysz * xlen);
        ptrdiff_t radv = xlong ? (chk.rsz * ylen) : (chk.rsz * xlen);

        for(int n=0; n < rounds; n++)
        {
            fn(r, x, y, roundlen);
            r = (uint8_t *)r + radv;
            x = (uint8_t *)x + xadv;
            y = (uint8_t *)y + yadv;
        }
        if(extra > 0)
        {
            fn(r, x, y, extra);
            // vm_warning(vm, "non-integral recycling");
        }
    }
    else // since rlen is correct, and arith_storage does the right thing
        fn(r, x, y, rlen);
    vm_release(vm, 2);
    return robj;
}

bool check_unary(unop_chk_t *chk, rtype_t *xt, bool pred)
{
    *chk = (unop_chk_t) {
        .vec = rtype_is_container(xt),
        .arr = rtype_is_array(xt),
    };
    rtype_t *xet = chk->vec ? xt->elt : xt;

    if(!rtype_is_scalar(xet))
        return false;

    rtype_t *etyp = pred ? r_type_boolean : xet;

    if(chk->arr)
        chk->typ = rarr_type_create(etyp);
    else if(chk->vec)
        chk->typ = rvec_type_create(etyp);
    else
        chk->typ = etyp;
    chk->code = rscal_code(xet);
    return true;
}

static void copy_ancilla_unary(bool arr, robject_t *robj, robject_t *xobj)
{
    if(arr)
        copy_shape(robj, xobj);
    if(is_named(xobj))
        copy_names(robj, xobj);
}

robject_t *arith_unary(vm_ctx_t *vm, unop_fntab fntab, bool pred,
                       robject_t *xobj)
{
    rtype_t *xt = r_typeof(xobj);
    unop_chk_t chk = { 0 };

    if(!check_unary(&chk, xt, pred))
        vm_error(vm, "arith_unary", "incompatible argument type.");

    rtype_t *rt = vm_retain(vm, chk.typ);
    unop_fn fn = dispatch_unary(&chk, fntab);

    if(!fn)
        vm_error(vm, "arith_unary", "operation not supported.");

    int len = arith_length(xobj);
    robject_t *robj = vm_retain(vm, arith_create(rt, len));
    void *x = arith_storage(xobj), *r = arith_storage(robj);

    copy_ancilla_unary(chk.arr, robj, xobj);
    fn(r, x, len);
    vm_release(vm, 2);
    return robj;
}


bool check_reduce(reduce_chk_t *chk, rtype_t *xt)
{
    *chk = (reduce_chk_t) {
        .vec = rtype_is_container(xt),
    };
    rtype_t *xet = chk->vec ? xt->elt : xt;

    if(!rtype_is_scalar(xet))
        return false;
    chk->typ = xet;
    chk->code = rscal_code(xet);
    return true;
}

robject_t *arith_reduce(vm_ctx_t *vm, reduce_fntab fntab, robject_t *xobj)
{
    rtype_t *xt = r_typeof(xobj);
    reduce_chk_t chk = { 0 };

    if(!check_reduce(&chk, xt))
        vm_error(vm, "arith_reduce", "incompatible argument type.");
    if(!chk.vec)
        return xobj;

    int len = arith_length(xobj);
    rtype_t *rt = vm_retain(vm, chk.typ);
    unop_fn fn = dispatch_reduce(&chk, fntab);

    if(!fn)
        vm_error(vm, "arith_reduce", "operation not supported.");

    robject_t *robj = vm_retain(vm, r_box_create(rt));
    void *x = arith_storage(xobj), *r = arith_storage(robj);

    fn(r, x, len);
    vm_release(vm, 2);
    return robj;
}

// convert a vector of objects to a single scalar type
static inline void vec_conv_generic(rtype_t *to, void *dest, void *src, int len)
{
    assert(rtype_is_scalar(to));
    size_t sz = rtype_eltsz(to);
    uint8_t *ptr = dest;
    robject_t **vs = src;
    fill_na_fn na_fn = fill_na_funcs[rscal_code(to)];

    for(int i=0; i < len; i++, ptr+=sz)
    {
        robject_t *elt = vs[i];
        rtype_t *from = r_typeof(elt);

        // if the element is of the correct type already, unbox it
        if(from == to)
            r_unbox(ptr, elt);
        // if it's another scalar, convert it
        else if(rtype_is_scalar(from))
        {
            conv_fn fn = conv_funcs[0][rscal_code(from)][rscal_code(to)];
            fn(ptr, BOXPTR(elt), 1);
        }
        // it's not a value, so NA
        else
            na_fn(ptr, sz);
    }
}

bool check_conv(conv_chk_t *chk, rtype_t *from, rtype_t *to)
{
    *chk = (conv_chk_t) {
        .vec = rtype_is_container(from),
        .arr = rtype_is_array(from),
    };
    assert(rtype_is_scalar(to));
    if(chk->vec)
    {
        chk->rt = chk->arr
            ? rarr_type_create(to)
            : rvec_type_create(to);

        if(rtype_is_scalar(from->elt))
            chk->fromcode = rscal_code(from->elt);
        else
            chk->generic = true;
    }
    else
    {
        chk->rt = to;
        if(!rtype_is_scalar(from))
            return false;
        chk->fromcode = rscal_code(from);
    }
    chk->tocode = rscal_code(to);
    return true;
}

// convert a vector or scalar to have a given scalar (element) type
robject_t *arith_convert(vm_ctx_t *vm, robject_t *obj, rtype_t *to)
{
    rtype_t *typ = r_typeof(obj);
    rtype_t *from = arith_elt_type(typ);
    conv_chk_t chk = { 0 };

    if(!check_conv(&chk, typ, to))
        vm_error(vm, "arith_convert", "invalid argument type.");
    // XXX we might want to copy instead, because:
    // y = as_double(x); y[...] = crap; shouldn't sometimes fill x with crap?
    if(to == from)
        return obj;

    rtype_t *rt = vm_retain(vm, chk.rt);

    if(!rt)
        vm_error(vm, "arith_convert", "invalid argument type.");

    int len = arith_length(obj);
    robject_t *robj = vm_retain(vm, arith_create(rt, len));
    void *x = arith_storage(obj), *r = arith_storage(robj);

    copy_ancilla_unary(chk.arr, robj, obj);

    if(chk.generic)
    {
        vec_conv_generic(to, r, x, len);
    }
    else
    {
        conv_fn fn = dispatch_conv(&chk);

        if(!fn)
            vm_error(vm, "arith_convert", "invalid argument type.");
        fn(r, x, len);
    }
    vm_release(vm, 2);
    return robj;
}

// now used in both compiler and vm
bool scalar_convert(void *dest, robject_t *obj, rtype_t *to, rtype_t *from)
{
    assert(rtype_is_scalar(from));
    assert(rtype_is_scalar(to));
    assert(to != from);

    conv_fn fn = conv_funcs[0][rscal_code(from)][rscal_code(to)];

    if(!fn)
        return false;
    fn(dest, arith_storage(obj), 1);
    return true;
}


static inline int idx_func_code(rtype_t *typ)
{
    return rtype_is_scalar(typ) ? rscal_code(typ) : CODE_ptr;
}

static inline idx_fn idx_access_func(idx_kind kind, int rcode, int vcode)
{
    switch(kind)
    {
    case SINGLE:
    case INDEX:
        return (rcode == -1) ?
            int_get_funcs[vcode] : int_set_funcs[vcode][rcode];
    case MASK:
        return (rcode == -1) ?
            bool_get_funcs[vcode] : bool_set_funcs[vcode][rcode];
    case MISSING:
        return (rcode == -1) ?
            missing_get_funcs[vcode] : missing_set_funcs[vcode][rcode];
    }
    return NULL; /* NOTREACHED */
}

static inline void
idx_copy_names(rsymbol_t **rnames, rsymbol_t **vnames, void *i, idx_kind kind,
               int rlen, int vlen, int ilen)
{
    idx_fn fn = idx_access_func(kind, -1, CODE_ptr);
    fn(rnames, vnames, i, rlen, vlen, ilen);
}

static int count_nonfalse(rboolean_t *elts, int n)
{
    int c = 0;
    for(int i=0; i<n; i++)
        c += (elts[i] != false);
    return c;
}

// given a mask of length ilen, return the length of the vector that
// will result when it's recycled over a vector of length vlen.
// (note that this may be > vlen, per semantics)
static int mask_result_len(int vlen, rboolean_t *mask, int ilen)
{
    if(ilen == 0)
        return 0;
    if(ilen >= vlen)
        return count_nonfalse(mask, ilen);

    int sum = 0;
    for(int i=0; i<vlen; i += ilen)
        sum += count_nonfalse(mask, min(ilen, vlen - i));
    return sum;
}

static inline bool check_index(idx_kind *kind, rtype_t *ityp)
{
    if(ityp == r_type_nil)
        *kind = MISSING;
    else if(arith_elt_type(ityp) == r_type_int)
        *kind = rtype_is_scalar(ityp) ? SINGLE : INDEX;
    else if(arith_elt_type(ityp) == r_type_boolean)
        *kind = MASK;
    else
        return false; // no other valid index types
    return true;
}

bool check_fetch(idx_chk_t *chk, rtype_t *typ, rtype_t *ityp)
{
    if(!rtype_is_vector(typ) && !rtype_is_array(typ))
        return false;
    if(!check_index(&chk->kind, ityp))
        return false;
    if(chk->kind == SINGLE)
    {
        // return a single element
        chk->typ = typ->elt;
        // single element of reference type can be returned directly
        if(!rtype_is_scalar(typ->elt))
            chk->takeptr = true;
    }
    else if(rtype_is_array(typ))
    {
        // indexing into an array with a single vectorised subscript
        // always returns a vector result (this differs from R)
        chk->typ = rvec_type_create(typ->elt);
        // (and never returns element names, since arrays don't have them)
    }
    else
    {
        chk->typ = typ;
        chk->names = true;
    }
    return true;
}

robject_t *vec_fetch(vm_ctx_t *vm, robject_t *val, robject_t *idx)
{
    rtype_t *typ = r_typeof(val), *ityp = r_typeof(idx);
    idx_chk_t chk = { 0 };

    if(!check_fetch(&chk, typ, ityp))
        vm_error(vm, "[", "invalid argument type.");

    rtype_t *rt = vm_retain(vm, chk.typ);
    int vlen = arith_length(val), ilen = arith_length(idx);
    bool names = chk.names & is_named(val);
    int rlen = 0;

    if(chk.kind == MASK)
        rlen = mask_result_len(vlen, arith_storage(idx), ilen);
    else if(chk.kind == MISSING)
        rlen = vlen;
    else
        rlen = ilen;

    robject_t *res = NULL;
    void *r;

    if(chk.takeptr)
        r = &res;
    else
    {
        res = arith_create(rt, rlen);
        r = arith_storage(res);
    }
    vm_retain(vm, res);
    if(rlen > 0)
    {
        idx_fn fn = idx_access_func(chk.kind, -1, idx_func_code(typ->elt));
        void *v = arith_storage(val), *i = arith_storage(idx);

        fn(r, v, i, rlen, vlen, ilen);
        if(names)
        {
            rvector_t *vvec = (rvector_t *)val, *rvec = (rvector_t *)res;

            rvec_add_names(rvec);
            idx_copy_names(rvec_elts(rvec->names), rvec_elts(vvec->names), i,
                           chk.kind, rlen, vlen, ilen);
        }
    }
    vm_release(vm, 2);
    return res;
}

bool check_store(idx_chk_t *chk, rtype_t *typ, rtype_t *ityp, rtype_t *rtyp)
{
    rtype_t *ret, *et;

    if(!rtype_is_vector(typ) && !rtype_is_array(typ))
        return false;
    if(!check_index(&chk->kind, ityp))
        return false;
    // replacement check
    if(rtype_is_vector(rtyp) && chk->kind != SINGLE) // vector replacement?
    {
        // replace with contents: check against element type
        ret = rtyp->elt;
    }
    else // either replacement not a vector, or single value required
    {
        // replace with value
        ret = rtyp;
        // replace with the reference as a value
        if(!rtype_is_scalar(rtyp))
            chk->takeptr = true;
    }
    // container is always a vector
    et = typ->elt;
    // storing into
    if(rtype_is_scalar(et))
    {
        // a vector of scalars
        // FIXME set_xxx_from_ptr is a thing now, maybe use that
        if(!rtype_is_scalar(ret)) // can't accept reference objects
            return false;
    }
    else
    {
        // a vector of objects
        if(!r_subtypep(ret, et)) // can't accept incompatible objects
            return false;
    }
    chk->typ = ret;
    return true;
}

robject_t *vec_store(vm_ctx_t *vm, robject_t *val, robject_t *rpl,
                     robject_t *idx)
{
    rtype_t *typ = r_typeof(val), *ityp = r_typeof(idx), *rtyp = r_typeof(rpl);
    idx_chk_t chk = { 0 };
    void *r;

    if(!check_store(&chk, typ, ityp, rtyp))
        vm_error(vm, "[=", "invalid argument type.");

    // vector[...]
    int vlen = arith_length(val), ilen = arith_length(idx), rlen;

    if(chk.takeptr)
    {
        r = &rpl;
        rlen = 1;
    }
    else
    {
        r = arith_storage(rpl);
        rlen = arith_length(rpl);
    }
    if(rlen > 0 && (ilen > 0 || chk.kind == MISSING))
    {
        idx_fn fn = idx_access_func(chk.kind, idx_func_code(chk.typ),
                                    idx_func_code(typ->elt));
        void *v = arith_storage(val), *i = arith_storage(idx);

        fn(r, v, i, rlen, vlen, ilen);
    }
    return rpl;
}


typedef struct extent_s extent_t;
typedef void (*ext_fn)(extent_t *, void *, void *);
typedef struct extent_s
{
    void *idx;
    int rlen, vlen, ilen;

    ext_fn ext_fn;
    union {
        idx_fn idx_fn;
        fill_na_fn na_fn;
    };
    size_t vsz, rsz;
    extent_t *next;
} extent_t;

static void extent_access(extent_t *ext, void *r, void *v)
{
    ext->idx_fn(r, v, ext->idx, ext->rlen, ext->vlen, ext->ilen);
}

#define NEXT(p, q, start, end) ((p+q >= end) ? start : p+q)

static void extent_bool_get(extent_t *ext, void *r, void *v)
{
    rboolean_t *vi = ext->idx;
    rboolean_t *istart = vi, *iend = vi + ext->ilen;
    uint8_t *vv = v, *vr = r;
    size_t vsz = ext->vsz, rsz = ext->rsz;
    uint8_t *rstart = vr, *rend = vr + ext->rlen * rsz;

    for(int i=0; i<ext->vlen; i++)
    {
        rboolean_t b = *vi;

        vi = NEXT1(vi, istart, iend);
        if(b != false)
        {
            if(boolean_na(b))
                ext->na_fn(vr, rsz);
            else
                ext->next->ext_fn(ext->next, vr, vv);
            vr = NEXT(vr, rsz, rstart, rend);
        }
        vv += vsz;
    }
    if(ext->vlen < ext->ilen)
        ext->na_fn(vr, rend - vr);
}

static void extent_int_get(extent_t *ext, void *r, void *v)
{
    rint_t *vi = ext->idx;
    uint8_t *vv = v, *vr = r;
    size_t vsz = ext->vsz, rsz = ext->rsz;

    for(int i=0; i<ext->ilen; i++)
    {
        rint_t j = vi[i];
        if(int_na(j) || j > ext->vlen || j <= 0)
            ext->na_fn(vr, rsz);
        else
            ext->next->ext_fn(ext->next, vr, vv + (j-1) * vsz);
        vr += rsz;
    }
}

static void extent_missing_get_set(extent_t *ext, void *r, void *v)
{
    uint8_t *vv = v, *vr = r;
    size_t vsz = ext->vsz, rsz = ext->rsz;

    for(int i=0; i<ext->vlen; i++)
    {
        ext->next->ext_fn(ext->next, vr, vv);
        vr += rsz;
        vv += vsz;
    }
}

static void extent_bool_set(extent_t *ext, void *r, void *v)
{
    rboolean_t *vi = ext->idx;
    rboolean_t *istart = vi, *iend = vi + ext->ilen;
    uint8_t *vv = v, *vr = r;
    size_t vsz = ext->vsz, rsz = ext->rsz;

    for(int i=0; i<ext->vlen; i++)
    {
        rboolean_t b = *vi;
        vi = NEXT1(vi, istart, iend);
        if(b == true)
        {
            ext->next->ext_fn(ext->next, vr, vv);
            vr += rsz;
        }
        vv += vsz;
    }
}

static void extent_int_set(extent_t *ext, void *r, void *v)
{
    rint_t *vi = ext->idx;
    uint8_t *vv = v, *vr = r;
    size_t vsz = ext->vsz, rsz = ext->rsz;

    for(int i=0; i<ext->ilen; i++)
    {
        rint_t j = vi[i];
        if(!(int_na(j) || j > ext->vlen || j <= 0))
        {
            ext->next->ext_fn(ext->next, vr, vv + (j-1) * vsz);
            vr += rsz;
        }
    }
}

static ext_fn idx_ext_func(idx_kind kind, bool isset)
{
    switch(kind)
    {
    case SINGLE:
    case INDEX:
        return isset ? extent_int_set : extent_int_get;
    case MASK:
        return isset ? extent_bool_set : extent_bool_get;
    case MISSING:
        return extent_missing_get_set;
    }
    return NULL; /* NOTREACHED */
}

static inline int idx_dim(int vlen, robject_t *idx, idx_kind kind)
{
    switch(kind)
    {
    case SINGLE:
        return 1;
    case INDEX:
        return arith_length(idx);
    case MASK:
        return mask_result_len(vlen, arith_storage(idx), arith_length(idx));
    case MISSING:
        return vlen;
    }
    return 0; /* NOTREACHED */
}

static rtype_t **idx_types(rtype_t **ityps, robject_t *idx,
                           robject_t **idxs, int nidxs)
{
    ityps[0] = r_typeof(idx);
    for(int i=1; i < nidxs+1; i++)
        ityps[i] = r_typeof(idxs[i-1]);
    return ityps;
}

static int idx_extents(extent_t *exts, idx_kind *kinds,
                       robject_t *idx, robject_t **idxs, int nidxs,
                       size_t rsz, size_t vsz,
                       int *rshape, int *vshape,
                       int rcode, int vcode, bool isscal)
{
    extent_t *ext = &exts[0];
    extent_t *next = NULL;
    robject_t **pidx = &idx;
    int rlen = 1;
    int j = 0;
    int rdim;
    bool isset = (rcode != -1);

    for(int i=0; i<nidxs+1; ext++, i++)
    {
        int kind = kinds[i];

        rdim = idx_dim(vshape[i], *pidx, kind);
        if(kind != SINGLE)
            rshape[j++] = rdim;
        rlen *= rdim;

        *ext = (extent_t) {
            // scalar replacement: correct length for recycling in *_set_*_from_*
            .rlen = isscal ? 1 : rdim,
            .vlen = vshape[i],
            .ilen = arith_length(*pidx),
            .idx = arith_storage(*pidx),
        };

        if(i == 0)
        {
            ext->ext_fn = extent_access;
            ext->idx_fn = idx_access_func(kind, rcode, vcode);
        }
        else
        {
            ext->ext_fn = idx_ext_func(kind, isset);
            ext->na_fn = fill_na_funcs[vcode];
            // scalar replacement: do not advance replacement pointer in extent_set_*
            ext->rsz = isscal ? 0 : rsz;
            ext->vsz = vsz;
            ext->next = next;
        }
        rsz *= rdim;
        vsz *= vshape[i];
        next = ext;
        // pidx may get to &idx[nidxs], but the loop ends before it's dereferenced
        pidx = &idxs[i];
    }
    return rlen;
}

// copy dimnames from val (as indexed by exts) to res, if the latter is
// a vector or array
static void idx_extent_names(robject_t *res, robject_t *val, int rank,
                             extent_t *exts, idx_kind *kinds, int nexts,
                             int *rshape, int *vshape)
{
    rarray_t *arr = (rarray_t *)val;

    if(rank == 1)
    {
        // to element names
        rvector_t *rvec = (rvector_t *)res;
        rvec_add_names(rvec);

        // FIXME split into function
        rvector_t **dimnames = rvec_elts(arr->dimnames);

        for(int i=0; i<nexts; i++)
        {
            if(kinds[i] != SINGLE && dimnames[i])
            {
                // there will be exactly one of these
                idx_copy_names(rvec_elts(rvec->names), rvec_elts(dimnames[i]),
                               exts[i].idx, kinds[i], rshape[0], vshape[i],
                               exts[i].ilen);
            }
        }
    }
    else if(rank > 1)
    {
        // to dimnames
        rarray_t *rarr = (rarray_t *)res;
        rarr_add_names(rarr);

        // FIXME split into function
        rvector_t **srcnames = rvec_elts(arr->dimnames);
        rvector_t **destnames = rvec_elts(rarr->dimnames);
        int j = 0;

        for(int i=0; i<nexts; i++)
        {
            if(kinds[i] != SINGLE)
            {
                if(srcnames[i])
                {
                    destnames[j] = rvec_create(r_type_vec_symbol, rshape[j]);
                    idx_copy_names(rvec_elts(destnames[j]), rvec_elts(srcnames[i]),
                                   exts[i].idx, kinds[i], rshape[j], vshape[i],
                                   exts[i].ilen);
                }
                j++;
            }
        }
    }
}

bool check_arr_fetch(arr_chk_t *achk, rtype_t *typ, rtype_t **ityps,
                     int ntyps)
{
    int rank = 0;

    if(!rtype_is_array(typ))
        return false;
    for(int i=0; i<ntyps; i++)
    {
        if(!check_index(&achk->kinds[i], ityps[i]))
            return false;
        if(achk->kinds[i] != SINGLE)
            rank++;
    }
    achk->rank = rank;
    if(rank == 0) // single element
    {
        achk->typ = typ->elt;
        if(!rtype_is_scalar(typ->elt))
            achk->takeptr = true;
    }
    else if(rank == 1) // vector
        achk->typ = rvec_type_create(typ->elt);
    else // array
        achk->typ = typ;
    return true;
}

robject_t *arr_fetch(vm_ctx_t *vm, robject_t *val, robject_t *idx,
                     robject_t **idxs, int nidxs)
{
    rtype_t *typ = r_typeof(val);
    robject_t *res = NULL;
    arr_chk_t achk = { .kinds = alloca((nidxs+1) * sizeof(idx_kind)) };
    rtype_t **ityps = idx_types(alloca((nidxs+1) * sizeof(rtype_t *)),
                                idx, idxs, nidxs);

    if(!check_arr_fetch(&achk, typ, ityps, nidxs+1))
        vm_error(vm, "[", "invalid argument type.");

    rarray_t *arr = (rarray_t *)val;
    int vrank = arr->rank;
    int *vshape = arr->shape;

    if(vrank > nidxs+1)
        vm_error(vm, "[", "too few indices.");
    else if(vrank < nidxs+1)
        vm_error(vm, "[", "too many indices.");

    rtype_t *rtyp = vm_retain(vm, achk.typ);
    int rrank = achk.rank;
    int *rshape = alloca(rrank * sizeof(int));

    extent_t *exts = alloca((nidxs+1) * sizeof(extent_t));
    int rlen = idx_extents(exts, achk.kinds, idx, idxs, nidxs,
                           rtype_eltsz(typ->elt), rtype_eltsz(typ->elt),
                           rshape, vshape,
                           -1, idx_func_code(typ->elt), false);

    void *r = NULL;
    void *v = arith_storage(val);

    if(achk.takeptr)
        r = &res;
    else
    {
        res = arith_create(rtyp, rlen);
        r = arith_storage(res);
        if(rrank > 1)
            rarr_set_shape((rarray_t *)res, rrank, rshape);
    }
    vm_retain(vm, res);
    if(rlen > 0)
    {
        extent_t *ext = &exts[nidxs];
        ext->ext_fn(ext, r, v);
    }
    // copy dimnames
    if(is_named(val))
        idx_extent_names(res, val, rrank, exts, achk.kinds,
                         nidxs+1, rshape, vshape);
    vm_release(vm, 2);
    return res;
}

bool check_arr_store(arr_chk_t *achk, rtype_t *typ, rtype_t **ityps,
                     int ntyps, rtype_t *rtyp)
{
    rtype_t *ret, *et;
    int rank = 0;

    if(!rtype_is_array(typ))
        return false;
    for(int i=0; i<ntyps; i++)
    {
        if(!check_index(&achk->kinds[i], ityps[i]))
            return false;
        if(achk->kinds[i] != SINGLE)
            rank++;
    }
    achk->rank = rank;
    if(rtype_is_container(rtyp) && rank > 0)
    {
        // replace with contents: check against element type
        ret = rtyp->elt;
    }
    else // either replacement not a container, or single value required
    {
        // replace with value
        ret = rtyp;
        // replace with the reference as a value
        if(!rtype_is_scalar(rtyp))
            achk->takeptr = true;
    }
    // container is always an array
    et = typ->elt;
    // element types
    if(rtype_is_scalar(et))
    {
        // can't accept reference objects
        if(!rtype_is_scalar(ret))
            return false;
    }
    else
    {
        // can't accept incompatible objects
        if(!r_subtypep(ret, et))
            return false;
    }
    achk->typ = ret;
    return true;
}

robject_t *arr_store(vm_ctx_t *vm, robject_t *val, robject_t *rpl,
                     robject_t *idx, robject_t **idxs, int nidxs)
{
    rtype_t *typ = r_typeof(val), *rtyp = r_typeof(rpl);
    arr_chk_t achk = { .kinds = alloca((nidxs+1) * sizeof(idx_kind)) };
    rtype_t **ityps = idx_types(alloca((nidxs+1) * sizeof(rtype_t *)),
                                idx, idxs, nidxs);

    if(!check_arr_store(&achk, typ, ityps, nidxs+1, rtyp))
        vm_error(vm, "[=", "invalid argument type.");

    rarray_t *arr = (rarray_t *)val;
    int vrank = arr->rank;
    int *vshape = arr->shape;

    if(vrank > nidxs+1)
        vm_error(vm, "[=", "too few indices.");
    else if(vrank < nidxs+1)
        vm_error(vm, "[=", "too many indices.");

    // 'requirements' that rpl must conform to
    int rrank = achk.rank;
    int *rshape = alloca(rrank * sizeof(int));

    extent_t *exts = alloca((nidxs+1) * sizeof(extent_t));
    int rlen = idx_extents(exts, achk.kinds, idx, idxs, nidxs,
                           rtype_eltsz(achk.typ), rtype_eltsz(typ->elt),
                           rshape, vshape,
                           idx_func_code(achk.typ), idx_func_code(typ->elt),
                           rtype_is_scalar(rtyp));

    if(rtype_is_vector(rtyp))
    {
        rvector_t *rvec = (rvector_t *)rpl;
        if(rvec_len(rvec) < rlen)
            vm_error(vm, "[=", "non-conforming replacement vector.");
    }
    else if(rtype_is_array(rtyp))
    {
        rarray_t *rarr = (rarray_t *)rpl;
        if(!rarr_shape_conform(rrank, rarr->rank, rshape, rarr->shape))
            vm_error(vm, "[=", "non-conforming replacement array.");
    }

    void *r = NULL;
    void *v = arith_storage(val);

    if(achk.takeptr)
        r = &rpl;
    else
        r = arith_storage(rpl);

    if(rlen > 0)
    {
        extent_t *ext = &exts[nidxs];
        ext->ext_fn(ext, r, v);
    }
    return rpl;
}
//
