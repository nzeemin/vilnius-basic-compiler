
#include <cassert>
#include <iomanip>

#include "main.h"


//////////////////////////////////////////////////////////////////////


const ParserKeywordSpec Parser::m_keywordspecs[] =
{
    { KeywordBEEP,      &Parser::ParseBeep },
    { KeywordCLEAR,     &Parser::ParseClear },
    { KeywordCLS,       &Parser::ParseCls },
    { KeywordCOLOR,     &Parser::ParseColor },
    { KeywordDATA,      &Parser::ParseData },
    { KeywordDIM,       &Parser::ParseDim },
    { KeywordEND,       &Parser::ParseEnd },
    { KeywordFOR,       &Parser::ParseFor },
    { KeywordGOSUB,     &Parser::ParseGosub },
    { KeywordGOTO,      &Parser::ParseGoto },
    { KeywordIF,        &Parser::ParseIf },
    { KeywordLET,       &Parser::ParseLet },
    { KeywordLOCATE,    &Parser::ParseLocate },
    { KeywordNEXT,      &Parser::ParseNext },
    { KeywordON,        &Parser::ParseOn },
    { KeywordOUT,       &Parser::ParseOut },
    { KeywordPOKE,      &Parser::ParsePoke },
    { KeywordPRINT,     &Parser::ParsePrint },
    { KeywordREAD,      &Parser::ParseRead },
    { KeywordREM,       &Parser::ParseRem },
    { KeywordRESTORE,   &Parser::ParseRestore },
    { KeywordRETURN,    &Parser::ParseReturn },
    { KeywordSCREEN,    &Parser::ParseScreen },
    { KeywordSTOP,      &Parser::ParseStop },
    { KeywordTRON,      &Parser::ParseTron },
    { KeywordTRON,      &Parser::ParseTroff },
};

const ParserFunctionSpec Parser::m_funcspecs[] =
{
    { KeywordSQR,       1, 1, ValueTypeSingle },
    { KeywordSIN,       1, 1, ValueTypeSingle },
    { KeywordCOS,       1, 1, ValueTypeSingle },
    { KeywordTAN,       1, 1, ValueTypeSingle },
    { KeywordATN,       1, 1, ValueTypeSingle },
    { KeywordPI,        0, 0, ValueTypeSingle },
    { KeywordEXP,       1, 1, ValueTypeSingle },
    { KeywordLOG,       1, 1, ValueTypeSingle },
    { KeywordABS,       1, 1, ValueTypeSingle },
    { KeywordFIX,       1, 1, ValueTypeInteger },
    { KeywordINT,       1, 1, ValueTypeInteger },
    { KeywordSGN,       1, 1, ValueTypeSingle },
    { KeywordRND,       1, 1, ValueTypeSingle },
    { KeywordFRE,       0, 1, ValueTypeInteger },
    { KeywordCINT,      1, 1, ValueTypeInteger },
    { KeywordCSNG,      1, 1, ValueTypeSingle },
    { KeywordCDBL,      1, 1, ValueTypeSingle }, // ValueTypeDouble
    { KeywordPEEK,      1, 1, ValueTypeInteger },
    { KeywordINP,       2, 2, ValueTypeInteger },
    { KeywordASC,       1, 1, ValueTypeInteger },
    { KeywordCHR,       1, 1, ValueTypeString },
    { KeywordLEN,       1, 1, ValueTypeInteger },
    { KeywordMID,       3, 3, ValueTypeString },
    { KeywordSTRING,    2, 2, ValueTypeString },
    { KeywordVAL,       1, 1, ValueTypeSingle },
    { KeywordINKEY,     0, 0, ValueTypeString },
    { KeywordSTR,       1, 1, ValueTypeString },
    { KeywordBIN,       1, 1, ValueTypeString },
    { KeywordOCT,       1, 1, ValueTypeString },
    { KeywordHEX,       1, 1, ValueTypeString },
    { KeywordCSRLIN,    0, 1, ValueTypeInteger },
    { KeywordPOS,       0, 1, ValueTypeInteger },
    { KeywordLPOS,      0, 1, ValueTypeInteger },
    { KeywordEOF,       0, 0, ValueTypeInteger },
    { KeywordAT,        2, 2, ValueTypeNone },
    { KeywordTAB,       1, 1, ValueTypeNone },
    { KeywordSPC,       1, 1, ValueTypeNone },
    //NOTE: FN has special syntax
    //NOTE: USR has special syntax
};


