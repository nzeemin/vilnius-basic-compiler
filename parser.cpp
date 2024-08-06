
#include <cassert>
#include <iomanip>

#include "main.h"


//////////////////////////////////////////////////////////////////////


const ParserKeywordSpec Parser::m_keywordspecs[] =
{
    { KeywordBEEP,      &Parser::ParseStatementNoParams },
    { KeywordBLOAD,     &Parser::ParseIgnoredStatement },
    { KeywordBSAVE,     &Parser::ParseIgnoredStatement },
    { KeywordCLEAR,     &Parser::ParseClear },
    { KeywordCLOAD,     &Parser::ParseIgnoredStatement },
    { KeywordCLOSE,     &Parser::ParseStatementNoParams },
    { KeywordCLS,       &Parser::ParseStatementNoParams },
    { KeywordCOLOR,     &Parser::ParseColor },
    { KeywordCSAVE,     &Parser::ParseIgnoredStatement },
    { KeywordDATA,      &Parser::ParseData },
    { KeywordDEF,       &Parser::ParseDef },
    { KeywordDIM,       &Parser::ParseDim },
    { KeywordKEY,       &Parser::ParseKey },
    { KeywordDRAW,      &Parser::ParseDraw },
    { KeywordEND,       &Parser::ParseStatementNoParams },
    { KeywordFOR,       &Parser::ParseFor },
    { KeywordGOSUB,     &Parser::ParseGotoGosub },
    { KeywordGOTO,      &Parser::ParseGotoGosub },
    { KeywordIF,        &Parser::ParseIf },
    { KeywordINPUT,     &Parser::ParseInput },
    { KeywordLET,       &Parser::ParseLet },
    { KeywordLOAD,      &Parser::ParseIgnoredStatement },
    { KeywordLOCATE,    &Parser::ParseLocate },
    { KeywordNEXT,      &Parser::ParseNext },
    { KeywordON,        &Parser::ParseOn },
    { KeywordOPEN,      &Parser::ParseOpen },
    { KeywordOUT,       &Parser::ParseOut },
    { KeywordPOKE,      &Parser::ParsePoke },
    { KeywordPSET,      &Parser::ParsePsetPreset },
    { KeywordPRESET,    &Parser::ParsePsetPreset },
    { KeywordLINE,      &Parser::ParseLine },
    { KeywordCIRCLE,    &Parser::ParseCircle },
    { KeywordPAINT,     &Parser::ParsePaint },
    { KeywordPRINT,     &Parser::ParsePrint },
    { KeywordREAD,      &Parser::ParseRead },
    { KeywordREM,       &Parser::ParseRem },
    { KeywordRESTORE,   &Parser::ParseRestore },
    { KeywordRETURN,    &Parser::ParseStatementNoParams },
    { KeywordSAVE,      &Parser::ParseIgnoredStatement },
    { KeywordSCREEN,    &Parser::ParseScreen },
    { KeywordSTOP,      &Parser::ParseStatementNoParams },
    { KeywordTROFF,     &Parser::ParseStatementNoParams },
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
    { KeywordMID,       2, 3, ValueTypeString },
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
    { KeywordPOINT,     2, 2, ValueTypeInteger },
};

const char* MSG_UNEXPECTED = "Unexpected text.";
const char* MSG_UNEXPECTED_AT_END_OF_STATEMENT = "Unexpected text at the end of the statement.";
const char* MSG_EXPRESSION_SHOULDNOT_BE_EMPTY = "Expression should not be empty.";
const char* MSG_COMMA_EXPECTED = "Comma expected.";
const char* MSG_OPEN_BRACKET_EXPECTED = "Open bracket expected.";
const char* MSG_CLOSE_BRACKET_EXPECTED = "Close bracket expected.";
const char* MSG_ARGUMENTS_EXPECTED = "Arguments expected.";


const ParserFunctionSpec* Parser::FindFunctionSpec(KeywordIndex keyword)
{
    for (auto it = std::begin(m_funcspecs); it != std::end(m_funcspecs); ++it)
    {
        if (keyword == it->keyword)
            return it;
    }

    return nullptr;
}

Parser::Parser(Tokenizer* tokenizer)
{
    assert(tokenizer != nullptr);

    m_tokenizer = tokenizer;
    m_nexttoken.type = TokenTypeNone;
    m_havenexttoken = false;
    m_prevlinenum = 0;
    m_line = nullptr;
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
    m_line = &model;
    model.text = m_tokenizer->GetLineText();

    if (token.type == TokenTypeEOT)
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
            if (token.type == TokenTypeEOT)
            {
                model.number = 0;
                return model;
            }

            Error(token, "Unexpected text after empty line.");
            return model;
        }
    }

    if (token.type != TokenTypeNumber)
    {
        Error(token, "Line number not found.");
        return model;
    }
    model.number = atoi(token.text.c_str());
    if (model.number <= 0 || model.number > MAX_LINE_NUMBER)
    {
        Error(token, "Line number is out of valid range.");
        SkipTilEnd();
        return model;
    }
    if (model.number <= m_prevlinenum)
    {
        Error(token, "Line number is incorrect.");
        SkipTilEnd();
        return model;
    }
    m_prevlinenum = model.number;

    ParseStatement(model.statement);

    token = PeekNextTokenSkipDivider();
    if (token.IsEolOrEof())
        GetNextToken();

    return model;
}

