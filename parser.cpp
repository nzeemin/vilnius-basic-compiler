
#include <cassert>
#include <iomanip>

#include "main.h"


int ExpressionNode::GetOperationPriority() const
{
    if (brackets)
        return 1;

    if (node.type == TokenTypeSymbol)
    {
        switch (node.symbol)
        {
        case '^':
            return 2;
        case '*': case '/':
            return 3;
        case '\\':
            return 4;
        case '+': case '-':
            return 6;
        default:
            return 0;
        }
    }

    if (node.type == TokenTypeKeyword && node.keyword == KeywordMOD)
        return 5;

    return 0;
}

void ExpressionNode::Dump(std::ostream& out) const
{
    out << "{Node ";

    if (left == -1)
        out << "  ";
    else
        out << std::right << std::setw(2) << left;
    std::cout << ":";
    if (right == -1)
        out << "  ";
    else
        out << std::left << std::setw(2) << right;
    int pri = GetOperationPriority();
    if (pri > 0)
        out << " !" << pri;
    else
        out << "   ";
    out << " ";

    node.Dump(out);

    if (brackets)
        std::cout << " brackets";

    out << " }";
}


const ParserKeywordSpec Parser::m_keywordspecs[] =
{
    { KeywordBEEP,	    &Parser::ParseBeep },
    { KeywordCLS,	    &Parser::ParseCls },
    { KeywordEND,	    &Parser::ParseEnd },
    { KeywordFOR,	    &Parser::ParseFor },
    { KeywordGOSUB,     &Parser::ParseGosub },
    { KeywordGOTO,	    &Parser::ParseGoto },
    { KeywordIF,        &Parser::ParseIf },
    { KeywordLET,	    &Parser::ParseLet },
    { KeywordNEXT,	    &Parser::ParseNext },
    { KeywordON,        &Parser::ParseOn },
    { KeywordPRINT,	    &Parser::ParsePrint },
    { KeywordREM,       &Parser::ParseRem },
    { KeywordRETURN,    &Parser::ParseReturn },
    { KeywordSTOP,      &Parser::ParseStop },
    { KeywordTRON,      &Parser::ParseTron },
    { KeywordTRON,      &Parser::ParseTroff },
};


Parser::Parser(Tokenizer* tokenizer)
{
    assert(tokenizer != nullptr);

    m_tokenizer = tokenizer;
    m_nexttoken.type = TokenTypeNone;
    m_havenexttoken = false;
}

Token Parser::GetNextToken()
{
    if (m_havenexttoken)
    {
        m_havenexttoken = false;
        return m_nexttoken;
    }

    return m_tokenizer->GetNextToken();
}

Token Parser::PeekNextToken()
{
    if (m_havenexttoken)
        return m_nexttoken;

    m_havenexttoken = true;
    m_nexttoken = m_tokenizer->GetNextToken();
    return m_nexttoken;
}

