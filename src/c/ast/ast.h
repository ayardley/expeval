/*
 * ast.h - Header file for the abstract syntax tree.
 */
#ifndef AST_H
#  define AST_H
#endif

#define MAX_LEN  256

#define EOT      257
#define ERR      258
#define ADD      259
#define SUB      260
#define MUL      261
#define DIV      262
#define NUM      263
#define O_PAR    264
#define C_PAR    265

#define UNDEF    266
#define OP_ADD   267
#define OP_SUB   268
#define OP_MUL   269
#define OP_DIV   270
#define U_MIN    271
#define NUM_VAL  272

struct token {
    int    type;
    double value;
    char   symbol;
};

typedef struct Astnode Astnode;
struct Astnode {
    int     type;
    double  value;
    Astnode *left;
    Astnode *right;
};