void Parser::ParseStatement(StatementModel& statement)
{
    Token token = PeekNextTokenSkipDivider();

    if (token.type == TokenTypeEndComment)  // REM short form
    {
        GetNextToken();  // get after peek
        Token tokenrem;
        tokenrem.keyword = KeywordREM;
        statement.token = tokenrem;
        return;  // Empty line with end-line comment
    }
    if (token.type == TokenTypeSymbol && token.symbol == '?')  // PRINT short form
    {
        GetNextToken();  // get after peek
        Token tokenprint;
        tokenprint.type = TokenTypeKeyword;
        tokenprint.keyword = KeywordPRINT;
        statement.token = tokenprint;

        ParsePrint(statement);
        if (m_line->error)
            SkipTilEnd();
        return;
    }
    if (token.type == TokenTypeIdentifier)  // LET without the keyword
    {
        Token tokenlet;
        tokenlet.type = TokenTypeKeyword;
        tokenlet.keyword = KeywordLET;
        statement.token = tokenlet;

        ParseLetShort(token, statement);
        if (m_line->error)
            SkipTilEnd();
        return;
    }
    if (token.IsKeyword(KeywordMID))
    {
        Token tokenlet;
        tokenlet.type = TokenTypeKeyword;
        tokenlet.keyword = KeywordLET;
        statement.token = tokenlet;

        ParseLetShort(token, statement);
        if (m_line->error)
            SkipTilEnd();
        return;
    }

    GetNextToken();  // get after peek

    if (token.type != TokenTypeKeyword)
    {
        Error(token, "Statement keyword expected.");
        return;
    }
    if (IsFunctionKeyword(token.keyword))
    {
        Error(token, "Statement keyword expected, function keyword found.");
        return;
    }

    statement.token = token;

    // Find keyword parser implementation
    ParseMethodRef methodref = nullptr;
    for (auto it = std::begin(m_keywordspecs); it != std::end(m_keywordspecs); ++it)
    {
        if (token.keyword == it->keyword)
        {
            methodref = it->methodref;
            break;
        }
    }
    if (methodref == nullptr)
    {
        Error(token, "Parser not found for keyword " + token.text + ".");
        SkipTilEnd();
        return;
    }

    (this->*methodref)(statement);

    if (m_line->error)
        SkipTilEnd();
}