SourceLineModel Parser::ParseNextLine()
{
    Token token = GetNextToken();

    SourceLineModel model;

    if (token.type == TokenTypeEOF)
    {
        model.number = 0;
        return model;
    }

    if (token.type == TokenTypeEOL)  // Empty lines allowed at the end of file
    {
        while (true)
        {
            token = GetNextToken();
            if (token.type == TokenTypeEOL || token.type == TokenTypeDivider)
                continue;
            if (token.type == TokenTypeEOF)
            {
                model.number = 0;
                return model;
            }

            Error(model, token, "Unexpected text after empty line.");
            return model;
        }
    }

    if (token.type != TokenTypeNumber)
    {
        Error(model, token, " Line number not found.");
        return model;
    }
    //TODO: Check if line number in proper format
    model.number = atoi(token.text.c_str());
    //TODO: Check for 0 and max

    token = GetNextToken();
    if (token.type == TokenTypeDivider)
        token = GetNextToken();

    if (token.type == TokenTypeSymbol && token.symbol == '\'')  // REM short form
    {
        ParseRem(model);
        model.statement = token;
        return model;
    }
    if (token.type == TokenTypeSymbol && token.symbol == '?')  // PRINT short form
    {
        ParsePrint(model);
        model.statement = token;
        return model;
    }
    if (token.type == TokenTypeIdentifier)  // LET without the keyword
    {
        Token tokenlet;
        tokenlet.type = TokenTypeKeyword;
        tokenlet.keyword = KeywordLET;
        tokenlet.text = "LET";
        model.statement = tokenlet;

        ParseLetShort(token, model);
        return model;
    }

    if (token.type != TokenTypeKeyword)
    {
        std::cerr << "ERROR at line " << model.number << ": Keyword expected." << std::endl;
        exit(EXIT_FAILURE);
    }

    model.statement = token;

    // Find keyword parser implementation
    ParseMethodRef methodref = nullptr;
    for (int i = 0; i < sizeof(m_keywordspecs) / sizeof(m_keywordspecs[0]); i++)
    {
        if (token.keyword == m_keywordspecs[i].keyword)
        {
            methodref = m_keywordspecs[i].methodref;
            break;
        }
    }

    if (methodref != nullptr)
    {
        (this->*methodref)(model);
    }
    else
        SkipTilEnd(); //STUB

    return model;
}

void Parser::Error(SourceLineModel& model, Token& token, const char* message)
{
    std::cerr << "ERROR at " << token.line << ":" << token.pos << " line " << model.number << " - " << message;
    exit(EXIT_FAILURE);
}

void Parser::SkipTilEnd()
{
    while (true)  // Skip til EOL/EOF
    {
        Token token = GetNextToken();
        if (token.type == TokenTypeEOL || token.type == TokenTypeEOF)
            break;
    }
}

