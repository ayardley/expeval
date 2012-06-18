/*
 * evalexp.c - 
 */
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Yeah, yeah, I know; I know .... */
static struct token g_token;
static const char *g_text;
static int g_index;
static int g_error;

/* prototypes */
void    test(const char *);
Astnode *parse(const char *);
double  evaluate(Astnode *);
void    next_token();
double  get_number();
void    match(char *);
Astnode *create_node(int, Astnode *, Astnode *);
Astnode *create_unarynode(Astnode *);
Astnode *create_numbernode(double);
Astnode *expression();
Astnode *expression1();
Astnode *term();
Astnode *term1();
Astnode *factor();

int main(int argc, char *argv[])
{
    test("1+2+3+4");
    test("1*2*3*4");
    test("1-2-3-4");
    test("1/2/3/4");
    test("1*2+3*4");
    test("1+2*3+4");
    test("(1+2)*(3+4)");
    test("1+(2*3)*(4+5)");
    test("1+(2*3)/4+5");
    test("5/(4+3)/2");
    test("1 + 2.5");
    test("125");
    test("-1");
    test("-1+(-2)");
    test("-1+(-2.0)");
    test("   1*2,5");
    test("   1*2.5e2");
    test("M1 + 2.5");
    test("1 + 2&5");
    test("1 * 2.5.6");
    test("1 ** 2.5");
    test("*1 / 2.5");

    return 0;
}

void test(const char *text)
{
    double value;
    Astnode *ast;

    /* Note: Ok, I'll admit it: This is rather poor programming practice.
     *       But, in-the-end, this is just a simple program, designed,
     *       primarily, as a benchmark implementation with which to
     *       test the Winxed implementation, where I will use try/catch
     *       blocks and exceptions.
     */
    g_error = 0; 

    ast = parse(text);
    value = evaluate(ast);
    if (!g_error)
        printf("%s\t%g\n", text, value);

    free(ast); /* Note: Were I doing this for something other than a simple,
                *       benchmark implementation, I would take much more
                *       care with the allocation and destruction of resources.
                */
    return;
}

Astnode *parse(const char *text)
{
    g_text  = text;
    g_index = 0;

    next_token();

    return expression();
}

double evaluate(Astnode *ast)
{
    if (ast->type == NUM_VAL)
        return ast->value;
    else if (ast->type == U_MIN)
            return -evaluate(ast->left);
    else {
        double v1 = evaluate(ast->left);
        double v2 = evaluate(ast->right);
        switch (ast->type) {
        case OP_ADD: return v1 + v2;
        case OP_SUB: return v1 - v2;
        case OP_MUL: return v1 * v2;
        case OP_DIV: return v1 / v2;
        }
    }
}

void next_token()
{
    while (isspace(g_text[g_index])) g_index++;

    g_token.value = 0;
    g_token.symbol = 0;

    /* Test for end of text */
    if (g_text[g_index] == 0) {
        g_token.type = EOT;
        return;
    }

    /* If current character is a digit, then we're reading a number */
    if (isdigit(g_text[g_index])) {
        g_token.type = NUM;
        g_token.value = get_number();
        return;
    }

    /* Setting to error is rather standard fair */
    g_token.type = ERR;

    /* Test if the current character is an operator or a paren */
    switch (g_text[g_index]) {
    case '+' : g_token.type = ADD;   break;
    case '-' : g_token.type = SUB;   break;
    case '*' : g_token.type = MUL;   break;
    case '/' : g_token.type = DIV;   break;
    case '(' : g_token.type = O_PAR; break;
    case ')' : g_token.type = C_PAR; break;
    }

    if (g_token.type != ERR) {
        g_token.symbol = g_text[g_index];
        g_index++;
    }
    else {
        g_error = 1; /* throw error switch */
        fprintf(stderr, "Unexpected token '%s' at position %d\n",
                g_text, g_index);
    }

    return;
}

double get_number()
{
    while (isspace(g_text[g_index])) g_index++;

    int index = g_index;
    while (isdigit(g_text[g_index])) g_index++;
    if (g_text[g_index] == '.') g_index++;
    while (isdigit(g_text[g_index])) g_index++;

    if (g_index - index == 0) {
        g_error = 1; /* throw error switch */
        fprintf(stderr, "Unexpected input at %d", g_index - index);
    }

    char buffer[32] = {0};
    memcpy(buffer, &g_text[index], g_index - index);
   
    return atof(buffer);
}

void match(char *expected)
{
    if (g_text[g_index-1] == (int)expected[0])
        next_token();
    else {
        g_error = 1; /* throw error switch */
        fprintf(stderr, "Expected token '%s' at position %d\n",
                expected, g_index);
    }

    return;
}

Astnode *create_node(int type, Astnode *left, Astnode *right)
{
    Astnode *node = (Astnode *)malloc(sizeof(struct Astnode));
    node->type  = type;
    node->left  = left;
    node->right = right;

    return node;
}

Astnode *create_unarynode(Astnode *left)
{
    Astnode *node = (Astnode *)malloc(sizeof(struct Astnode));
    node->type  = U_MIN;
    node->left  = left;
    node->right = NULL;

    return node;
}

Astnode *create_numbernode(double value)
{
    Astnode *node = (Astnode *)malloc(sizeof(struct Astnode));
    node->type  = NUM_VAL;
    node->value = value;
    node->left  = NULL; /* Unnecessary, but .... */
    node->right = NULL; /*           "           */

    return node;
}

Astnode *expression()
{
    Astnode *tnode  = term();
    Astnode *e1node = expression1();

    return create_node(OP_ADD, tnode, e1node);
}

Astnode *expression1()
{
    Astnode *tnode;
    Astnode *e1node;

    switch (g_token.type) {
    case ADD:
        next_token();
        tnode  = term();
        e1node = expression1();

        return create_node(OP_ADD, e1node, tnode);
    case SUB:
        next_token();
        tnode  = term();
        e1node = expression1();

        return create_node(OP_SUB, e1node, tnode);
    }

    return create_numbernode(0);
}

Astnode *term()
{
    Astnode *fnode  = factor();
    Astnode *t1node = term1();

    return create_node(OP_MUL, fnode, t1node);
}

Astnode *term1()
{
    Astnode *fnode;
    Astnode *t1node;

    switch (g_token.type) {
    case MUL:
        next_token();
        fnode  = factor();
        t1node = term1();

        return create_node(OP_MUL, t1node, fnode);
    case DIV:
        next_token();
        fnode  = factor();
        t1node = term1();

        return create_node(OP_DIV, t1node, fnode);
    }

    return create_numbernode(1);
}

Astnode *factor()
{
    double  value;
    Astnode *node;

    switch (g_token.type) {
    case O_PAR:
        next_token();
        node = expression();
        match(")");

        return node;
    case SUB:
        next_token();
        node = factor();

        return create_unarynode(node);
    case NUM:
        value = g_token.value;
        next_token();

        return create_numbernode(value);
    default:
        fprintf(stderr, "Unexpected token '%c' at position %d\n",
                g_token.symbol, g_index);
        g_error = 1; /* throw error switch */
        break;
    }
}