void Parser::Error(const Token& token, const string& message)
{
    assert(m_line != nullptr);
    std::cerr << "ERROR at " << token.line << ":" << token.pos << " line " << m_line->number << " - " << message << std::endl;
    const string& linetext = m_line->text;
    if (!linetext.empty())
    {
        std::cerr << linetext << std::endl;
        std::cerr << std::right << std::setw(token.pos) << "^" << std::endl;
    }
    m_line->error = true;
    RegisterError();
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

void Parser::SkipTilStatementEnd()
{
    while (true)  // Skip til EOL/EOF
    {
        Token token = PeekNextToken();
        if (token.IsEndOfStatement())
            break;
        GetNextToken();
    }
}

void Parser::SkipComma()
{
    Token token = GetNextTokenSkipDivider();
    if (!token.IsComma())
    {
        Error(token, MSG_COMMA_EXPECTED);
        return;
    }
}

ExpressionModel Parser::ParseExpression()
{
    ExpressionModel expression;
    expression.root = -1;  // Empty expression for now

    bool isop = false;  // Currently on operation or not
    int prev = -1;  // Index of previous operation

    Token token = PeekNextTokenSkipDivider();
    if (token.IsEndOfExpression())
        return expression;  // Empty expression

    // Check if we have unary plus/minus sign or NOT operation
    if (token.type == TokenTypeOperation && (token.text == "+" || token.text == "-" || token.text == "NOT"))
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
                break;  // End of expression: we have something unknown here

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
                Error(token, "Operand expected in expression.");
                return expression;
            }

            //TODO: Check if we have unary plus/minus sign

            if (token.IsBinaryOperation())
            {
                Error(token, "Binary operation is not expected here.");
                return expression;
            }

            token = GetNextToken();  // get the token we peeked

            int index = -1;  // Index of the new node/sub-tree
            if (token.IsOpenBracket())  // Do recursion for expression inside brackets
            {
                ExpressionModel exprin = ParseExpression();

                if (exprin.IsEmpty())
                {
                    Error(token, "Expression in brackets should not be empty.");
                    return expression;
                }

                // Move expression nodes in the list
                int shift = (int)expression.nodes.size();
                for (size_t i = 0; i < exprin.nodes.size(); i++)
                {
                    ExpressionNode& node = exprin.nodes[i];
                    if ((int)i == exprin.root)
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
                    Error(token, "Close bracket expected in expression.");
                    return expression;
                }

                index = exprin.root + shift;
            }
            else if (IsFunctionKeyword(token.keyword))  // Function with parameter list
            {
                const ParserFunctionSpec* funcspec = FindFunctionSpec(token.keyword);
                assert(funcspec != nullptr);

                ExpressionNode node;
                node.node = token;
                node.vtype = funcspec->resulttype;

                token = PeekNextTokenSkipDivider();
                if (token.IsOpenBracket())  // Function parameter list
                {
                    if (funcspec->maxparams == 0)
                    {
                        Error(token, "This function should not have any parameters.");
                        return expression;
                    }

                    GetNextToken();  // open bracket

                    while (true)
                    {
                        ExpressionModel exprarg = ParseExpression();
                        node.args.push_back(exprarg);

                        token = PeekNextTokenSkipDivider();
                        if (token.IsCloseBracket())
                        {
                            GetNextToken();
                            break;
                        }
                        if (!token.IsComma())
                        {
                            Error(token, "Comma expected in function parameter list.");
                            return expression;
                        }

                        GetNextToken();  // comma
                    }
                }

                // Validate number of params for this function
                if ((int)node.args.size() < funcspec->minparams)
                {
                    Error(token, "Specified too few parameters for this function.");
                    return expression;
                }
                if ((int)node.args.size() > funcspec->maxparams)
                {
                    Error(token, "Specified too many parameters for this function.");
                    return expression;
                }

                index = (int)expression.nodes.size();
                expression.nodes.push_back(node);
            }
            else  // Other token like Ident, Number, String
            {
                // Put the token into the list
                ExpressionNode node;
                node.node = token;
                node.vtype = token.vtype;
                node.constval = (token.type == TokenTypeNumber || token.type == TokenTypeString);

                index = (int)expression.nodes.size();
                expression.nodes.push_back(node);

                if (token.type == TokenTypeIdentifier)
                {
                    token = PeekNextTokenSkipDivider();
                    if (token.IsOpenBracket())  // List of array indices
                    {
                        GetNextToken();  // open bracket
                        while (true)
                        {
                            token = PeekNextTokenSkipDivider();
                            ExpressionModel expri = ParseExpression();
                            if (m_line->error)
                                return expression;
                            if (expri.IsEmpty())
                            {
                                Error(token, "Expression should not be empty.");
                                return expression;
                            }
                            node.args.push_back(expri);

                            token = PeekNextTokenSkipDivider();
                            if (token.IsCloseBracket())
                            {
                                GetNextToken();  // close bracket
                                break;
                            }
                            if (!token.IsComma())
                            {
                                Error(token, MSG_COMMA_EXPECTED);
                                return expression;
                            }

                            GetNextToken();  // comma
                        }
                    }
                }
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

// Parse variable like "A", or variable with indices like "A(1,2)"
VariableModel Parser::ParseVariable()
{
    VariableModel var;
    Token token = PeekNextTokenSkipDivider();
    if (token.type != TokenTypeIdentifier)
    {
        Error(token, "Identifier expected.");
        return var;
    }
    token = GetNextToken();  // Identifier
    var.name = GetCanonicVariableName(token.text);

    token = PeekNextTokenSkipDivider();
    if (!token.IsOpenBracket())  // end of definition
        return var;

    // Parse array indices
    GetNextToken();  // Open bracket
    while (true)
    {
        token = GetNextTokenSkipDivider();
        if (token.type != TokenTypeNumber)
        {
            Error(token, "Array index expected.");
            return var;
        }
        if (!token.IsDValueInteger())
        {
            Error(token, "Array index should be an integer.");
            return var;
        }
        //TODO: Check for limits
        var.indices.push_back((int)token.dvalue);

        token = GetNextTokenSkipDivider();
        if (token.IsCloseBracket())
            break;
        if (!token.IsComma())
        {
            Error(token, MSG_COMMA_EXPECTED);
            return var;
        }
    }

    return var;
}

VariableExpressionModel Parser::ParseVariableExpression()
{
    VariableExpressionModel var;
    Token token = PeekNextTokenSkipDivider();
    if (token.type != TokenTypeIdentifier)
    {
        Error(token, "Identifier expected.");
        return var;
    }
    token = GetNextToken();  // Identifier
    var.name = GetCanonicVariableName(token.text);

    token = PeekNextTokenSkipDivider();
    if (!token.IsOpenBracket())  // end of definition
        return var;

    // Parse array indices
    GetNextToken();  // Open bracket
    while (true)
    {
        token = PeekNextTokenSkipDivider();
        ExpressionModel expr1 = ParseExpression();
        if (m_line->error)
            return var;
        if (expr1.IsEmpty())
        {
            Error(token, MSG_EXPRESSION_SHOULDNOT_BE_EMPTY);
            return var;
        }
        var.args.push_back(expr1);

        token = GetNextTokenSkipDivider();
        if (token.IsCloseBracket())
            break;
        if (!token.IsComma())
        {
            Error(token, MSG_COMMA_EXPECTED);
            return var;
        }
    }

    return var;
}

#define MODEL_ERROR(msg) \
    { Error(token, msg); return; }
#define CHECK_MODEL_ERROR \
    { if (m_line->error) return; }
#define CHECK_EXPRESSION_NOT_EMPTY(expr) \
    { if (expr.IsEmpty()) { Error(token, MSG_EXPRESSION_SHOULDNOT_BE_EMPTY); return; } }
#define SKIP_COMMA \
    { SkipComma(); if (m_line->error) return; }

void Parser::ParseIgnoredStatement(StatementModel& statement)
{
    SkipTilStatementEnd();
}

void Parser::ParseStatementNoParams(StatementModel& statement)
{
    Token token = PeekNextTokenSkipDivider();
    if (!token.IsEndOfStatement())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseClear(StatementModel& statement)
{
    Token token = PeekNextTokenSkipDivider();
    if (token.IsEndOfStatement())
        MODEL_ERROR("Argument expected.");

    ExpressionModel expr1 = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr1);
    statement.args.push_back(expr1);

    token = GetNextTokenSkipDivider();
    if (token.IsEndOfStatement())
        return;  // One argument

    if (!token.IsComma())
        MODEL_ERROR(MSG_UNEXPECTED);

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr2 = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr2);
    statement.args.push_back(expr2);

    token = PeekNextTokenSkipDivider();
    if (!token.IsEndOfStatement())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseColor(StatementModel& statement)
{
    Token token = PeekNextTokenSkipDivider();
    if (token.IsEndOfStatement())
        MODEL_ERROR(MSG_ARGUMENTS_EXPECTED);

    ExpressionModel expr1 = ParseExpression();
    CHECK_MODEL_ERROR;
    statement.args.push_back(expr1);

    token = GetNextTokenSkipDivider();
    if (token.IsEndOfStatement())
        return;
    if (!token.IsComma())
        MODEL_ERROR(MSG_UNEXPECTED);

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr2 = ParseExpression();
    CHECK_MODEL_ERROR;
    statement.args.push_back(expr2);

    token = GetNextTokenSkipDivider();
    if (token.IsEndOfStatement())
        return;
    if (!token.IsComma())
        MODEL_ERROR(MSG_UNEXPECTED);

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr3 = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr3);
    statement.args.push_back(expr3);

    token = PeekNextTokenSkipDivider();
    if (!token.IsEndOfStatement())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseData(StatementModel& statement)
{
    m_tokenizer->SetMode(TokenizerModeData);

    Token token;
    while (true)
    {
        token = GetNextTokenSkipDivider();
        if (token.type == TokenTypeOperation && token.text == "-")  // unary minus
        {
            token = GetNextToken();
            if (token.type != TokenTypeNumber)
                MODEL_ERROR("Number expected.");
            token.text.insert(0, "-");
            token.dvalue = -token.dvalue;  // invert sign
        }
        else if (token.type != TokenTypeNumber && token.type != TokenTypeString)
            MODEL_ERROR("Number or string expected.");
        statement.params.push_back(token);

        token = PeekNextTokenSkipDivider();
        if (!token.IsComma())
            break;
        GetNextToken();  // Comma
    }

    if (!token.IsEndOfStatement())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseDim(StatementModel& statement)
{
    Token token;
    while (true)
    {
        VariableModel var = ParseVariable();
        CHECK_MODEL_ERROR;
        statement.variables.push_back(var);

        token = PeekNextTokenSkipDivider();
        if (token.IsEndOfStatement())
            break;  // End of the list

        SKIP_COMMA;
    }
}

void Parser::ParseDraw(StatementModel& statement)
{
    Token token = PeekNextTokenSkipDivider();
    ExpressionModel expr1 = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr1);
    statement.args.push_back(expr1);

    token = PeekNextTokenSkipDivider();
    if (!token.IsEndOfStatement())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseFor(StatementModel& statement)
{
    Token token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeIdentifier)
        MODEL_ERROR("Identifier expected.");

    statement.ident = token;

    token = GetNextTokenSkipDivider();
    if (!token.IsEqualSign())
        MODEL_ERROR("Equal sign (\'=\') expected.");

    token = PeekNextToken();
    ExpressionModel expr1 = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr1);
    statement.args.push_back(expr1);

    token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeKeyword || token.keyword != KeywordTO)
        MODEL_ERROR("TO keyword expected.");

    token = PeekNextToken();
    ExpressionModel expr2 = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr2);
    statement.args.push_back(expr2);

    token = GetNextTokenSkipDivider();
    if (token.IsEndOfStatement())
        return;

    if (token.type != TokenTypeKeyword || token.keyword != KeywordSTEP)
        MODEL_ERROR(MSG_UNEXPECTED);

    token = PeekNextToken();
    ExpressionModel expr3 = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr3);
    statement.args.push_back(expr3);

    token = GetNextTokenSkipDivider();
    if (!token.IsEndOfStatement())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseGotoGosub(StatementModel& statement)
{
    Token token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeNumber)
        MODEL_ERROR("Line number expected.");
    statement.paramline = atoi(token.text.c_str());

    token = PeekNextTokenSkipDivider();
    if (!token.IsEndOfStatement())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