const int Parser::FindFunctionSpec(KeywordIndex keyword)
{
    for (int i = 0; i < sizeof(m_funcspecs) / sizeof(m_funcspecs[0]); i++)
    {
        if (keyword == m_funcspecs[i].keyword)
            return i;
    }

    return -1;
}

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

Token Parser::GetNextTokenSkipDivider()
{
    Token token = GetNextToken();
    if (token.type == TokenTypeDivider)
        token = GetNextToken();
    return token;
}

Token Parser::PeekNextToken()
{
    if (m_havenexttoken)
        return m_nexttoken;

    m_havenexttoken = true;
    m_nexttoken = m_tokenizer->GetNextToken();
    return m_nexttoken;
}

Token Parser::PeekNextTokenSkipDivider()
{
    Token token = PeekNextToken();
    if (token.type == TokenTypeDivider)
    {
        GetNextToken();
        token = PeekNextToken();
    }
    return token;
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
        Error(model, token, "Line number not found.");
        return model;
    }
    //TODO: Check if line number in proper format
    model.number = atoi(token.text.c_str());
    if (model.number <= 0 || model.number > MAX_LINE_NUMBER)
    {
        Error(model, token, "Line number is out of range.");
        return model;
    }
    //TODO: Compare line number with previous line number

    token = GetNextTokenSkipDivider();

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
        Error(model, token, "Statement keyword expected.");
        exit(EXIT_FAILURE);
    }
    if (IsFunctionKeyword(token.keyword))
    {
        Error(model, token, "Statement keyword expected, function keyword found.");
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
    if (methodref == nullptr)
    {
        Error(model, token, "Parser not found for the keyword.");
        exit(EXIT_FAILURE);
    }

    (this->*methodref)(model);

    return model;
}

void Parser::Error(SourceLineModel& model, Token& token, const char* message)
{
    std::cerr << "ERROR at " << token.line << ":" << token.pos << " line " << model.number << " - " << message << std::endl;
    string linetext = m_tokenizer->GetLineText();
    if (!linetext.empty())
    {
        std::cerr << linetext << std::endl;
        std::cerr << std::right << std::setw(token.pos) << "^";
    }
    exit(EXIT_FAILURE);
}

void Parser::SkipTilEnd()
{
    while (true)  // Skip til EOL/EOF
    {
        Token token = GetNextToken();
        if (token.IsEolOrEof())
            break;
    }
}


