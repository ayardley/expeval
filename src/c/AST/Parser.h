/*
 * Parser.cpp - A simple expression evaluator, which uses an AST as the
 * backing data structure with which to evaluate the arithmetic expressions.
 * (See '../Simple/Parser.cpp' for additional comments, which I will not
 *  duplicate here.)
 *
 * Note: An abstract syntax tree is a binary tree. The inner nodes represent
 *       operators and leafs will be numerical values.
 *
 *       This is how an AST node looks[*]:
 *
 *        ---------------------
 *       |Type|Value|Left|Right|
 *        ---------------------
 *
 *       For example, the AST for the expression, '1+2*3', is,
 *
 *        ---------------------
 *       | +  |     |Left|Right|
 *        ---------------------
 *                   /      \
 *                  /        \
 *                 /          \
 *  --------------------    ---------------------
 * |NUM |  1  |NULL|NULL|  | *  |     |Left|Right|
 *  --------------------    ---------------------
 *                                     /      \
 *                                    /        \
 *                                   /          \
 *                  ---------------------    ---------------------
 *                 |NUM |     |Left|Right|  | +  |     |Left|Right|
 *                  ---------------------    ---------------------
 *
 * ---------------------
 * [*] I've compressed, i.e., removed additional whitespace, the above graphic
 * in order to present the truly important information.
 *
 *
 * We build the tree by inserting semantic actions and adding nodes, according
 * to the following rules:
 *
 *  ---------------------------------------------------------------------------
 * |PRODUCTION              |SEMANTIC RULE                                     |
 *  ---------------------------------------------------------------------------
 * |EXP    -> TERM EXP1     |EXP.node    = mknode(Plus, TERM.node, EXP1.node)  |
 * |EXP1   -> + TERM EXP1   |EXP1.node   = mknode(Plus, EXP1.node, TERM.node)  |
 * |EXP1   -> - TERM EXP1   |EXP1.node   = mknode(Minus, EXP1.node, TERM.node) |
 * |EXP1   -> epsilon       |EXP1.node   = mknode(Number, 0)                   |
 * |TERM   -> FACTOR TERM1  |TERM.node   = mknode(Mul, FACTOR.node, TERM1.node)|
 * |TERM1  -> * FACTOR TERM1|TERM1.node  = mknode(Mul, TERM1.node, FACTOR.node)|
 * |TERM1  -> / FACTOR TERM1|TERM1.node  = mknode(Div, TERM1.node, FACTOR.node)|
 * |TERM1  -> epsilon       |TERM1.node  = mknode(Number, 1)                   |
 * |FACTOR -> ( EXP )       |FACTOR.node = mknode(EXP.node)                    |
 * |FACTOR -> - EXP         |FACTOR.node = mknode(UnaryMinus, EXP.node)        |
 * |FACTOR -> number        |FACTOR.node = mknode(Number, number)              |
 *  ---------------------------------------------------------------------------
 *
 * Based on these rules, we will modify the AST somehwat, with some additional
 * nodes for the + and * operators (i.e., on the left, a leaf node with a
 * neutral element for the operation (0 for + and 1 for *), and on the right,
 * a node corresponding to a TERM or a FACTOR). This will not affect the
 * evaluation.
 */

#include <sstream>
#include <assert.h>
#include <stdexcept>
#include <string.h>
#include <stdlib.h>

#ifndef AST_H
#  include "AST.h"
#endif

// Exception class
// Note: I had to derive from 'std::runtime_error' which *will* take
//       a reference to a 'std::string'. 
class ParserException : public std::runtime_error
{
    int m_Pos;

public:
   ParserException(const std::string& message, int pos):
       std::runtime_error(message.c_str()),
       m_Pos(pos)
       {
       }
};

class Parser
{
    Token m_crtToken;
    const char* m_Text;
    size_t m_Index;

private:

    ASTNode* Expression()
    {
        ASTNode* tnode = Term();
        ASTNode* e1node = Expression1();

        return CreateNode(OperatorPlus, tnode, e1node);
    }

    ASTNode* Expression1()
    {
        ASTNode* tnode;
        ASTNode* e1node;

        switch(m_crtToken.Type) {
        case Plus:
            GetNextToken();
            tnode = Term();
            e1node = Expression1();

            return CreateNode(OperatorPlus, e1node, tnode);

        case Minus:
            GetNextToken();
            tnode = Term();
            e1node = Expression1();

            return CreateNode(OperatorMinus, e1node, tnode);
        }

        return CreateNodeNumber(0);
    }