//NOTE: For now, in form: IF expr THEN linenum [ELSE linueum]
void Parser::ParseIf(StatementModel& statement)
{
    Token token = PeekNextTokenSkipDivider();
    ExpressionModel expr = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr);
    statement.args.push_back(expr);

    token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeKeyword || (token.keyword != KeywordTHEN && token.keyword != KeywordGOTO))
        MODEL_ERROR("Keyword THEN or GOTO expected.");
    bool isthen = (token.keyword == KeywordTHEN);

    token = PeekNextTokenSkipDivider();
    if (token.type == TokenTypeNumber)
    {
        GetNextToken();  // Number
        if (!token.IsDValueInteger())
            MODEL_ERROR("Line number must be an Integer.");
        statement.params.push_back(token);
    }
    else if (isthen)  // Statement under THEN
    {
        assert(statement.stthen == nullptr);
        statement.stthen = new StatementModel();
        ParseStatement(*statement.stthen);

        Token token0;
        statement.params.push_back(token0);  // stub for THEN line number
    }

    token = PeekNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;
    if (token.type != TokenTypeKeyword || token.keyword != KeywordELSE)
        MODEL_ERROR(MSG_UNEXPECTED);
    GetNextToken();  // ELSE

    token = PeekNextTokenSkipDivider();
    if (token.type == TokenTypeNumber)
    {
        GetNextToken();  // Number
        if (!token.IsDValueInteger())
            MODEL_ERROR("Line number must be an Integer.");
        statement.params.push_back(token);
    }
    else
    {
        assert(statement.stelse == nullptr);
        statement.stelse = new StatementModel();
        ParseStatement(*statement.stelse);
    }

    token = PeekNextTokenSkipDivider();
    if (!token.IsEndOfStatement())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseInput(StatementModel& statement)
{
    Token token = PeekNextTokenSkipDivider();
    if (token.type == TokenTypeSymbol && token.symbol == '#')  // File operation
    {
        GetNextToken();
        statement.fileoper = true;
    }
    else if (token.type == TokenTypeString)  // prompt string
    {
        GetNextToken();
        statement.params.push_back(token);
        token = GetNextTokenSkipDivider();
        if (token.type != TokenTypeSymbol || token.symbol != ';')
            MODEL_ERROR("Semicolon expected.");
    }

    while (true)
    {
        VariableModel var = ParseVariable();
        CHECK_MODEL_ERROR;
        statement.variables.push_back(var);

        token = PeekNextTokenSkipDivider();
        if (!token.IsComma())
            break;
        GetNextToken();  // comma
    }

    token = PeekNextTokenSkipDivider();
    if (!token.IsEndOfStatement())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseOpen(StatementModel& statement)
{
    Token token = PeekNextTokenSkipDivider();
    ExpressionModel expr1 = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr1);
    statement.args.push_back(expr1);

    token = GetNextTokenSkipDivider();
    if (token.IsKeyword(KeywordFOR))
    {
        token = GetNextTokenSkipDivider();
        if (token.IsKeyword(KeywordINPUT))
            statement.filemode = FileModeInput;
        else if (token.IsKeyword(KeywordOUTPUT))
            statement.filemode = FileModeOutput;
        else
            MODEL_ERROR(MSG_UNEXPECTED);

        token = GetNextTokenSkipDivider();
    }

    if (!token.IsEndOfStatement())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseKey(StatementModel& statement)
{
    Token token = PeekNextTokenSkipDivider();
    ExpressionModel expr1 = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr1);
    statement.args.push_back(expr1);

    SKIP_COMMA;

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr2 = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr2);
    statement.args.push_back(expr2);

    token = PeekNextTokenSkipDivider();
    if (!token.IsEndOfStatement())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseLet(StatementModel& statement)
{
    Token token = PeekNextTokenSkipDivider();
    if (token.type != TokenTypeIdentifier && (token.type != TokenTypeKeyword || token.keyword != KeywordMID))
        MODEL_ERROR("Identifier or MID$ expected.");

    ParseLetShort(token, statement);
}