ExpressionModel Parser::ParseExpression(SourceLineModel& model)
{
    ExpressionModel expression;
    expression.root = -1;  // Empty expression for now

    bool isop = false;  // Currently on operation or not
    int prev = -1;  // Index of previous operation

    Token token = PeekNextToken();
    if (token.type == TokenTypeDivider)
    {
        GetNextToken();
        token = PeekNextToken();
    }
    if (token.IsEndOfExpression())
        return expression;  // Empty expression

    // Check if we have unary plus/minus sign
    if (token.type == TokenTypeSymbol && (token.symbol == '+' || token.symbol == '-'))
    {
        token = GetNextToken();  // get the token we peeked

        ExpressionNode nodeun;
        nodeun.node = token;
        expression.nodes.push_back(nodeun);
        expression.root = 0;
        prev = 0;
    }

    // Loop parse expression tokens into list
    while (true)
    {
        Token token = PeekNextToken();
        if (token.type == TokenTypeDivider)
        {
            GetNextToken();
            token = PeekNextToken();
        }

        if (isop)  // Current node should be a binary operation
        {
            if (token.IsEndOfExpression())
                break;  // It's okay to end here

            if (!token.IsBinaryOperation())
            {
                Error(model, token, "Binary operation expected in expression.");
                return expression;
            }

            token = GetNextToken();  // get the token we peeked

            // Put the token into the list
            ExpressionNode node;
            node.node = token;
            int index = (int)expression.nodes.size();

            // Put node in the tree
            int pred = prev < 0 ? expression.root : prev;
            ExpressionNode& nodepred = expression.nodes[pred];
            if (nodepred.node.IsBinaryOperation() && !nodepred.brackets)
            {
                int pripred = nodepred.GetOperationPriority();
                int pri = node.GetOperationPriority();

                //TODO: Bubble up according to priority
                if (nodepred.left < 0)
                    nodepred.left = index;
                else if (nodepred.right < 0)
                    nodepred.right = index;
                else
                {
                    if (pripred <= pri)
                    {
                        node.left = pred;
                        expression.root = index;
                    }
                    else
                    {
                        node.left = nodepred.right;
                        nodepred.right = index;
                    }
                }
            }
            else {
                node.left = pred;
                expression.root = index;
            }

            expression.nodes.push_back(node);
            prev = index;
        }
        else  // Current note should be non-operation
        {
            if (token.IsEndOfExpression())
            {
                Error(model, token, "Operand expected in expression.");
                return expression;
            }
            if (token.IsBinaryOperation())
            {
                Error(model, token, "Binary operation is not expected here.");
                return expression;
            }

            token = GetNextToken();  // get the token we peeked

            int index = -1;  // Index of the new node/sub-tree
            if (token.IsOpenBracket())  // Do recursion for expression inside brackets
            {
                ExpressionModel exprin = ParseExpression(model);

                if (exprin.IsEmpty())
                {
                    Error(model, token, "Expression in brackets should not be empty.");
                    return expression;
                }

                // Move expression nodes in the list
                int shift = (int)expression.nodes.size();
                for (size_t i = 0; i < exprin.nodes.size(); i++)
                {
                    ExpressionNode& node = exprin.nodes[i];
                    if (i == exprin.root)
                        node.brackets = true;
                    if (node.left >= 0)
                        node.left += shift;
                    if (node.right >= 0)
                        node.right += shift;
                    expression.nodes.push_back(node);
                }

                token = GetNextToken();
                if (!token.IsCloseBracket())
                {
                    Error(model, token, "Close bracket expected in expression.");
                    return expression;
                }

                index = exprin.root + shift;
            }
            else if (IsFunctionKeyword(token.keyword))  // Function with parameter list
            {
                ExpressionNode node;
                node.node = token;

                token = PeekNextToken();
                if (token.type == TokenTypeDivider)
                {
                    GetNextToken();
                    token = PeekNextToken();
                }
                if (token.IsOpenBracket())  // Function parameter list
                {
                    GetNextToken();  // open bracket

                    while (true)
                    {
                        ExpressionModel exprarg = ParseExpression(model);
                        node.args.push_back(exprarg);

                        token = PeekNextToken();
                        if (token.type == TokenTypeDivider)
                        {
                            GetNextToken();
                            token = PeekNextToken();
                        }
                        if (token.IsCloseBracket())
                        {
                            GetNextToken();
                            break;
                        }
                        if (!token.IsComma())
                        {
                            Error(model, token, "Comma expected in function parameter list.");
                            return expression;
                        }

                        GetNextToken();  // comma
                    }

                    //TODO: Validate number of params for this function
                }

                index = (int)expression.nodes.size();
                expression.nodes.push_back(node);
            }
            else  // Other token like Ident
            {
                // Put the token into the list
                ExpressionNode node;
                node.node = token;
                index = (int)expression.nodes.size();
                expression.nodes.push_back(node);
            }

            // Put node in the tree
            if (expression.root < 0)
                expression.root = index;
            else
            {
                int pred = prev < 0 ? expression.root : prev;
                ExpressionNode& nodepred = expression.nodes[pred];
                if (nodepred.right < 0)
                    nodepred.right = index;
            }
        }

        isop = !isop;
    }

    return expression;
}

void Parser::ParseBeep(SourceLineModel& model)
{
    Token token = GetNextToken();
    if (token.type == TokenTypeDivider)
        token = GetNextToken();
    if (token.type == TokenTypeEOL || token.type == TokenTypeEOF)
        return;

    Error(model, token, "Unexpected text after BEEP.");
}

void Parser::ParseCls(SourceLineModel& model)
{
    Token token = GetNextToken();
    if (token.type == TokenTypeDivider)
        token = GetNextToken();
    if (token.type == TokenTypeEOL || token.type == TokenTypeEOF)
        return;

    Error(model, token, "Unexpected text after CLS.");
}