ExpressionModel Parser::ParseExpression(SourceLineModel& model)
{
    ExpressionModel expression;
    expression.root = -1;  // Empty expression for now

    bool isop = false;  // Currently on operation or not
    int prev = -1;  // Index of previous operation

    Token token = PeekNextTokenSkipDivider();
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
        Token token = PeekNextTokenSkipDivider();

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

            prev = expression.AddOperationNode(node, prev);
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
                int funcspecindex = FindFunctionSpec(token.keyword);
                assert(funcspecindex >= 0);
                const ParserFunctionSpec& funcspec = m_funcspecs[funcspecindex];

                ExpressionNode node;
                node.node = token;
                node.vtype = funcspec.resulttype;

                token = PeekNextTokenSkipDivider();
                if (token.IsOpenBracket())  // Function parameter list
                {
                    if (funcspec.maxparams == 0)
                    {
                        Error(model, token, "This function should not have any parameters.");
                        return expression;
                    }

                    GetNextToken();  // open bracket

                    while (true)
                    {
                        ExpressionModel exprarg = ParseExpression(model);
                        node.args.push_back(exprarg);

                        token = PeekNextTokenSkipDivider();
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
                }

                // Validate number of params for this function
                if ((int)node.args.size() < funcspec.minparams)
                {
                    Error(model, token, "Specified too few parameters for this function.");
                    return expression;
                }
                if ((int)node.args.size() > funcspec.maxparams)
                {
                    Error(model, token, "Specified too many parameters for this function.");
                    return expression;
                }

                index = (int)expression.nodes.size();
                expression.nodes.push_back(node);
            }
            else  // Other token like Ident
            {
                // Put the token into the list
                ExpressionNode node;
                node.node = token;
                node.vtype = token.vtype;
                node.constval = (token.type == TokenTypeNumber || token.type == TokenTypeString);

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

    // Calculate vtype/const for all expression nodes
    expression.CalculateVTypes();

    return expression;
}

void Parser::ParseBeep(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    Error(model, token, "Unexpected text after BEEP.");
}

void Parser::ParseClear(SourceLineModel& model)
{
    Token token = PeekNextTokenSkipDivider();
    if (token.IsEolOrEof())
    {
        Error(model, token, "Argument expected in CLEAR statement.");
        return;
    }

    ExpressionModel expr1 = ParseExpression(model);
    model.args.push_back(expr1);
    if (expr1.IsEmpty())
    {
        Error(model, token, "Expression should not be empty.");
        return;
    }

    token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;  // One argument

    if (!token.IsComma())
    {
        Error(model, token, "Unexpected text in CLEAR statement.");
        return;
    }

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr2 = ParseExpression(model);
    model.args.push_back(expr2);
    if (expr2.IsEmpty())
    {
        Error(model, token, "Expression should not be empty.");
        return;
    }

    token = PeekNextTokenSkipDivider();
    if (!token.IsEolOrEof())
        Error(model, token, "Unexpected text after CLEAR arguments.");
}

void Parser::ParseCls(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    Error(model, token, "Unexpected text after CLS.");
}

void Parser::ParseColor(SourceLineModel& model)
{
    Token token = PeekNextTokenSkipDivider();
    if (token.IsEolOrEof())
    {
        Error(model, token, "Arguments expected in COLOR statement.");
        return;
    }

    ExpressionModel expr1 = ParseExpression(model);
    model.args.push_back(expr1);
    if (expr1.IsEmpty())
    {
        Error(model, token, "Expression should not be empty.");
        return;
    }

    token = PeekNextTokenSkipDivider();
    if (!token.IsComma())
    {
        Error(model, token, "Comma expected after the argument.");
        return;
    }
    GetNextToken();  // Comma

    ExpressionModel expr2 = ParseExpression(model);
    model.args.push_back(expr2);
    if (expr2.IsEmpty())
    {
        Error(model, token, "Expression should not be empty.");
        return;
    }

    token = PeekNextTokenSkipDivider();
    if (!token.IsComma())
    {
        Error(model, token, "Comma expected after the argument.");
        return;
    }
    GetNextToken();  // Comma

    ExpressionModel expr3 = ParseExpression(model);
    model.args.push_back(expr3);
    if (expr3.IsEmpty())
    {
        Error(model, token, "Expression should not be empty.");
        return;
    }

    token = PeekNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    Error(model, token, "Unexpected text after COLOR arguments.");
}

void Parser::ParseData(SourceLineModel& model)
{
    //TODO

    SkipTilEnd();//STUB
}

void Parser::ParseDim(SourceLineModel& model)
{
    Token token;
    while (true)
    {
        token = GetNextTokenSkipDivider();
        if (token.type != TokenTypeIdentifier)
        {
            Error(model, token, "Identifier expected in DIM statement.");
            return;
        }
        token = GetNextTokenSkipDivider();
        if (token.IsOpenBracket())
        {
            while (true)
            {
                token = GetNextTokenSkipDivider();
                if (token.type != TokenTypeNumber)
                {
                    Error(model, token, "Array size expected in DIM statement.");
                    return;
                }
                //TODO: save to model

                token = GetNextTokenSkipDivider();
                if (token.IsCloseBracket())
                    break;
                if (!token.IsComma())
                {
                    Error(model, token, "Comma expected in DIM statement.");
                    return;
                }
            }

            token = GetNextTokenSkipDivider();
        }
        if (token.IsEolOrEof())
            return;  // End of the list

        if (!token.IsComma())
        {
            Error(model, token, "Comma expected in DIM statement.");
            return;
        }
    }
}

void Parser::ParseEnd(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    Error(model, token, "Unexpected text after END.");
}

void Parser::ParseFor(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeIdentifier)
    {
        Error(model, token, "FOR variable expected.");
        return;
    }

    model.ident = token;

    token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeSymbol || token.symbol != '=')
    {
        Error(model, token, "FOR \'=\' symbol expected.");
        return;
    }

    token = PeekNextToken();
    ExpressionModel expr1 = ParseExpression(model);
    if (expr1.IsEmpty())
    {
        Error(model, token, "Expression should not be empty.");
        return;
    }
    model.args.push_back(expr1);

    token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeKeyword || token.keyword != KeywordTO)
    {
        Error(model, token, "TO expected in FOR operator.");
        return;
    }

    token = PeekNextToken();
    ExpressionModel expr2 = ParseExpression(model);
    if (expr2.IsEmpty())
    {
        Error(model, token, "Expression should not be empty.");
        return;
    }
    model.args.push_back(expr2);

    token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    if (token.type != TokenTypeKeyword || token.keyword != KeywordSTEP)
    {
        Error(model, token, "Unepected text in FOR operator.");
        return;
    }

    token = PeekNextToken();
    ExpressionModel expr3 = ParseExpression(model);
    if (expr3.IsEmpty())
    {
        Error(model, token, "Expression should not be empty.");
        return;
    }
    model.args.push_back(expr3);

    token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    Error(model, token, "Unexpected text after FOR operator.");
}