//NOTE: We did only peek on tokenIdentOrMid, NOT get
void Parser::ParseLetShort(Token& tokenIdentOrMid, StatementModel& statement)
{
    statement.ident = tokenIdentOrMid;

    Token token;
    if (tokenIdentOrMid.type == TokenTypeIdentifier)
    {
        VariableExpressionModel var = ParseVariableExpression();
        CHECK_MODEL_ERROR;
        statement.varexprs.push_back(var);
    }
    else if (tokenIdentOrMid.IsKeyword(KeywordMID))
    {
        GetNextToken();  // MID$

        token = GetNextTokenSkipDivider();
        if (!token.IsOpenBracket())
            MODEL_ERROR(MSG_OPEN_BRACKET_EXPECTED);

        VariableExpressionModel var = ParseVariableExpression();
        CHECK_MODEL_ERROR;
        statement.varexprs.push_back(var);

        SKIP_COMMA;

        token = PeekNextTokenSkipDivider();
        ExpressionModel expr1 = ParseExpression();
        //TODO: Save to model

        SKIP_COMMA;

        token = GetNextTokenSkipDivider();
        if (token.type != TokenTypeNumber || !token.IsDValueInteger())
            MODEL_ERROR("Integer argument expected.");
        statement.params.push_back(token);

        token = GetNextTokenSkipDivider();
        if (!token.IsCloseBracket())
            MODEL_ERROR(MSG_CLOSE_BRACKET_EXPECTED);
    }

    token = GetNextTokenSkipDivider();
    if (!token.IsEqualSign())
        MODEL_ERROR("Equal sign (\'=\') expected.");

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr);
    statement.args.push_back(expr);

    token = PeekNextTokenSkipDivider();
    if (!token.IsEndOfStatement())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseLocate(StatementModel& statement)
{
    Token token = PeekNextTokenSkipDivider();
    if (token.IsEndOfStatement())
        MODEL_ERROR(MSG_ARGUMENTS_EXPECTED);

    ExpressionModel expr1 = ParseExpression();
    CHECK_MODEL_ERROR;
    statement.args.push_back(expr1);

    token = GetNextTokenSkipDivider();
    if (token.IsEndOfStatement())
        return;
    if (!token.IsComma())
        MODEL_ERROR(MSG_UNEXPECTED);

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr2 = ParseExpression();
    CHECK_MODEL_ERROR;
    statement.args.push_back(expr2);

    token = GetNextTokenSkipDivider();
    if (token.IsEndOfStatement())
        return;
    if (!token.IsComma())
        MODEL_ERROR(MSG_UNEXPECTED);

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr3 = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr3);
    statement.args.push_back(expr3);

    token = PeekNextTokenSkipDivider();
    if (!token.IsEndOfStatement())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseNext(StatementModel& statement)
{
    Token token = GetNextTokenSkipDivider();
    if (token.IsEndOfStatement())
        return;

    while (true)
    {
        if (token.type != TokenTypeIdentifier)
            MODEL_ERROR("Identifier expected.");

        statement.params.push_back(token);

        token = PeekNextTokenSkipDivider();
        if (token.IsEndOfStatement())
            break;
        GetNextToken();

        if (!token.IsComma())
            MODEL_ERROR("Comma or end of statement expected.");

        token = GetNextTokenSkipDivider();
    }
}

void Parser::ParseOn(StatementModel& statement)
{
    Token token = PeekNextTokenSkipDivider();
    if (token.IsEndOfStatement())
        MODEL_ERROR("Expreession expected.");

    ExpressionModel expr = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr);
    statement.args.push_back(expr);

    token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeKeyword || (token.keyword != KeywordGOTO && token.keyword != KeywordGOSUB))
        MODEL_ERROR("GOTO or GOSUB expected.");
    statement.gotogosub = (token.keyword == KeywordGOTO);

    // Loop for line numbers, comma separated
    while (true)
    {
        token = PeekNextTokenSkipDivider();
        if (token.type != TokenTypeNumber)
            MODEL_ERROR("Line number expected.");
        token = GetNextToken();
        statement.params.push_back(token);

        token = PeekNextTokenSkipDivider();
        if (token.IsEndOfStatement())
            break;
        GetNextToken();

        if (!token.IsComma())
            MODEL_ERROR(MSG_UNEXPECTED);
    }
}

