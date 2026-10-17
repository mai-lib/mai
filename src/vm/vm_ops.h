#define _CAR(a, rest...) a
#define CAR(a) _CAR(a)
#define CONV(a, b) PASTE3(a, 2, b)
#define ENUM(name, suffix...) PASTE3(OP_, name, suffix)
#define LABEL(name, suffix...) PASTE3(do_, name, suffix)
#define OPS_GEN                                                         \
        OP_DEF(call_fast), OP_DEF(call_uni), OP_DEF(missing),           \
        OP_DEF(ret), OP_DEF(complete_box), OP_DEF(check),               \
        OP_DEF(jump), OP_DEF(if), OP_DEF(const), OP_DEF(defvar),        \
        OP_DEF(getvar_uni), OP_DEF(setvar_uni), OP_DEF(lambda),         \
        OP_DEF(frame), OP_DEF(geteltptr), OP_DEF(getel2ptr),            \
        OP_DEF(cellmake), OP_DEF(and, boolean), OP_DEF(or, boolean),    \
        OP_DEF(not, boolean)
#define OPS_WIDTH(w)                                                    \
        OP_DEF(mov, w), OP_DEF(literal, w), OP_DEF(getvar, w),          \
        OP_DEF(setvar, w), OP_DEF(env, w), OP_DEF(complete, w),         \
        OP_DEF(setelt, w), OP_DEF(setel2, w), OP_DEF(cellget, w),       \
        OP_DEF(cellset, w)
#define OPS_WIDTH_ALL OPS_WIDTH(8), OPS_WIDTH(32), OPS_WIDTH(64)
#define OPS_WIDTH_GROUP                         \
    enum { OPS_WIDTH() };                       \
    static const op_code_t op_width_ofs[] = {   \
        CAR(OPS_WIDTH(8)),                      \
        CAR(OPS_WIDTH(32)),                     \
        CAR(OPS_WIDTH(64))                      \
    };
#define OPS_GETELT(t)                                                   \
        OP_DEF(getelt, t), OP_DEF(getel2, t)
#define OPS_SCALAR(t)                                                   \
        OP_DEF(box, t), OP_DEF(unbox, t), OP_DEF(eql, t),               \
        OP_DEF(neq, t), OP_DEF(is_na, t)
#define OPS_ARITH(t)                                                    \
        OP_DEF(add, t), OP_DEF(sub, t), OP_DEF(mul, t), OP_DEF(div, t), \
        OP_DEF(neg, t), OP_DEF(pow, t), OP_DEF(lth, t), OP_DEF(lte, t), \
        OP_DEF(gth, t), OP_DEF(gte, t)
#define OPS_TYPE_BOOL                           \
    OPS_SCALAR(boolean), OPS_GETELT(boolean)
#define OPS_TYPE_INT                            \
    OPS_SCALAR(int), OPS_GETELT(int), OPS_ARITH(int)
#define OPS_TYPE_DOUBLE                         \
    OPS_SCALAR(double), OPS_GETELT(double), OPS_ARITH(double)
#define OPS_TYPE_ALL OPS_TYPE_BOOL, OPS_TYPE_INT, OPS_TYPE_DOUBLE
#define OPS_TYPE_GROUP                          \
    enum {                                      \
        OPS_SCALAR(),                           \
        OPS_GETELT(),                           \
        OPS_ARITH()                             \
    };                                          \
    static const op_code_t op_type_ofs[] = {    \
        CAR(OPS_TYPE_BOOL),                     \
        CAR(OPS_TYPE_INT),                      \
        CAR(OPS_TYPE_DOUBLE)                    \
    };
#define OPS_CONVERSION(f)                       \
    f(boolean, int)                             \
    f(boolean, double)                          \
    f(int, boolean)                             \
    f(int, double)                              \
    f(double, boolean)                          \
    f(double, int)
#define _CONV(a, b) OP_DEF(conv, CONV(a, b)),
#define OPS_CONV_ALL OPS_CONVERSION(_CONV)
#define OPS_CONV_GROUP                          \
    static const op_code_t op_conv_ofs = CAR(OPS_CONV_ALL)
#define OPS_ALL                                 \
    OPS_GEN, OPS_WIDTH_ALL, OPS_TYPE_ALL, OPS_CONV_ALL
#define OP_DEF(def...) ENUM(def)
typedef enum
{
    OPS_ALL
} vm_opcode_t;
OPS_TYPE_GROUP;
OPS_WIDTH_GROUP;
OPS_CONV_GROUP;
#undef OP_DEF
enum { OP_call_end, OP_call_kwd, OP_call_pos,
       OP_call_rest, OP_call_omit };
enum { OP_lambda_end, OP_lambda_mov8,
       OP_lambda_mov32, OP_lambda_mov64 };
typedef ARRAY(op_code_t) op_code_array_t;
typedef struct
{
    void *sp, *fp, *ip, *bp;
} vm_act_rec_t;
typedef const void *vm_instr_t;
extern const void *const *vm_instr;
