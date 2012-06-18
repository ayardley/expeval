/*
 * parser.c - An implementation of a simple expression evaluator.
 *
 * Note: Basically, the plan is to use this example with which to write a
 *       similar, simple expression evaluator in the Winxed Programming 
 *       Language. This will, then, form the basis for an article on how
 *       to write a simple expression evaluator in Winxed for the Parrot VM.
 *
 * Subnote: One major reason for this -- other than the hope it will prove
 *          useful to others who want to use Winxed similarly -- is, I *have*
 *          to write Chapter 02 of the VM Specification for Parrot, which,
 *          in-the-main, is about the Winxed Programming Language on the 
 *          Parrot Virtual Machine. Given this, it would be a "good-thing"
 *          to better understand both the language and the platform!
 *
 * Note: The below grammar is the what we *want* to write, because it's simple
 *       and intuitive.  Unfortunately, it is also left recursive -- note the
 *       'EXP -> EXP' -- and, therefore, will not work.
 *
 * EXP    -> EXP + TERM |
 *           EXP - TERM |
 *           TERM
 * TERM   -> TERM * FACTOR |
 *           TERM / FACTOR |
 *           FACTOR
 * FACTOR -> ( EXP ) | - EXP | number
 *
 * Consequently, we have to go with something like the below, less
 * aesthetically pleasing, but, clearly more functional, approach:
 *
 * EXP    -> TERM EXP1
 * EXP1   -> + TERM EXP1 |
 *           - TERM EXP1 |
 *           epsilon
 * TERM   -> FACTOR TERM1
 * TERM1  -> * FACTOR TERM1 |
 *           / FACTOR TERM1 |
 *           epsilon
 * FACTOR -> ( EXP ) | - EXP | number
 *
 * epsilon here means nothing or the empty string.
 *
 * Note: How the above grammar eliminates the left recursive problem.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN  256

#define EOT      257
#define ERROR    258
#define ADD      259
#define SUBTRACT 260
#define MULTIPLY 261
#define DIVIDE   262
#define NUMBER   263
#define O_PAREN  264
#define C_PAREN  265

struct token {
    int    type;
    double value;
    char   symbol;
};

static struct token g_token;
static const char *g_text;
static int g_index;
static int g_error;

void test(const char *);
void parse(const char *);
void next_token();
double get_number();
void match(char *);
void expression();
void expression1();
void term();
void term1();
void factor();

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
    g_error = 0; /* Note: This is a rather poor programming practice,
                          but, in-the-end, this is just a simple program
                          with which to test the correctness of the parser. */
    parse(text);
    if (!g_error)
        printf("%s\n", text);

    return;
}

void parse(const char *text)
{
    g_text  = text;
    g_index = 0;

    next_token(); 
    expression();

    return;
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
        g_token.type = NUMBER;
        g_token.value = get_number();
        return;
    }

    /* Note: I'm not sure I like doing it this way, but ... better safe
             than sorry, as the ol' sayin' goes. */
    g_token.type = ERROR;

    /* Test if the current character is an operator or a paren */
    switch (g_text[g_index]) {
    case '+' : g_token.type = ADD;      break;
    case '-' : g_token.type = SUBTRACT; break;
    case '*' : g_token.type = MULTIPLY; break;
    case '/' : g_token.type = DIVIDE;   break;
    case '(' : g_token.type = O_PAREN;  break;
    case ')' : g_token.type = C_PAREN;  break;
    }

    if (g_token.type != ERROR) {
        g_token.symbol = g_text[g_index];
        g_index++;
    }
    else {
        fprintf(stderr, "Unexpected token '%s' at position %d\n",
                g_text, g_index);
        g_error = 1; /* throw error switch */
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
        fprintf(stderr, "Unexpected input at %d", g_index - index);
        g_error = 1; /* throw error switch */
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
        fprintf(stderr, "Expected token '%s' at position %d\n",
                expected, g_index);
        g_error = 1; /* throw error switch */
    }

    return;
}

void expression()
{
    term();
    expression1();

    return;
}

void expression1()
{
    switch (g_token.type) {
    case ADD:
        next_token();
        term();
        expression1();
        break;
    case SUBTRACT:
        next_token();
        term();
        expression1();
        break;
    }

    return;
}

void term()
{
    factor();
    term1();

    return;
}

void term1()
{
    switch (g_token.type) {
    case MULTIPLY:
        next_token();
        factor();
        term1();
        break;
    case DIVIDE:
        next_token();
        factor();
        term1();
        break;
    }

    return;
}

void factor()
{
    switch (g_token.type) {
    case O_PAREN:
        next_token();
        expression();
        match(")");
        break;
    case SUBTRACT:
        next_token();
        factor();
        break;
    case NUMBER:
        next_token();
        break;
    default:
        fprintf(stderr, "Unexpected token '%c' at position %d\n",
                g_token.symbol, g_index);
        g_error = 1; /* throw error switch */
        break;
    }

    return;
}