void Parser::ParseOut(StatementModel& statement)
{
    Token token = PeekNextTokenSkipDivider();
    if (token.IsEndOfStatement())
        MODEL_ERROR(MSG_ARGUMENTS_EXPECTED);

    ExpressionModel expr1 = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr1);
    statement.args.push_back(expr1);

    SKIP_COMMA;

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr2 = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr2);
    statement.args.push_back(expr2);

    SKIP_COMMA;

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr3 = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr3);
    statement.args.push_back(expr3);

    token = PeekNextTokenSkipDivider();
    if (!token.IsEndOfStatement())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

//TODO: LPRINT
void Parser::ParsePrint(StatementModel& statement)
{
    Token token = PeekNextTokenSkipDivider();
    if (token.IsEndOfStatement())
        return;  // Empty PRINT

    //TODO: Symbol #, optional
    //TODO: Check for end

    while (true)
    {
        token = PeekNextTokenSkipDivider();
        if (token.IsEndOfStatement())
            break;

        if (token.IsKeyword(KeywordAT))
        {
            ExpressionNode node0;
            node0.node = GetNextToken();  // AT keyword

            token = GetNextTokenSkipDivider();
            if (!token.IsOpenBracket())
                MODEL_ERROR(MSG_OPEN_BRACKET_EXPECTED);

            ExpressionModel expr1 = ParseExpression();
            CHECK_MODEL_ERROR;
            CHECK_EXPRESSION_NOT_EMPTY(expr1);

            token = GetNextTokenSkipDivider();
            if (!token.IsComma())
                MODEL_ERROR(MSG_COMMA_EXPECTED);

            ExpressionModel expr2 = ParseExpression();
            CHECK_MODEL_ERROR;
            CHECK_EXPRESSION_NOT_EMPTY(expr2);

            token = GetNextTokenSkipDivider();
            if (!token.IsCloseBracket())
                MODEL_ERROR(MSG_CLOSE_BRACKET_EXPECTED);

            node0.args.push_back(expr1);
            node0.args.push_back(expr2);

            ExpressionModel expr0;
            expr0.nodes.push_back(node0);
            expr0.root = 0;

            statement.args.push_back(expr0);

            continue;
        }
        if (token.IsKeyword(KeywordTAB))
        {
            ExpressionNode node0;
            node0.node = GetNextToken();  // TAB keyword

            token = GetNextTokenSkipDivider();
            if (!token.IsOpenBracket())
                MODEL_ERROR(MSG_OPEN_BRACKET_EXPECTED);

            ExpressionModel expr1 = ParseExpression();
            CHECK_MODEL_ERROR;
            CHECK_EXPRESSION_NOT_EMPTY(expr1);

            token = GetNextTokenSkipDivider();
            if (!token.IsCloseBracket())
                MODEL_ERROR(MSG_CLOSE_BRACKET_EXPECTED);

            node0.args.push_back(expr1);

            ExpressionModel expr0;
            expr0.nodes.push_back(node0);
            expr0.root = 0;

            statement.args.push_back(expr0);

            continue;
        }
        if (token.IsKeyword(KeywordSPC))
        {
            ExpressionNode node0;
            node0.node = GetNextToken();  // SPC keyword
            
            token = GetNextTokenSkipDivider();
            if (!token.IsOpenBracket())
                MODEL_ERROR(MSG_OPEN_BRACKET_EXPECTED);

            ExpressionModel expr1 = ParseExpression();
            CHECK_MODEL_ERROR;
            CHECK_EXPRESSION_NOT_EMPTY(expr1);

            token = GetNextTokenSkipDivider();
            if (!token.IsCloseBracket())
                MODEL_ERROR(MSG_CLOSE_BRACKET_EXPECTED);

            node0.args.push_back(expr1);

            ExpressionModel expr0;
            expr0.nodes.push_back(node0);
            expr0.root = 0;

            statement.args.push_back(expr0);

            continue;
        }

        ExpressionModel expr = ParseExpression();
        CHECK_MODEL_ERROR;
        if (!expr.IsEmpty())
            statement.args.push_back(expr);

        token = PeekNextTokenSkipDivider();
        if (token.IsEndOfStatement())
            break;

        if (token.IsSemicolon())
        {
            GetNextToken();
            token = PeekNextTokenSkipDivider();
            if (token.IsEndOfStatement())
            {
                statement.nocrlf = true;  // Semicolon ends the PRINT means no CR/LF at the end
                break;
            }
        }
        else if (token.IsComma())
        {
            GetNextToken();

            // Add special expression with Comma as root
            ExpressionNode node0;
            node0.node = token;  // Comma
            ExpressionModel expr0;
            expr0.nodes.push_back(node0);
            expr0.root = 0;
            statement.args.push_back(expr0);
        }
    }
}

