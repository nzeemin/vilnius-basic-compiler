
#include <cassert>
#include <iomanip>

#include "main.h"


//////////////////////////////////////////////////////////////////////


const ParserKeywordSpec Parser::m_keywordspecs[] =
{
    { KeywordBEEP,      &Parser::ParseStatementNoParams },
    { KeywordCLEAR,     &Parser::ParseClear },
    { KeywordCLS,       &Parser::ParseStatementNoParams },
    { KeywordCOLOR,     &Parser::ParseColor },
    { KeywordDATA,      &Parser::ParseData },
    { KeywordDIM,       &Parser::ParseDim },
    { KeywordDRAW,      &Parser::ParseDraw },
    { KeywordEND,       &Parser::ParseStatementNoParams },
    { KeywordFOR,       &Parser::ParseFor },
    { KeywordGOSUB,     &Parser::ParseGotoGosub },
    { KeywordGOTO,      &Parser::ParseGotoGosub },
    { KeywordIF,        &Parser::ParseIf },
    { KeywordLET,       &Parser::ParseLet },
    { KeywordLOCATE,    &Parser::ParseLocate },
    { KeywordNEXT,      &Parser::ParseNext },
    { KeywordON,        &Parser::ParseOn },
    { KeywordOUT,       &Parser::ParseOut },
    { KeywordPOKE,      &Parser::ParsePoke },
    { KeywordPSET,      &Parser::ParsePsetPreset },
    { KeywordPRESET,    &Parser::ParsePsetPreset },
    { KeywordPRINT,     &Parser::ParsePrint },
    { KeywordREAD,      &Parser::ParseRead },
    { KeywordREM,       &Parser::ParseRem },
    { KeywordRESTORE,   &Parser::ParseRestore },
    { KeywordRETURN,    &Parser::ParseStatementNoParams },
    { KeywordSCREEN,    &Parser::ParseScreen },
    { KeywordSTOP,      &Parser::ParseStatementNoParams },
    { KeywordTRON,      &Parser::ParseStatementNoParams },
    { KeywordTRON,      &Parser::ParseStatementNoParams },
    { KeywordWIDTH,     &Parser::ParseWidth },
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

const char* MSG_UNEXPECTED = "Unexpected text.";
const char* MSG_UNEXPECTED_AT_END_OF_STATEMENT = "Unexpected text at the end of the statement.";
const char* MSG_EXPRESSION_SHOULDNOT_BE_EMPTY = "Expression should not be empty.";
const char* MSG_COMMA_EXPECTED = "Comma expected.";
const char* MSG_OPEN_BRACKET_EXPECTED = "Open bracket expected.";
const char* MSG_CLOSE_BRACKET_EXPECTED = "Close bracket expected.";
const char* MSG_ARGUMENTS_EXPECTED = "Arguments expected.";


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

void Parser::CheckExpressionNotEmpty(SourceLineModel& model, Token& token, ExpressionModel& expr)
{
    if (expr.IsEmpty())
        Error(model, token, MSG_EXPRESSION_SHOULDNOT_BE_EMPTY);
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

    if (token.type == TokenTypeEndComment)  // REM short form
    {
        return model;  // Empty line with end-line comment
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
    if (token.type == TokenTypeKeyword && token.keyword == KeywordMID)
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

void Parser::SkipComma(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if (!token.IsComma())
    {
        Error(model, token, MSG_COMMA_EXPECTED);
        return;
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

void Parser::ParseStatementNoParams(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    Error(model, token, MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseClear(SourceLineModel& model)
{
    Token token = PeekNextTokenSkipDivider();
    if (token.IsEolOrEof())
    {
        Error(model, token, "Argument expected.");
        return;
    }

    ExpressionModel expr1 = ParseExpression(model);
    model.args.push_back(expr1);
    CheckExpressionNotEmpty(model, token, expr1);

    token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;  // One argument

    if (!token.IsComma())
    {
        Error(model, token, MSG_UNEXPECTED);
        return;
    }

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr2 = ParseExpression(model);
    model.args.push_back(expr2);
    CheckExpressionNotEmpty(model, token, expr2);

    token = PeekNextTokenSkipDivider();
    if (!token.IsEolOrEof())
        Error(model, token, MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseColor(SourceLineModel& model)
{
    Token token = PeekNextTokenSkipDivider();
    if (token.IsEolOrEof())
    {
        Error(model, token, MSG_ARGUMENTS_EXPECTED);
        return;
    }

    ExpressionModel expr1 = ParseExpression(model);
    model.args.push_back(expr1);
    CheckExpressionNotEmpty(model, token, expr1);

    SkipComma(model);

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr2 = ParseExpression(model);
    model.args.push_back(expr2);
    CheckExpressionNotEmpty(model, token, expr2);

    token = PeekNextTokenSkipDivider();
    if (!token.IsComma())
    {
        Error(model, token, MSG_COMMA_EXPECTED);
        return;
    }
    GetNextToken();  // Comma

    ExpressionModel expr3 = ParseExpression(model);
    model.args.push_back(expr3);
    CheckExpressionNotEmpty(model, token, expr3);

    token = GetNextTokenSkipDivider();
    if (!token.IsEolOrEof())
        Error(model, token, MSG_UNEXPECTED_AT_END_OF_STATEMENT);
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
            Error(model, token, "Identifier expected.");
            return;
        }

        VariableModel var;
        var.name = token.text;  //TODO: canonic form
            
        token = GetNextTokenSkipDivider();
        if (token.IsComma())  // end of definition
        {
            model.variables.push_back(var);
        }
        else if (token.IsOpenBracket())  // Array
        {
            while (true)
            {
                token = GetNextTokenSkipDivider();
                if (token.type != TokenTypeNumber)
                {
                    Error(model, token, "Array size expected.");
                    return;
                }
                if (!token.IsDValueInteger())
                {
                    Error(model, token, "Array size should be an integer.");
                    return;
                }
                //TODO: Check for limits
                var.indices.push_back((int)token.dvalue);

                token = GetNextTokenSkipDivider();
                if (token.IsCloseBracket())  // end of definition
                {
                    model.variables.push_back(var);
                }

                if (token.IsCloseBracket())
                    break;
                if (!token.IsComma())
                {
                    Error(model, token, MSG_COMMA_EXPECTED);
                    return;
                }
            }

            token = GetNextTokenSkipDivider();
        }
        if (token.IsEolOrEof())
            return;  // End of the list

        if (!token.IsComma())
        {
            Error(model, token, MSG_COMMA_EXPECTED);
            return;
        }
    }
}

void Parser::ParseDraw(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
    {
        Error(model, token, "Argument expected.");
        return;
    }
    if (token.type != TokenTypeString)
    {
        Error(model, token, "String argument expected.");
        return;
    }
    model.params.push_back(token);

    token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    Error(model, token, MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseFor(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeIdentifier)
    {
        Error(model, token, "Identifier expected.");
        return;
    }

    model.ident = token;

    token = GetNextTokenSkipDivider();
    if (!token.IsEqualSign())
    {
        Error(model, token, "FOR \'=\' symbol expected.");
        return;
    }

    token = PeekNextToken();
    ExpressionModel expr1 = ParseExpression(model);
    CheckExpressionNotEmpty(model, token, expr1);
    model.args.push_back(expr1);

    token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeKeyword || token.keyword != KeywordTO)
    {
        Error(model, token, "TO keyword expected.");
        return;
    }

    token = PeekNextToken();
    ExpressionModel expr2 = ParseExpression(model);
    CheckExpressionNotEmpty(model, token, expr2);
    model.args.push_back(expr2);

    token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    if (token.type != TokenTypeKeyword || token.keyword != KeywordSTEP)
    {
        Error(model, token, MSG_UNEXPECTED);
        return;
    }

    token = PeekNextToken();
    ExpressionModel expr3 = ParseExpression(model);
    CheckExpressionNotEmpty(model, token, expr3);
    model.args.push_back(expr3);

    token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    Error(model, token, MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseGotoGosub(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeNumber)
    {
        Error(model, token, "Line number expected.");
        return;
    }
    model.paramline = atoi(token.text.c_str());

    token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    Error(model, token, MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseIf(SourceLineModel& model)
{
    Token token = PeekNextToken();
    ExpressionModel expr = ParseExpression(model);
    CheckExpressionNotEmpty(model, token, expr);

    //TODO

    SkipTilEnd();//STUB
}

//TODO: LET MID$

void Parser::ParseLet(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeIdentifier)
    {
        Error(model, token, "Identifier expected.");
        return;
    }

    ParseLetShort(token, model);
}

void Parser::ParseLetShort(Token& tokenIdentOrMid, SourceLineModel& model)
{
    model.ident = tokenIdentOrMid;

    Token token;
    if (tokenIdentOrMid.type == TokenTypeIdentifier)
    {
        VariableModel var;
        var.name = tokenIdentOrMid.text;  //TODO: canonic form
        //TODO: Check for open bracket, parse variable indices
        model.variables.push_back(var);

        token = GetNextTokenSkipDivider();
        if (token.IsOpenBracket())  // Open bracket - read array indices
        {
            token = GetNextTokenSkipDivider();
            //TODO

            //TODO: Close bracket
            token = GetNextTokenSkipDivider();

            token = GetNextTokenSkipDivider();  // get token after the close bracket
        }
    }
    else if (tokenIdentOrMid.type == TokenTypeKeyword && tokenIdentOrMid.keyword == KeywordMID)
    {
        token = GetNextTokenSkipDivider();
        if (!token.IsOpenBracket())
        {
            Error(model, token, MSG_OPEN_BRACKET_EXPECTED);
            return;
        }

        token = GetNextTokenSkipDivider();
        if (token.type != TokenTypeIdentifier)
        {
            Error(model, token, "Identifier expected.");
            return;
        }
        VariableModel var;
        var.name = token.text;  //TODO: canonic form
        //TODO: Check for open bracket, parse variable indices
        model.variables.push_back(var);

        SkipComma(model);

        token = GetNextTokenSkipDivider();
        if (token.type != TokenTypeNumber || !token.IsDValueInteger())
        {
            Error(model, token, "Integer argument expected.");
            return;
        }
        model.params.push_back(token);

        SkipComma(model);

        token = GetNextTokenSkipDivider();
        if (token.type != TokenTypeNumber || !token.IsDValueInteger())
        {
            Error(model, token, "Integer argument expected.");
            return;
        }
        model.params.push_back(token);

        token = GetNextTokenSkipDivider();
        if (!token.IsCloseBracket())
        {
            Error(model, token, MSG_CLOSE_BRACKET_EXPECTED);
            return;
        }

        token = GetNextTokenSkipDivider();
    }

    if (!token.IsEqualSign())
    {
        Error(model, token, "Equal sign (\'=\') expected.");
        return;
    }

    ExpressionModel expr = ParseExpression(model);
    CheckExpressionNotEmpty(model, token, expr);
    model.args.push_back(expr);

    token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    Error(model, token, MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseLocate(SourceLineModel& model)
{
    Token token = PeekNextTokenSkipDivider();
    if (token.IsEolOrEof())
    {
        Error(model, token, MSG_ARGUMENTS_EXPECTED);
        return;
    }

    ExpressionModel expr1 = ParseExpression(model);
    model.args.push_back(expr1);
    CheckExpressionNotEmpty(model, token, expr1);

    SkipComma(model);

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr2 = ParseExpression(model);
    model.args.push_back(expr2);
    CheckExpressionNotEmpty(model, token, expr2);

    SkipComma(model);

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr3 = ParseExpression(model);
    model.args.push_back(expr3);
    CheckExpressionNotEmpty(model, token, expr3);

    token = PeekNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    Error(model, token, MSG_UNEXPECTED_AT_END_OF_STATEMENT);
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
            Error(model, token, "Identifier expected.");
            return;
        }
        //TODO: Check for numeric variable

        model.params.push_back(token);

        token = GetNextTokenSkipDivider();
        if (token.IsEolOrEof())
            break;

        if (token.type != TokenTypeSymbol || token.symbol != ',')
        {
            Error(model, token, "Comma or end of line expected.");
            return;
        }

        token = GetNextTokenSkipDivider();
    }
}

void Parser::ParseOn(SourceLineModel& model)
{
    Token token = PeekNextTokenSkipDivider();
    ExpressionModel expr = ParseExpression(model);
    CheckExpressionNotEmpty(model, token, expr);

    token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeKeyword || (token.keyword != KeywordGOTO && token.keyword != KeywordGOSUB))
    {
        Error(model, token, "GOTO or GOSUB expected.");
        return;
    }
    model.gotogosub = (token.keyword == KeywordGOTO);

    // Loop for line numbers, comma separated
    while (true)
    {
        token = PeekNextTokenSkipDivider();
        if (token.type != TokenTypeNumber)
        {
            Error(model, token, "Line number expected.");
            return;
        }
        token = GetNextToken();
        model.params.push_back(token);

        token = PeekNextTokenSkipDivider();
        if (token.IsEolOrEof())
            break;
        if (!token.IsComma())
        {
            Error(model, token, MSG_UNEXPECTED);
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
        Error(model, token, MSG_ARGUMENTS_EXPECTED);
        return;
    }

    ExpressionModel expr1 = ParseExpression(model);
    model.args.push_back(expr1);
    CheckExpressionNotEmpty(model, token, expr1);

    SkipComma(model);

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr2 = ParseExpression(model);
    model.args.push_back(expr2);
    CheckExpressionNotEmpty(model, token, expr2);

    SkipComma(model);

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr3 = ParseExpression(model);
    model.args.push_back(expr3);
    CheckExpressionNotEmpty(model, token, expr3);

    token = PeekNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    Error(model, token, MSG_UNEXPECTED_AT_END_OF_STATEMENT);
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
        token = PeekNextTokenSkipDivider();
        if (token.type == TokenTypeKeyword && token.keyword == KeywordAT)
        {
            ExpressionNode node0;
            node0.node = GetNextToken();  // AT keyword

            token = GetNextTokenSkipDivider();
            if (!token.IsOpenBracket())
            {
                Error(model, token, MSG_OPEN_BRACKET_EXPECTED);
                return;
            }

            ExpressionModel expr1 = ParseExpression(model);
            CheckExpressionNotEmpty(model, token, expr1);

            token = GetNextTokenSkipDivider();
            if (!token.IsComma())
            {
                Error(model, token, MSG_COMMA_EXPECTED);
                return;
            }

            ExpressionModel expr2 = ParseExpression(model);
            CheckExpressionNotEmpty(model, token, expr2);

            token = GetNextTokenSkipDivider();
            if (!token.IsCloseBracket())
            {
                Error(model, token, MSG_CLOSE_BRACKET_EXPECTED);
                return;
            }

            node0.args.push_back(expr1);
            node0.args.push_back(expr2);

            ExpressionModel expr0;
            expr0.nodes.push_back(node0);
            expr0.root = 0;

            model.args.push_back(expr0);

            continue;
        }
        if (token.type == TokenTypeKeyword && token.keyword == KeywordTAB)
        {
            ExpressionNode node0;
            node0.node = GetNextToken();  // TAB keyword

            token = GetNextTokenSkipDivider();
            if (!token.IsOpenBracket())
            {
                Error(model, token, MSG_OPEN_BRACKET_EXPECTED);
                return;
            }

            ExpressionModel expr1 = ParseExpression(model);
            CheckExpressionNotEmpty(model, token, expr1);

            token = GetNextTokenSkipDivider();
            if (!token.IsCloseBracket())
            {
                Error(model, token, MSG_CLOSE_BRACKET_EXPECTED);
                return;
            }

            node0.args.push_back(expr1);

            ExpressionModel expr0;
            expr0.nodes.push_back(node0);
            expr0.root = 0;

            model.args.push_back(expr0);

            continue;
        }
        if (token.type == TokenTypeKeyword && token.keyword == KeywordSPC)
        {
            ExpressionNode node0;
            node0.node = GetNextToken();  // SPC keyword
            
            token = GetNextTokenSkipDivider();
            if (!token.IsOpenBracket())
            {
                Error(model, token, MSG_OPEN_BRACKET_EXPECTED);
                return;
            }

            ExpressionModel expr1 = ParseExpression(model);
            CheckExpressionNotEmpty(model, token, expr1);

            token = GetNextTokenSkipDivider();
            if (!token.IsCloseBracket())
            {
                Error(model, token, MSG_CLOSE_BRACKET_EXPECTED);
                return;
            }

            node0.args.push_back(expr1);

            ExpressionModel expr0;
            expr0.nodes.push_back(node0);
            expr0.root = 0;

            model.args.push_back(expr0);

            continue;
        }

        ExpressionModel expr = ParseExpression(model);
        model.args.push_back(expr);

        token = GetNextTokenSkipDivider();
        if (token.IsEolOrEof())
            return;
        if (!token.IsComma() && !token.IsSemicolon())
        {
            Error(model, token, "Comma or semicolon expected.");
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
    CheckExpressionNotEmpty(model, token, expr1);
    model.args.push_back(expr1);

    SkipComma(model);

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr2 = ParseExpression(model);
    model.args.push_back(expr2);
    CheckExpressionNotEmpty(model, token, expr2);

    token = GetNextTokenSkipDivider();
    if (!token.IsEolOrEof())
        Error(model, token, MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParsePsetPreset(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if (token.type == TokenTypeSymbol && token.symbol == '@' ||
        token.type == TokenTypeKeyword && token.keyword == KeywordSTEP)
    {
        model.relative = true;
        token = GetNextTokenSkipDivider();
    }

    if (!token.IsOpenBracket())
    {
        Error(model, token, MSG_OPEN_BRACKET_EXPECTED);
        return;
    }

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr1 = ParseExpression(model);
    CheckExpressionNotEmpty(model, token, expr1);
    model.args.push_back(expr1);

    SkipComma(model);

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr2 = ParseExpression(model);
    model.args.push_back(expr2);
    CheckExpressionNotEmpty(model, token, expr2);

    token = GetNextTokenSkipDivider();
    if (!token.IsCloseBracket())
    {
        Error(model, token, MSG_CLOSE_BRACKET_EXPECTED);
        return;
    }

    token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;
    if (!token.IsComma())
    {
        Error(model, token, "Unexpected text after arguments.");
        return;
    }

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr3 = ParseExpression(model);
    model.args.push_back(expr3);
    CheckExpressionNotEmpty(model, token, expr3);

    token = GetNextTokenSkipDivider();
    if (!token.IsEolOrEof())
        Error(model, token, MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseRead(SourceLineModel& model)
{
    Token token;
    while (true)
    {
        token = GetNextTokenSkipDivider();
        if (token.type != TokenTypeIdentifier)
        {
            Error(model, token, "Identifier expected.");
            return;
        }
        token = GetNextTokenSkipDivider();
        if (token.IsOpenBracket())
        {
            while (true)
            {
                ExpressionModel expr = ParseExpression(model);
                CheckExpressionNotEmpty(model, token, expr);
                //TODO: save to model

                token = GetNextTokenSkipDivider();
                if (token.IsCloseBracket())
                    break;
                if (!token.IsComma())
                {
                    Error(model, token, MSG_COMMA_EXPECTED);
                    return;
                }
            }

            token = GetNextTokenSkipDivider();
        }
        if (token.IsEolOrEof())
            return;  // End of the list

        if (!token.IsComma())
        {
            Error(model, token, MSG_COMMA_EXPECTED);
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
        Error(model, token, "Numeric argument expected.");
        return;
    }
    model.paramline = (int)token.dvalue;

    token = GetNextTokenSkipDivider();
    if (!token.IsEolOrEof())
        Error(model, token, MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseScreen(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeNumber)
    {
        Error(model, token, "Numeric argument expected.");
        return;
    }

    model.params.push_back(token);

    token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    Error(model, token, "Unexpected text after SCREEN argument.");
}

void Parser::ParseWidth(SourceLineModel& model)
{
    //TODO

    SkipTilEnd();
}

//////////////////////////////////////////////////////////////////////