void Parser::ParseGosub(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeNumber)
    {
        Error(model, token, "GOSUB line number expected.");
        return;
    }

    model.paramline = atoi(token.text.c_str());

    token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    Error(model, token, "Unexpected text after GOSUB line number.");
}

void Parser::ParseGoto(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeNumber)
    {
        Error(model, token, "GOTO line number expected.");
        return;
    }

    model.paramline = atoi(token.text.c_str());

    token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
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
    Token token = GetNextTokenSkipDivider();
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

    Token token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeSymbol || token.symbol != '=')
    {
        Error(model, token, "LET \'=\' symbol expected.");
        return;
    }

    ExpressionModel expr = ParseExpression(model);
    if (expr.IsEmpty())
    {
        Error(model, token, "Expression should not be empty.");
        return;
    }
    model.args.push_back(expr);

    token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    Error(model, token, "Unexpected text after LET expression.");
}

void Parser::ParseLocate(SourceLineModel& model)
{
    Token token = PeekNextTokenSkipDivider();
    if (token.IsEolOrEof())
    {
        Error(model, token, "Arguments expected in LOCATE statement.");
        return;
    }

    ExpressionModel expr1 = ParseExpression(model);
    model.args.push_back(expr1);
    if (expr1.IsEmpty())
    {
        Error(model, token, "Expression should not be empty.");
        return;
    }

    token = PeekNextTokenSkipDivider();
    if (!token.IsComma())
    {
        Error(model, token, "Comma expected after the argument.");
        return;
    }
    GetNextToken();  // Comma

    ExpressionModel expr2 = ParseExpression(model);
    model.args.push_back(expr2);
    if (expr2.IsEmpty())
    {
        Error(model, token, "Expression should not be empty.");
        return;
    }

    token = PeekNextTokenSkipDivider();
    if (!token.IsComma())
    {
        Error(model, token, "Comma expected after the argument.");
        return;
    }
    GetNextToken();  // Comma

    ExpressionModel expr3 = ParseExpression(model);
    model.args.push_back(expr3);
    if (expr3.IsEmpty())
    {
        Error(model, token, "Expression should not be empty.");
        return;
    }

    token = PeekNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    Error(model, token, "Unexpected text after LOCATE arguments.");
}

void Parser::ParseNext(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    while (true)
    {
        if (token.type != TokenTypeIdentifier)
        {
            Error(model, token, "NEXT variable expected.");
            return;
        }

        model.params.push_back(token);

        token = GetNextTokenSkipDivider();
        if (token.IsEolOrEof())
            break;

        if (token.type != TokenTypeSymbol || token.symbol != ',')
        {
            Error(model, token, "NEXT comma or end of line expected.");
            return;
        }

        token = GetNextTokenSkipDivider();
    }
}

void Parser::ParseOn(SourceLineModel& model)
{
    Token token = PeekNextTokenSkipDivider();
    ExpressionModel expr = ParseExpression(model);
    if (expr.IsEmpty())
    {
        Error(model, token, "Expression should not be empty.");
        return;
    }

    token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeKeyword || (token.keyword != KeywordGOTO && token.keyword != KeywordGOSUB))
    {
        Error(model, token, "GOTO or GOSUB expected in ON statement.");
        return;
    }
    model.gotogosub = (token.keyword == KeywordGOTO);

    // Loop for line numbers, comma separated
    while (true)
    {
        token = PeekNextTokenSkipDivider();
        if (token.type != TokenTypeNumber)
        {
            Error(model, token, "Line number expected in ON statement.");
            return;
        }
        token = GetNextToken();
        model.params.push_back(token);

        token = PeekNextTokenSkipDivider();
        if (token.IsEolOrEof())
            break;
        if (!token.IsComma())
        {
            Error(model, token, "Unexpected text in ON statement.");
            return;
        }

        GetNextToken();
    }
}