void Parser::ParsePoke(StatementModel& statement)
{
    Token token = PeekNextTokenSkipDivider();
    ExpressionModel expr1 = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr1);
    statement.args.push_back(expr1);

    SKIP_COMMA;

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr2 = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr2);
    statement.args.push_back(expr2);

    token = PeekNextTokenSkipDivider();
    if (!token.IsEndOfStatement())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParsePsetPreset(StatementModel& statement)
{
    Token token = GetNextTokenSkipDivider();
    if ((token.type == TokenTypeSymbol && token.symbol == '@') ||
        (token.IsKeyword(KeywordSTEP)))
    {
        statement.relative = true;
        token = GetNextTokenSkipDivider();
    }

    if (!token.IsOpenBracket())
        MODEL_ERROR(MSG_OPEN_BRACKET_EXPECTED);

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr1 = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr1);
    statement.args.push_back(expr1);

    SKIP_COMMA;

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr2 = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr2);
    statement.args.push_back(expr2);

    token = GetNextTokenSkipDivider();
    if (!token.IsCloseBracket())
        MODEL_ERROR(MSG_CLOSE_BRACKET_EXPECTED);

    token = PeekNextTokenSkipDivider();
    if (token.IsEndOfStatement())
        return;

    SKIP_COMMA;

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr3 = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr3);
    statement.args.push_back(expr3);

    token = PeekNextTokenSkipDivider();
    if (!token.IsEndOfStatement())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseLine(StatementModel& statement)
{
    Token token = PeekNextTokenSkipDivider();
    if ((token.type == TokenTypeSymbol && token.symbol == '@') ||
        (token.IsKeyword(KeywordSTEP)))
    {
        GetNextToken();  // @
        statement.relative = true;
    }

    token = PeekNextTokenSkipDivider();
    if (token.IsOpenBracket())  // we have ARG1, ARG2
    {
        GetNextToken();  // open bracket
        token = PeekNextTokenSkipDivider();
        ExpressionModel expr1 = ParseExpression();
        CHECK_MODEL_ERROR;
        CHECK_EXPRESSION_NOT_EMPTY(expr1);
        statement.args.push_back(expr1);

        SKIP_COMMA;

        token = PeekNextTokenSkipDivider();
        ExpressionModel expr2 = ParseExpression();
        CHECK_MODEL_ERROR;
        CHECK_EXPRESSION_NOT_EMPTY(expr2);
        statement.args.push_back(expr2);

        token = GetNextTokenSkipDivider();
        if (!token.IsCloseBracket())
            MODEL_ERROR(MSG_CLOSE_BRACKET_EXPECTED);
    }
    else
    {
        //TODO: add two empty expressions
    }

    token = PeekNextTokenSkipDivider();
    if (token.type != TokenTypeOperation || token.text != "-")
        MODEL_ERROR("Minus \'-\' sign expected.");
    GetNextToken();  // minus sign

    token = PeekNextTokenSkipDivider();
    if ((token.type == TokenTypeSymbol && token.symbol == '@') ||
        (token.IsKeyword(KeywordSTEP)))
    {
        GetNextToken();  // @
        //model.relative = true;//TODO
    }

    token = PeekNextTokenSkipDivider();
    if (token.IsOpenBracket())  // we have ARG3, ARG4
    {
        GetNextToken();  // open bracket

        token = PeekNextTokenSkipDivider();
        ExpressionModel expr3 = ParseExpression();
        CHECK_MODEL_ERROR;
        CHECK_EXPRESSION_NOT_EMPTY(expr3);
        statement.args.push_back(expr3);

        SKIP_COMMA;

        token = PeekNextTokenSkipDivider();
        ExpressionModel expr4 = ParseExpression();
        CHECK_MODEL_ERROR;
        CHECK_EXPRESSION_NOT_EMPTY(expr4);
        statement.args.push_back(expr4);

        token = GetNextTokenSkipDivider();
        if (!token.IsCloseBracket())
            MODEL_ERROR(MSG_CLOSE_BRACKET_EXPECTED);
    }
    else
    {
        //TODO: add two empty expressions
    }

    token = PeekNextTokenSkipDivider();
    if (token.IsEndOfStatement())
        return;

    if (token.IsComma())  // we have ARG5
    {
        GetNextToken();  // comma

        token = PeekNextTokenSkipDivider();
        ExpressionModel expr5 = ParseExpression();
        CHECK_MODEL_ERROR;
        CHECK_EXPRESSION_NOT_EMPTY(expr5);
        statement.args.push_back(expr5);

        token = PeekNextTokenSkipDivider();
        if (token.IsComma())  // we have "B" or "BF" here
        {
            GetNextToken();  // comma

            token = GetNextTokenSkipDivider();
            if (token.type != TokenTypeIdentifier || (token.text != "B" && token.text != "BF"))
                MODEL_ERROR("\'B\' or \'BR\' expected.");

            //TODO: save to model
        }
    }

    token = PeekNextTokenSkipDivider();
    if (!token.IsEndOfStatement())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseCircle(StatementModel& statement)
{
    Token token = PeekNextTokenSkipDivider();
    if ((token.type == TokenTypeSymbol && token.symbol == '@') ||
        (token.IsKeyword(KeywordSTEP)))
    {
        GetNextToken();  // @ or STEP
        statement.relative = true;
    }

    token = GetNextTokenSkipDivider();
    if (!token.IsOpenBracket())
        MODEL_ERROR(MSG_OPEN_BRACKET_EXPECTED);

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr1 = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr1);
    statement.args.push_back(expr1);

    SKIP_COMMA;

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr2 = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr2);
    statement.args.push_back(expr2);

    token = GetNextTokenSkipDivider();
    if (!token.IsCloseBracket())
        MODEL_ERROR(MSG_CLOSE_BRACKET_EXPECTED);

    SKIP_COMMA;

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr3 = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr3);
    statement.args.push_back(expr3);

    token = PeekNextTokenSkipDivider();
    if (token.IsEndOfStatement())
        return;

    if (token.IsComma())
    {
        GetNextToken();  // comma

        token = PeekNextTokenSkipDivider();
        ExpressionModel expr4 = ParseExpression();
        CHECK_MODEL_ERROR;
        statement.args.push_back(expr4);

        token = PeekNextTokenSkipDivider();
        if (token.IsComma())
        {
            GetNextToken();  // comma

            token = PeekNextTokenSkipDivider();
            ExpressionModel expr5 = ParseExpression();
            CHECK_MODEL_ERROR;
            statement.args.push_back(expr5);

            token = PeekNextTokenSkipDivider();
            if (token.IsComma())
            {
                GetNextToken();  // comma

                token = PeekNextTokenSkipDivider();
                ExpressionModel expr6 = ParseExpression();
                CHECK_MODEL_ERROR;
                statement.args.push_back(expr6);

                token = PeekNextTokenSkipDivider();
                if (token.IsComma())
                {
                    GetNextToken();  // comma

                    token = PeekNextTokenSkipDivider();
                    ExpressionModel expr7 = ParseExpression();
                    CHECK_MODEL_ERROR;
                    statement.args.push_back(expr7);
                }
            }
        }
    }
    
    token = PeekNextTokenSkipDivider();
    if (!token.IsEndOfStatement())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParsePaint(StatementModel& statement)
{
    Token token = PeekNextTokenSkipDivider();
    if ((token.type == TokenTypeSymbol && token.symbol == '@') ||
        (token.IsKeyword(KeywordSTEP)))
    {
        GetNextToken();  // @ or STEP
        statement.relative = true;
    }

    token = GetNextTokenSkipDivider();
    if (!token.IsOpenBracket())
        MODEL_ERROR(MSG_OPEN_BRACKET_EXPECTED);

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr1 = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr1);
    statement.args.push_back(expr1);

    SKIP_COMMA;

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr2 = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr2);
    statement.args.push_back(expr2);

    token = GetNextTokenSkipDivider();
    if (!token.IsCloseBracket())
        MODEL_ERROR(MSG_CLOSE_BRACKET_EXPECTED);

    token = PeekNextTokenSkipDivider();
    if (token.IsComma())
    {
        GetNextToken();  // comma

        token = PeekNextTokenSkipDivider();
        ExpressionModel expr3 = ParseExpression();
        CHECK_MODEL_ERROR;
        statement.args.push_back(expr3);

        token = PeekNextTokenSkipDivider();
        if (token.IsComma())
        {
            GetNextToken();  // comma

            token = PeekNextTokenSkipDivider();
            ExpressionModel expr4 = ParseExpression();
            CHECK_MODEL_ERROR;
            statement.args.push_back(expr4);
        }
    }

    token = PeekNextTokenSkipDivider();
    if (!token.IsEndOfStatement())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseRead(StatementModel& statement)
{
    Token token;
    while (true)
    {
        VariableExpressionModel var = ParseVariableExpression();
        CHECK_MODEL_ERROR;
        statement.varexprs.push_back(var);

        token = PeekNextTokenSkipDivider();
        if (token.IsEndOfStatement())
            return;  // End of the list
        
        SKIP_COMMA;
    }
}

