typedef enum
{
    AST_INVALID, AST_NODE, AST_TOKEN,
    AST_INT, AST_DOUBLE, AST_STRING,
    AST_QUOTED, AST_SYMBOL, AST_NAME
} asttype;
typedef struct ast ast_t;
typedef ARRAY(ast_t) ast_array_t;
typedef struct ast
{
    asttype type;
    union
    {
        ast_array_t *children;
        int token;
        int integer;
        double dfloat;
        char *string;
        rsymbol_t *symbol;
    };
} ast_t;
static inline bool ast_is_rest(ast_t *ast)
    { return (ast->type == AST_SYMBOL && ast->symbol == r_sym_rest); }
static inline bool ast_is_omitted(ast_t *ast)
    { return ast->type == AST_INVALID; }
void ast_fini(ast_t *ast);
const char *ast_str(ast_t *ast);
int p_source(char *name, ast_t *ast);
int p_readline(char *p1, char *p2, ast_t *result);