void Parser::ParseEnd(SourceLineModel& model)
{
    Token token = GetNextToken();
    if (token.type == TokenTypeDivider)
        token = GetNextToken();
    if (token.type == TokenTypeEOL || token.type == TokenTypeEOF)
        return;

    Error(model, token, "Unexpected text after END.");
}

void Parser::ParseFor(SourceLineModel& model)
{
    Token token = GetNextToken();
    if (token.type == TokenTypeDivider)
        token = GetNextToken();
    if (token.type != TokenTypeIdentifier)
    {
        Error(model, token, "FOR variable expected.");
        return;
    }

    model.ident = token;

    token = GetNextToken();
    if (token.type == TokenTypeDivider)
        token = GetNextToken();
    if (token.type != TokenTypeSymbol || token.symbol != '=')
    {
        Error(model, token, "FOR \'=\' symbol expected.");
        return;
    }

    ExpressionModel exprfrom = ParseExpression(model);
    model.args.push_back(exprfrom);

    token = GetNextToken();
    if (token.type == TokenTypeDivider)
        token = GetNextToken();
    if (token.type != TokenTypeKeyword || token.keyword != KeywordTO)
    {
        Error(model, token, "TO expected in FOR operator.");
        return;
    }

    ExpressionModel exprto = ParseExpression(model);
    model.args.push_back(exprto);

    token = GetNextToken();
    if (token.type == TokenTypeDivider)
        token = GetNextToken();
    if (token.type == TokenTypeEOL || token.type == TokenTypeEOF)
        return;

    if (token.type != TokenTypeKeyword || token.keyword != KeywordSTEP)
    {
        Error(model, token, "Unepected text in FOR operator.");
        return;
    }

    ExpressionModel exprstep = ParseExpression(model);
    model.args.push_back(exprstep);

    token = GetNextToken();
    if (token.type == TokenTypeDivider)
        token = GetNextToken();
    if (token.type == TokenTypeEOL || token.type == TokenTypeEOF)
        return;

    Error(model, token, "Unexpected text after FOR operator.");
}

void Parser::ParseGosub(SourceLineModel& model)
{
    Token token = GetNextToken();
    if (token.type == TokenTypeDivider)
        token = GetNextToken();
    if (token.type != TokenTypeNumber)
    {
        Error(model, token, "GOSUB line number expected.");
        return;
    }

    model.gotoLine = atoi(token.text.c_str());

    token = GetNextToken();
    if (token.type == TokenTypeDivider)
        token = GetNextToken();
    if (token.type == TokenTypeEOL || token.type == TokenTypeEOF)
        return;

    Error(model, token, "Unexpected text after GOSUB line number.");
}

void Parser::ParseGoto(SourceLineModel& model)
{
    Token token = GetNextToken();
    if (token.type == TokenTypeDivider)
        token = GetNextToken();
    if (token.type != TokenTypeNumber)
    {
        Error(model, token, "GOTO line number expected.");
        return;
    }

    model.gotoLine = atoi(token.text.c_str());

    token = GetNextToken();
    if (token.type == TokenTypeDivider)
        token = GetNextToken();
    if (token.type == TokenTypeEOL || token.type == TokenTypeEOF)
        return;

    Error(model, token, "Unexpected text after GOTO line number.");
}

void Parser::ParseIf(SourceLineModel& model)
{
    Token token = PeekNextToken();
    ExpressionModel expr = ParseExpression(model);
    if (expr.IsEmpty())
    {
        Error(model, token, "IF condition should not be empty.");
        return;
    }

    //TODO

    SkipTilEnd();//STUB
}

//TODO: LET MID$

void Parser::ParseLet(SourceLineModel& model)
{
    Token token = GetNextToken();
    if (token.type == TokenTypeDivider)
        token = GetNextToken();
    if (token.type != TokenTypeIdentifier)
    {
        Error(model, token, "LET variable expected.");
        return;
    }

    ParseLetShort(token, model);
}