void Parser::ParseRem(StatementModel& statement)
{
    SkipTilEnd();
}

void Parser::ParseRestore(StatementModel& statement)
{
    Token token = PeekNextTokenSkipDivider();
    if (token.IsEndOfStatement())
    {
        statement.paramline = 0;
        return;  // RESTORE without parameters
    }
    GetNextToken();
    if (token.type != TokenTypeNumber)
        MODEL_ERROR("Numeric argument expected.");
    //TODO: Check for valid range
    statement.paramline = (int)token.dvalue;

    token = PeekNextTokenSkipDivider();
    if (!token.IsEndOfStatement())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseDef(StatementModel& statement)
{
    Token token = GetNextTokenSkipDivider();
    if (token.IsKeyword(KeywordFN))  // DEF FN
        ParseDefFn(statement);
    else if (token.IsKeyword(KeywordUSR))  // DEF USR
        ParseDefUsr(statement);
    else
        MODEL_ERROR("\'FN\' or \'USR\' expected.");
}

void Parser::ParseDefFn(StatementModel& statement)
{
    statement.deffnorusr = true;

    Token token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeIdentifier)
        MODEL_ERROR("Identifier expected.");
    statement.ident = token;

    token = GetNextTokenSkipDivider();
    if (token.IsOpenBracket())  // Parse optional parameters
    {
        while (true)
        {
            token = GetNextTokenSkipDivider();
            if (token.type != TokenTypeIdentifier)
                MODEL_ERROR("Identifier expected.");
            statement.params.push_back(token);

            token = GetNextTokenSkipDivider();
            if (!token.IsComma())
                break;
        }

        if (!token.IsCloseBracket())
            MODEL_ERROR(MSG_CLOSE_BRACKET_EXPECTED);

        token = GetNextTokenSkipDivider();
    }

    if (!token.IsEqualSign())
        MODEL_ERROR("Equal sign expected.");

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr);
    statement.args.push_back(expr);

    token = PeekNextTokenSkipDivider();
    if (!token.IsEndOfStatement())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseDefUsr(StatementModel& statement)
{
    statement.deffnorusr = false;

    int usrnumber = 0;
    Token token = GetNextToken();
    if (token.type == TokenTypeNumber)
    {
        usrnumber = atoi(token.text.c_str());
    }
    else if (token.type != TokenTypeDivider)
        MODEL_ERROR(MSG_UNEXPECTED);
    statement.paramline = usrnumber;

    token = GetNextTokenSkipDivider();
    if (!token.IsEqualSign())
        MODEL_ERROR("Equal sign expected.");

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr = ParseExpression();
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr);
    statement.args.push_back(expr);

    token = PeekNextTokenSkipDivider();
    if (!token.IsEndOfStatement())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseScreen(StatementModel& statement)
{
    Token token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeNumber)
        MODEL_ERROR("Numeric argument expected.");

    statement.params.push_back(token);

    token = PeekNextTokenSkipDivider();
    if (!token.IsEndOfStatement())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

// Undocumented instruction
// WIDTH <Integer>, [<Integer>]
void Parser::ParseWidth(StatementModel& statement)
{
    Token token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeNumber)
        MODEL_ERROR("Numeric argument expected.");
    statement.params.push_back(token);

    token = GetNextTokenSkipDivider();
    if (token.IsEndOfStatement())
        return;
    if (!token.IsComma())
        MODEL_ERROR(MSG_UNEXPECTED);

    token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeNumber)
        MODEL_ERROR("Numeric argument expected.");
    statement.params.push_back(token);

    token = PeekNextTokenSkipDivider();
    if (!token.IsEndOfStatement())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}


//////////////////////////////////////////////////////////////////////
