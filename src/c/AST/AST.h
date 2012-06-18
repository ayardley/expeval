#ifndef AST_H
#  define AST_H 1
#endif

enum TokenType {
    Error,
    Plus,
    Minus,
    Mul,
    Div,
    EndOfText,
    OpenParenthesis,
    ClosedParenthesis,
    Number
};

struct Token {
    TokenType    Type;
    double       Value;
    char         Symbol;

    Token():Type(Error), Value(0), Symbol(0)
    {}
};

enum ASTNodeType {
    Undefined,
    OperatorPlus,
    OperatorMinus,
    OperatorMul,
    OperatorDiv,
    UnaryMinus,
    NumberValue
};

class ASTNode
{
public:
    ASTNodeType Type;
    double      Value;
    ASTNode*    Left;
    ASTNode*    Right;

    ASTNode()
    {
        Type = Undefined;
        Value = 0;
        Left = NULL;
        Right = NULL;
    }

    ~ASTNode()
    {
        delete Left;
        delete Right;
    }
};