    ASTNode* Term()
    {
        ASTNode* fnode = Factor();
        ASTNode* t1node = Term1();

        return CreateNode(OperatorMul, fnode, t1node);
    }

    ASTNode* Term1()
    {
        ASTNode* fnode;
        ASTNode* t1node;

        switch(m_crtToken.Type) {
        case Mul:
            GetNextToken();
            fnode = Factor();
            t1node = Term1();
            return CreateNode(OperatorMul, t1node, fnode);

        case Div:
            GetNextToken();
            fnode = Factor();
            t1node = Term1();
            return CreateNode(OperatorDiv, t1node, fnode);
        }
        
        return CreateNodeNumber(1);
    }

    ASTNode* Factor()
    {
        ASTNode* node;
        switch(m_crtToken.Type) {
        case OpenParenthesis:
            GetNextToken();
            node = Expression();
            Match(')');
            return node;

        case Minus:
            GetNextToken();
            node = Factor();
            return CreateUnaryNode(node);

        case Number: {
            double value = m_crtToken.Value;
            GetNextToken();
            return CreateNodeNumber(value);
        }

        default: {
            std::stringstream sstr;
            sstr << "Unexpected token '" << m_crtToken.Symbol << "' at position " << m_Index;
            throw ParserException(sstr.str(), m_Index);
        }
        }
    }

    ASTNode* CreateNode(ASTNodeType type, ASTNode* left, ASTNode* right)
    {
        ASTNode* node = new ASTNode;
        node->Type = type;
        node->Left = left;
        node->Right = right;

        return node;
    }

    ASTNode* CreateUnaryNode(ASTNode* left) 
    {
        ASTNode* node = new ASTNode;
        node->Type = UnaryMinus;
        node->Left = left;
        node->Right = NULL;

        return node;
    }

    ASTNode* CreateNodeNumber(double value)
    {
        ASTNode* node = new ASTNode;
        node->Type = NumberValue;
        node->Value = value;

        return node;
    }

    void Match(char expected)
    {
        if(m_Text[m_Index-1] == expected)
            GetNextToken();
        else {
            std::stringstream sstr;
            sstr << "Expected token '" << expected << "' at position " << m_Index;
            throw ParserException(sstr.str(), m_Index);
        }
    }

    void SkipWhitespaces()
    {
        while(isspace(m_Text[m_Index])) m_Index++;
    }

    void GetNextToken()
    {
        SkipWhitespaces();

        m_crtToken.Value = 0;
        m_crtToken.Symbol = 0;

        if(m_Text[m_Index] == 0) {
            m_crtToken.Type = EndOfText;
            return;
        }

        if(isdigit(m_Text[m_Index])) {
            m_crtToken.Type = Number;
            m_crtToken.Value = GetNumber();
            return;
        }

        m_crtToken.Type = Error;

        switch(m_Text[m_Index]) {
        case '+': m_crtToken.Type = Plus; break;
        case '-': m_crtToken.Type = Minus; break;
        case '*': m_crtToken.Type = Mul; break;
        case '/': m_crtToken.Type = Div; break;
        case '(': m_crtToken.Type = OpenParenthesis; break;
        case ')': m_crtToken.Type = ClosedParenthesis; break;
        }

        if(m_crtToken.Type != Error) {
            m_crtToken.Symbol = m_Text[m_Index];
            m_Index++;
        }
        else {
            std::stringstream sstr;
            sstr << "Unexpected token '" << m_Text[m_Index] << "' at position " << m_Index;
            throw ParserException(sstr.str(), m_Index);
        }
    }

    double GetNumber()
    {
        SkipWhitespaces();
        
        int index = m_Index;
        while(isdigit(m_Text[m_Index])) m_Index++;
        if(m_Text[m_Index] == '.') m_Index++;
        while(isdigit(m_Text[m_Index])) m_Index++;

        if(m_Index - index == 0)
            throw ParserException("Number expected but not found!", m_Index);

        char buffer[32] = {0};
        memcpy(buffer, &m_Text[index], m_Index - index);

        return atof(buffer);
    }

public:
    ASTNode* Parse(const char* text)
    {
        m_Text = text;
        m_Index = 0;
        GetNextToken();

        return Expression();
    }
};