void Parser::ParseOut(SourceLineModel& model)
{
    Token token = PeekNextTokenSkipDivider();
    if (token.IsEolOrEof())
    {
        Error(model, token, "Arguments expected in OUT statement.");
        return;
    }

    ExpressionModel expr1 = ParseExpression(model);
    model.args.push_back(expr1);
    if (expr1.IsEmpty())
    {
        Error(model, token, "Expression should not be empty.");
        return;
    }

    token = PeekNextTokenSkipDivider();
    if (!token.IsComma())
    {
        Error(model, token, "Comma expected after the argument.");
        return;
    }
    GetNextToken();  // Comma

    ExpressionModel expr2 = ParseExpression(model);
    model.args.push_back(expr2);
    if (expr2.IsEmpty())
    {
        Error(model, token, "Expression should not be empty.");
        return;
    }

    token = PeekNextTokenSkipDivider();
    if (!token.IsComma())
    {
        Error(model, token, "Comma expected after the argument.");
        return;
    }
    GetNextToken();  // Comma

    ExpressionModel expr3 = ParseExpression(model);
    model.args.push_back(expr3);
    if (expr3.IsEmpty())
    {
        Error(model, token, "Expression should not be empty.");
        return;
    }

    token = PeekNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    Error(model, token, "Unexpected text after OUT arguments.");
}

//TODO: LPRINT
void Parser::ParsePrint(SourceLineModel& model)
{
    Token token = PeekNextTokenSkipDivider();
    if (token.IsEolOrEof())
    {
        GetNextToken();
        return;  // Empty PRINT
    }

    //TODO: Symbol #, optional
    //TODO: Check for end

    while (true)
    {
        ExpressionModel expr = ParseExpression(model);
        model.args.push_back(expr);

        token = GetNextTokenSkipDivider();
        if (token.IsEolOrEof())
            return;
        if (!token.IsComma() && !token.IsSemicolon())
        {
            Error(model, token, "Comma or semicolon expected in PRINT statement.");
            return;
        }

        token = PeekNextTokenSkipDivider();
        if (token.IsEolOrEof())
        {
            GetNextToken();
            return;
        }
    }
}

void Parser::ParsePoke(SourceLineModel& model)
{
    Token token = PeekNextTokenSkipDivider();
    ExpressionModel expr1 = ParseExpression(model);
    if (expr1.IsEmpty())
    {
        Error(model, token, "Expression should not be empty.");
        return;
    }
    model.args.push_back(expr1);

    token = PeekNextTokenSkipDivider();
    if (!token.IsComma())
    {
        Error(model, token, "Comma expected after the argument.");
        return;
    }
    GetNextToken();  // Comma

    ExpressionModel expr2 = ParseExpression(model);
    model.args.push_back(expr2);
    if (expr2.IsEmpty())
    {
        Error(model, token, "Expression should not be empty.");
        return;
    }

    token = PeekNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    Error(model, token, "Unexpected text after POKE arguments.");
}

void Parser::ParseRead(SourceLineModel& model)
{
    Token token;
    while (true)
    {
        token = GetNextTokenSkipDivider();
        if (token.type != TokenTypeIdentifier)
        {
            Error(model, token, "Identifier expected in DIM statement.");
            return;
        }
        token = GetNextTokenSkipDivider();
        if (token.IsOpenBracket())
        {
            while (true)
            {
                ExpressionModel expr = ParseExpression(model);

                if (expr.IsEmpty())
                {
                    Error(model, token, "Expression should not be empty.");
                    return;
                }
                //TODO: save to model

                token = GetNextTokenSkipDivider();
                if (token.IsCloseBracket())
                    break;
                if (!token.IsComma())
                {
                    Error(model, token, "Comma expected in DIM statement.");
                    return;
                }
            }

            token = GetNextTokenSkipDivider();
        }
        if (token.IsEolOrEof())
            return;  // End of the list

        if (!token.IsComma())
        {
            Error(model, token, "Comma expected in DIM statement.");
            return;
        }
    }
}

void Parser::ParseRem(SourceLineModel& model)
{
    while (true)  // Skip til EOL/EOF
    {
        Token token = GetNextToken();
        if (token.IsEolOrEof())
            break;
    }
}

void Parser::ParseRestore(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
    {
        model.paramline = 0;
        return;  // RESTORE without parameters
    }

    if (token.type != TokenTypeNumber)
    {
        Error(model, token, "Number argument expected.");
        return;
    }
    model.paramline = (int)token.dvalue;

    token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    Error(model, token, "Unexpected text after RESTORE argument.");
}

void Parser::ParseReturn(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    Error(model, token, "Unexpected text after RETURN.");
}

void Parser::ParseScreen(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeNumber)
    {
        Error(model, token, "Number argument expected.");
        return;
    }

    model.params.push_back(token);

    token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    Error(model, token, "Unexpected text after SCREEN argument.");
}

void Parser::ParseStop(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    Error(model, token, "Unexpected text after STOP.");
}

void Parser::ParseTron(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    Error(model, token, "Unexpected text after TRON.");
}

void Parser::ParseTroff(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    Error(model, token, "Unexpected text after TROFF.");
}


//////////////////////////////////////////////////////////////////////
