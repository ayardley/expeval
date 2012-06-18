#ifndef AST_H
#  include "AST.h"
#endif

class EvaluatorException : public std::runtime_error
{
public:
EvaluatorException(const std::string& message):
    std::runtime_error(message.c_str())
    {
    }
};

class Evaluator 
{
    double EvaluateSubtree(ASTNode* ast)
    {
        if(ast == NULL) 
            throw EvaluatorException("Incorrect syntax tree!");

        if(ast->Type == NumberValue)
            return ast->Value;
        else if(ast->Type == UnaryMinus)
            return -EvaluateSubtree(ast->Left);
        else 
        {
            double v1 = EvaluateSubtree(ast->Left);
            double v2 = EvaluateSubtree(ast->Right);
            switch(ast->Type) {
            case OperatorPlus:  return v1 + v2;
            case OperatorMinus: return v1 - v2;
            case OperatorMul:   return v1 * v2;
            case OperatorDiv:   return v1 / v2;
            }
        }

        throw EvaluatorException("Incorrect syntax tree!");
    }

public:
    double Evaluate(ASTNode* ast)
    {
        if(ast == NULL)
            throw EvaluatorException("Incorrect abstract syntax tree");

        return EvaluateSubtree(ast);
    }
};