void Parser::ParseLetShort(Token& tokenIdent, SourceLineModel& model)
{
    model.ident = tokenIdent;

    Token token = GetNextToken();
    if (token.type == TokenTypeDivider)
        token = GetNextToken();
    if (token.type != TokenTypeSymbol || token.symbol != '=')
    {
        Error(model, token, "LET \'=\' symbol expected.");
        return;
    }

    ExpressionModel expr = ParseExpression(model);
    model.args.push_back(expr);

    SkipTilEnd(); //STUB
}

void Parser::ParseNext(SourceLineModel& model)
{
    Token token = GetNextToken();
    if (token.type == TokenTypeDivider)
        token = GetNextToken();
    if (token.type == TokenTypeEOL || token.type == TokenTypeEOF)
        return;

    while (true)
    {
        if (token.type != TokenTypeIdentifier)
        {
            Error(model, token, "NEXT variable expected.");
            return;
        }

        model.params.push_back(token);

        token = GetNextToken();
        if (token.type == TokenTypeDivider)
            token = GetNextToken();
        if (token.type == TokenTypeEOL || token.type == TokenTypeEOF)
            break;

        if (token.type != TokenTypeSymbol || token.symbol != ',')
        {
            Error(model, token, "NEXT comma or end of line expected.");
            return;
        }

        token = GetNextToken();
        if (token.type == TokenTypeDivider)
            token = GetNextToken();
    }
}

void Parser::ParseOn(SourceLineModel& model)
{
    Token token = PeekNextToken();
    ExpressionModel expr = ParseExpression(model);
    if (expr.IsEmpty())
    {
        Error(model, token, "ON expression should not be empty.");
        return;
    }

    token = GetNextToken();
    if (token.type == TokenTypeDivider)
        token = GetNextToken();
    if (token.type != TokenTypeKeyword || (token.keyword != KeywordGOTO && token.keyword != KeywordGOSUB))
    {
        Error(model, token, "Expected GOTO or GOSUB in ON statement.");
        return;
    }

    //TODO: Loop for line numbers, comma separated

    SkipTilEnd(); //STUB
}

//TODO: LPRINT
void Parser::ParsePrint(SourceLineModel& model)
{
    //TODO: Symbol #, optional

    ExpressionModel expr = ParseExpression(model);
    model.args.push_back(expr);

    //TODO: Check for end

    //TODO: Get separator

    //TODO: Check for end

    SkipTilEnd(); //STUB
}

void Parser::ParseRem(SourceLineModel& model)
{
    while (true)  // Skip til EOL/EOF
    {
        Token token = GetNextToken();
        if (token.type == TokenTypeEOL || token.type == TokenTypeEOF)
            break;
    }
}

void Parser::ParseReturn(SourceLineModel& model)
{
    Token token = GetNextToken();
    if (token.type == TokenTypeDivider)
        token = GetNextToken();
    if (token.type == TokenTypeEOL || token.type == TokenTypeEOF)
        return;

    Error(model, token, "Unexpected text after RETURN.");
}

void Parser::ParseStop(SourceLineModel& model)
{
    Token token = GetNextToken();
    if (token.type == TokenTypeDivider)
        token = GetNextToken();
    if (token.type == TokenTypeEOL || token.type == TokenTypeEOF)
        return;

    Error(model, token, "Unexpected text after STOP.");
}

void Parser::ParseTron(SourceLineModel& model)
{
    Token token = GetNextToken();
    if (token.type == TokenTypeDivider)
        token = GetNextToken();
    if (token.type == TokenTypeEOL || token.type == TokenTypeEOF)
        return;

    Error(model, token, "Unexpected text after TRON.");
}

void Parser::ParseTroff(SourceLineModel& model)
{
    Token token = GetNextToken();
    if (token.type == TokenTypeDivider)
        token = GetNextToken();
    if (token.type == TokenTypeEOL || token.type == TokenTypeEOF)
        return;

    Error(model, token, "Unexpected text after TROFF.");
}
