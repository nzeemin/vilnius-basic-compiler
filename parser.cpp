
#include <cassert>
#include <iomanip>

#include "main.h"


//////////////////////////////////////////////////////////////////////


const ParserKeywordSpec Parser::m_keywordspecs[] =
{
    { KeywordBEEP,      &Parser::ParseStatementNoParams },
    { KeywordCLEAR,     &Parser::ParseClear },
    { KeywordCLOSE,     &Parser::ParseStatementNoParams },
    { KeywordCLS,       &Parser::ParseStatementNoParams },
    { KeywordCOLOR,     &Parser::ParseColor },
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
        Error(model, token, "Line number is out of valid range.");
        return model;
    }
    if (model.number <= m_prevlinenum)
    {
        Error(model, token, "Line number is incorrect.");
        return model;
    }
    m_prevlinenum = model.number;

    token = PeekNextTokenSkipDivider();

    if (token.type == TokenTypeEndComment)  // REM short form
    {
        GetNextToken();  // get after peek
        Token tokenrem;
        tokenrem.keyword = KeywordREM;
        model.statement = tokenrem;
        return model;  // Empty line with end-line comment
    }
    if (token.type == TokenTypeSymbol && token.symbol == '?')  // PRINT short form
    {
        GetNextToken();  // get after peek
        Token tokenprint;
        tokenprint.type = TokenTypeKeyword;
        tokenprint.keyword = KeywordPRINT;
        model.statement = tokenprint;

        ParsePrint(model);
        if (model.error)
            SkipTilEnd();
        return model;
    }
    if (token.type == TokenTypeIdentifier)  // LET without the keyword
    {
        Token tokenlet;
        tokenlet.type = TokenTypeKeyword;
        tokenlet.keyword = KeywordLET;
        model.statement = tokenlet;

        ParseLetShort(token, model);
        if (model.error)
            SkipTilEnd();
        return model;
    }
    if (token.type == TokenTypeKeyword && token.keyword == KeywordMID)
    {
        Token tokenlet;
        tokenlet.type = TokenTypeKeyword;
        tokenlet.keyword = KeywordLET;
        model.statement = tokenlet;

        ParseLetShort(token, model);
        if (model.error)
            SkipTilEnd();
        return model;
    }

    GetNextToken();  // get after peek

    if (token.type != TokenTypeKeyword)
    {
        Error(model, token, "Statement keyword expected.");
        return model;
    }
    if (IsFunctionKeyword(token.keyword))
    {
        Error(model, token, "Statement keyword expected, function keyword found.");
        return model;
    }

    model.statement = token;

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
        Error(model, token, "Parser not found for keyword " + token.text + ".");
        SkipTilEnd();
        return model;
    }

    (this->*methodref)(model);

    if (model.error)
        SkipTilEnd();

    return model;
}

void Parser::Error(SourceLineModel& model, Token& token, string message)
{
    std::cerr << "ERROR at " << token.line << ":" << token.pos << " line " << model.number << " - " << message << std::endl;
    const string& linetext = model.text;
    if (!linetext.empty())
    {
        std::cerr << linetext << std::endl;
        std::cerr << std::right << std::setw(token.pos) << "^" << std::endl;
    }
    model.error = true;
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
                    Error(model, token, "Close bracket expected in expression.");
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
                if ((int)node.args.size() < funcspec->minparams)
                {
                    Error(model, token, "Specified too few parameters for this function.");
                    return expression;
                }
                if ((int)node.args.size() > funcspec->maxparams)
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

    return expression;
}

// Parse variable like "A", or variable with indices like "A(1,2)"
VariableModel Parser::ParseVariable(SourceLineModel& model)
{
    VariableModel var;
    Token token = PeekNextTokenSkipDivider();
    if (token.type != TokenTypeIdentifier)
    {
        Error(model, token, "Identifier expected.");
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
            Error(model, token, "Array index expected.");
            return var;
        }
        if (!token.IsDValueInteger())
        {
            Error(model, token, "Array index should be an integer.");
            return var;
        }
        //TODO: Check for limits
        var.indices.push_back((int)token.dvalue);

        token = GetNextTokenSkipDivider();
        if (token.IsCloseBracket())
            break;
        if (!token.IsComma())
        {
            Error(model, token, MSG_COMMA_EXPECTED);
            return var;
        }
    }

    return var;
}

#define MODEL_ERROR(msg) \
    { Error(model, token, msg); return; }
#define CHECK_MODEL_ERROR \
    { if (model.error) return; }
#define CHECK_EXPRESSION_NOT_EMPTY(expr) \
    { if (expr.IsEmpty()) { Error(model, token, MSG_EXPRESSION_SHOULDNOT_BE_EMPTY); return; } }
#define SKIP_COMMA \
    { SkipComma(model); if (model.error) return; }

void Parser::ParseStatementNoParams(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseClear(SourceLineModel& model)
{
    Token token = PeekNextTokenSkipDivider();
    if (token.IsEolOrEof())
        MODEL_ERROR("Argument expected.");

    ExpressionModel expr1 = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr1);
    model.args.push_back(expr1);

    token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;  // One argument

    if (!token.IsComma())
        MODEL_ERROR(MSG_UNEXPECTED);

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr2 = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr2);
    model.args.push_back(expr2);

    token = PeekNextTokenSkipDivider();
    if (!token.IsEolOrEof())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseColor(SourceLineModel& model)
{
    Token token = PeekNextTokenSkipDivider();
    if (token.IsEolOrEof())
        MODEL_ERROR(MSG_ARGUMENTS_EXPECTED);

    ExpressionModel expr1 = ParseExpression(model);
    CHECK_MODEL_ERROR;
    model.args.push_back(expr1);

    token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;
    if (!token.IsComma())
        MODEL_ERROR(MSG_UNEXPECTED);

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr2 = ParseExpression(model);
    CHECK_MODEL_ERROR;
    model.args.push_back(expr2);

    token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;
    if (!token.IsComma())
        MODEL_ERROR(MSG_UNEXPECTED);

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr3 = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr3);
    model.args.push_back(expr3);

    token = GetNextTokenSkipDivider();
    if (!token.IsEolOrEof())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseData(SourceLineModel& model)
{
    m_tokenizer->SetMode(TokenizerModeData);

    Token token;
    while (true)
    {
        ExpressionModel expr = ParseExpression(model);
        CHECK_MODEL_ERROR;
        model.args.push_back(expr);

        token = GetNextTokenSkipDivider();
        if (!token.IsComma())
            break;
    }

    if (!token.IsEolOrEof())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseDim(SourceLineModel& model)
{
    Token token;
    while (true)
    {
        VariableModel var = ParseVariable(model);
        CHECK_MODEL_ERROR;
        model.variables.push_back(var);

        token = GetNextTokenSkipDivider();
        if (token.IsEolOrEof())
            return;  // End of the list

        if (!token.IsComma())
            MODEL_ERROR(MSG_COMMA_EXPECTED);
    }
}

void Parser::ParseDraw(SourceLineModel& model)
{
    Token token = PeekNextTokenSkipDivider();
    ExpressionModel expr1 = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr1);
    model.args.push_back(expr1);

    token = GetNextTokenSkipDivider();
    if (!token.IsEolOrEof())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseFor(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeIdentifier)
        MODEL_ERROR("Identifier expected.");

    model.ident = token;

    token = GetNextTokenSkipDivider();
    if (!token.IsEqualSign())
        MODEL_ERROR("Equal sign (\'=\') expected.");

    token = PeekNextToken();
    ExpressionModel expr1 = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr1);
    model.args.push_back(expr1);

    token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeKeyword || token.keyword != KeywordTO)
        MODEL_ERROR("TO keyword expected.");

    token = PeekNextToken();
    ExpressionModel expr2 = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr2);
    model.args.push_back(expr2);

    token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    if (token.type != TokenTypeKeyword || token.keyword != KeywordSTEP)
        MODEL_ERROR(MSG_UNEXPECTED);

    token = PeekNextToken();
    ExpressionModel expr3 = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr3);
    model.args.push_back(expr3);

    token = GetNextTokenSkipDivider();
    if (!token.IsEolOrEof())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseGotoGosub(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeNumber)
        MODEL_ERROR("Line number expected.");
    model.paramline = atoi(token.text.c_str());

    token = GetNextTokenSkipDivider();
    if (!token.IsEolOrEof())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

//NOTE: For now, in form: IF expr THEN linenum [ELSE linueum]
void Parser::ParseIf(SourceLineModel& model)
{
    Token token = PeekNextToken();
    ExpressionModel expr = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr);
    model.args.push_back(expr);

    token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeKeyword || (token.keyword != KeywordTHEN && token.keyword != KeywordGOTO))
        MODEL_ERROR("Keyword THEN or GOTO expected.");

    token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeNumber || !token.IsDValueInteger())
        MODEL_ERROR("Line number expected.");
    model.params.push_back(token);

    token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;
    if (token.type != TokenTypeKeyword || token.keyword != KeywordELSE)
        MODEL_ERROR(MSG_UNEXPECTED);

    token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeNumber || !token.IsDValueInteger())
        MODEL_ERROR("Line number expected.");
    model.params.push_back(token);

    token = GetNextTokenSkipDivider();
    if (!token.IsEolOrEof())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseInput(SourceLineModel& model)
{
    Token token = PeekNextTokenSkipDivider();
    if (token.type == TokenTypeSymbol && token.symbol == '#')  // File operation
    {
        GetNextToken();
        model.fileoper = true;
    }
    else if (token.type == TokenTypeString)  // prompt string
    {
        GetNextToken();
        model.params.push_back(token);
        token = GetNextTokenSkipDivider();
        if (token.type != TokenTypeSymbol || token.symbol != ';')
            MODEL_ERROR("Semicolon expected.");
    }

    while (true)
    {
        VariableModel var = ParseVariable(model);
        CHECK_MODEL_ERROR;
        model.variables.push_back(var);

        token = GetNextTokenSkipDivider();
        if (!token.IsComma())
            break;
    }

    if (!token.IsEolOrEof())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseOpen(SourceLineModel& model)
{
    Token token = PeekNextTokenSkipDivider();
    ExpressionModel expr1 = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr1);
    model.args.push_back(expr1);

    token = GetNextTokenSkipDivider();
    if (token.type == TokenTypeKeyword && token.keyword == KeywordFOR)
    {
        token = GetNextTokenSkipDivider();
        if (token.type == TokenTypeKeyword && token.keyword == KeywordINPUT)
            model.filemode = FileModeInput;
        else if (token.type == TokenTypeKeyword && token.keyword == KeywordOUTPUT)
            model.filemode = FileModeOutput;
        else
            MODEL_ERROR(MSG_UNEXPECTED);

        token = GetNextTokenSkipDivider();
    }

    if (!token.IsEolOrEof())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseKey(SourceLineModel& model)
{
    Token token = PeekNextTokenSkipDivider();
    ExpressionModel expr1 = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr1);
    model.args.push_back(expr1);

    SKIP_COMMA;

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr2 = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr2);
    model.args.push_back(expr2);

    token = GetNextTokenSkipDivider();
    if (!token.IsEolOrEof())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseLet(SourceLineModel& model)
{
    Token token = PeekNextTokenSkipDivider();
    if (token.type != TokenTypeIdentifier && (token.type != TokenTypeKeyword || token.keyword != KeywordMID))
        MODEL_ERROR("Identifier or MID$ expected.");

    ParseLetShort(token, model);
}

//NOTE: We did only peek on tokenIdentOrMid, NOT get
void Parser::ParseLetShort(Token& tokenIdentOrMid, SourceLineModel& model)
{
    model.ident = tokenIdentOrMid;

    Token token;
    if (tokenIdentOrMid.type == TokenTypeIdentifier)
    {
        VariableModel var = ParseVariable(model);
        CHECK_MODEL_ERROR;
        model.variables.push_back(var);
    }
    else if (tokenIdentOrMid.type == TokenTypeKeyword && tokenIdentOrMid.keyword == KeywordMID)
    {
        GetNextToken();  // MID$

        token = GetNextTokenSkipDivider();
        if (!token.IsOpenBracket())
            MODEL_ERROR(MSG_OPEN_BRACKET_EXPECTED);

        VariableModel var = ParseVariable(model);
        CHECK_MODEL_ERROR;
        model.variables.push_back(var);

        SKIP_COMMA;

        token = PeekNextTokenSkipDivider();
        ExpressionModel expr1 = ParseExpression(model);
        //TODO: Save to model

        SKIP_COMMA;

        token = GetNextTokenSkipDivider();
        if (token.type != TokenTypeNumber || !token.IsDValueInteger())
            MODEL_ERROR("Integer argument expected.");
        model.params.push_back(token);

        token = GetNextTokenSkipDivider();
        if (!token.IsCloseBracket())
            MODEL_ERROR(MSG_CLOSE_BRACKET_EXPECTED);
    }

    token = GetNextTokenSkipDivider();
    if (!token.IsEqualSign())
        MODEL_ERROR("Equal sign (\'=\') expected.");

    ExpressionModel expr = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr);
    model.args.push_back(expr);

    token = GetNextTokenSkipDivider();
    if (!token.IsEolOrEof())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseLocate(SourceLineModel& model)
{
    Token token = PeekNextTokenSkipDivider();
    if (token.IsEolOrEof())
        MODEL_ERROR(MSG_ARGUMENTS_EXPECTED);

    ExpressionModel expr1 = ParseExpression(model);
    CHECK_MODEL_ERROR;
    model.args.push_back(expr1);

    token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;
    if (!token.IsComma())
        MODEL_ERROR(MSG_UNEXPECTED);

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr2 = ParseExpression(model);
    CHECK_MODEL_ERROR;
    model.args.push_back(expr2);

    token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;
    if (!token.IsComma())
        MODEL_ERROR(MSG_UNEXPECTED);

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr3 = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr3);
    model.args.push_back(expr3);

    token = GetNextTokenSkipDivider();
    if (!token.IsEolOrEof())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseNext(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    while (true)
    {
        if (token.type != TokenTypeIdentifier)
            MODEL_ERROR("Identifier expected.");
        //TODO: Check for numeric variable

        model.params.push_back(token);

        token = GetNextTokenSkipDivider();
        if (token.IsEolOrEof())
            break;

        if (token.type != TokenTypeSymbol || token.symbol != ',')
            MODEL_ERROR("Comma or end of line expected.");

        token = GetNextTokenSkipDivider();
    }
}

void Parser::ParseOn(SourceLineModel& model)
{
    Token token = PeekNextTokenSkipDivider();
    if (token.IsEolOrEof())
        MODEL_ERROR("Expreession expected.");

    ExpressionModel expr = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr);
    model.args.push_back(expr);

    token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeKeyword || (token.keyword != KeywordGOTO && token.keyword != KeywordGOSUB))
        MODEL_ERROR("GOTO or GOSUB expected.");
    model.gotogosub = (token.keyword == KeywordGOTO);

    // Loop for line numbers, comma separated
    while (true)
    {
        token = PeekNextTokenSkipDivider();
        if (token.type != TokenTypeNumber)
            MODEL_ERROR("Line number expected.");
        token = GetNextToken();
        model.params.push_back(token);

        token = GetNextTokenSkipDivider();
        if (token.IsEolOrEof())
            break;
        if (!token.IsComma())
            MODEL_ERROR(MSG_UNEXPECTED);
    }
}

void Parser::ParseOut(SourceLineModel& model)
{
    Token token = PeekNextTokenSkipDivider();
    if (token.IsEolOrEof())
        MODEL_ERROR(MSG_ARGUMENTS_EXPECTED);

    ExpressionModel expr1 = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr1);
    model.args.push_back(expr1);

    SKIP_COMMA;

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr2 = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr2);
    model.args.push_back(expr2);

    SKIP_COMMA;

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr3 = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr3);
    model.args.push_back(expr3);

    token = GetNextTokenSkipDivider();
    if (!token.IsEolOrEof())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

//TODO: LPRINT
void Parser::ParsePrint(SourceLineModel& model)
{
    m_tokenizer->SetMode(TokenizerModePrint);

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
        if (token.IsEolOrEof())
        {
            GetNextToken();
            return;
        }

        token = PeekNextTokenSkipDivider();
        if (token.type == TokenTypeKeyword && token.keyword == KeywordAT)
        {
            ExpressionNode node0;
            node0.node = GetNextToken();  // AT keyword

            token = GetNextTokenSkipDivider();
            if (!token.IsOpenBracket())
                MODEL_ERROR(MSG_OPEN_BRACKET_EXPECTED);

            ExpressionModel expr1 = ParseExpression(model);
            CHECK_MODEL_ERROR;
            CHECK_EXPRESSION_NOT_EMPTY(expr1);

            token = GetNextTokenSkipDivider();
            if (!token.IsComma())
                MODEL_ERROR(MSG_COMMA_EXPECTED);

            ExpressionModel expr2 = ParseExpression(model);
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

            model.args.push_back(expr0);

            continue;
        }
        if (token.type == TokenTypeKeyword && token.keyword == KeywordTAB)
        {
            ExpressionNode node0;
            node0.node = GetNextToken();  // TAB keyword

            token = GetNextTokenSkipDivider();
            if (!token.IsOpenBracket())
                MODEL_ERROR(MSG_OPEN_BRACKET_EXPECTED);

            ExpressionModel expr1 = ParseExpression(model);
            CHECK_MODEL_ERROR;
            CHECK_EXPRESSION_NOT_EMPTY(expr1);

            token = GetNextTokenSkipDivider();
            if (!token.IsCloseBracket())
                MODEL_ERROR(MSG_CLOSE_BRACKET_EXPECTED);

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
                MODEL_ERROR(MSG_OPEN_BRACKET_EXPECTED);

            ExpressionModel expr1 = ParseExpression(model);
            CHECK_MODEL_ERROR;
            CHECK_EXPRESSION_NOT_EMPTY(expr1);

            token = GetNextTokenSkipDivider();
            if (!token.IsCloseBracket())
                MODEL_ERROR(MSG_CLOSE_BRACKET_EXPECTED);

            node0.args.push_back(expr1);

            ExpressionModel expr0;
            expr0.nodes.push_back(node0);
            expr0.root = 0;

            model.args.push_back(expr0);

            continue;
        }

        ExpressionModel expr = ParseExpression(model);
        CHECK_MODEL_ERROR;
        model.args.push_back(expr);

        token = GetNextTokenSkipDivider();
        if (token.IsEolOrEof())
            return;
        if (!token.IsComma() && !token.IsSemicolon())
            MODEL_ERROR("Comma or semicolon expected.");
    }
}

void Parser::ParsePoke(SourceLineModel& model)
{
    Token token = PeekNextTokenSkipDivider();
    ExpressionModel expr1 = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr1);
    model.args.push_back(expr1);

    SKIP_COMMA;

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr2 = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr2);
    model.args.push_back(expr2);

    token = GetNextTokenSkipDivider();
    if (!token.IsEolOrEof())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParsePsetPreset(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if ((token.type == TokenTypeSymbol && token.symbol == '@') ||
        (token.type == TokenTypeKeyword && token.keyword == KeywordSTEP))
    {
        model.relative = true;
        token = GetNextTokenSkipDivider();
    }

    if (!token.IsOpenBracket())
        MODEL_ERROR(MSG_OPEN_BRACKET_EXPECTED);

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr1 = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr1);
    model.args.push_back(expr1);

    SKIP_COMMA;

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr2 = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr2);
    model.args.push_back(expr2);

    token = GetNextTokenSkipDivider();
    if (!token.IsCloseBracket())
        MODEL_ERROR(MSG_CLOSE_BRACKET_EXPECTED);

    token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;
    if (!token.IsComma())
        MODEL_ERROR("Unexpected text after arguments.");

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr3 = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr3);
    model.args.push_back(expr3);

    token = GetNextTokenSkipDivider();
    if (!token.IsEolOrEof())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseLine(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if ((token.type == TokenTypeSymbol && token.symbol == '@') ||
        (token.type == TokenTypeKeyword && token.keyword == KeywordSTEP))
    {
        model.relative = true;
        token = GetNextTokenSkipDivider();
    }

    if (token.IsOpenBracket())  // we ahve ARG1, ARG2
    {
        token = PeekNextTokenSkipDivider();
        ExpressionModel expr1 = ParseExpression(model);
        CHECK_MODEL_ERROR;
        CHECK_EXPRESSION_NOT_EMPTY(expr1);
        model.args.push_back(expr1);

        SKIP_COMMA;

        token = PeekNextTokenSkipDivider();
        ExpressionModel expr2 = ParseExpression(model);
        CHECK_MODEL_ERROR;
        CHECK_EXPRESSION_NOT_EMPTY(expr2);
        model.args.push_back(expr2);

        token = GetNextTokenSkipDivider();
        if (!token.IsCloseBracket())
            MODEL_ERROR(MSG_CLOSE_BRACKET_EXPECTED);

        token = GetNextTokenSkipDivider();
    }
    else
    {
        //TODO: add two empty expressions
    }

    if (token.type != TokenTypeOperation || token.text != "-")
        MODEL_ERROR("Minus \'-\' sign expected.");

    token = GetNextTokenSkipDivider();
    if ((token.type == TokenTypeSymbol && token.symbol == '@') ||
        (token.type == TokenTypeKeyword && token.keyword == KeywordSTEP))
    {
        //model.relative = true;//TODO
        token = GetNextTokenSkipDivider();
    }

    if (token.IsOpenBracket())  // we have ARG3, ARG4
    {
        token = PeekNextTokenSkipDivider();
        ExpressionModel expr3 = ParseExpression(model);
        CHECK_MODEL_ERROR;
        CHECK_EXPRESSION_NOT_EMPTY(expr3);
        model.args.push_back(expr3);

        SKIP_COMMA;

        token = PeekNextTokenSkipDivider();
        ExpressionModel expr4 = ParseExpression(model);
        CHECK_MODEL_ERROR;
        CHECK_EXPRESSION_NOT_EMPTY(expr4);
        model.args.push_back(expr4);

        token = GetNextTokenSkipDivider();
        if (!token.IsCloseBracket())
            MODEL_ERROR(MSG_CLOSE_BRACKET_EXPECTED);

        token = GetNextTokenSkipDivider();
    }
    else
    {
        //TODO: add two empty expressions
    }

    if (token.IsEolOrEof())
        return;

    if (token.IsComma())  // we have ARG5
    {
        token = PeekNextTokenSkipDivider();
        ExpressionModel expr5 = ParseExpression(model);
        CHECK_MODEL_ERROR;
        CHECK_EXPRESSION_NOT_EMPTY(expr5);
        model.args.push_back(expr5);

        token = GetNextTokenSkipDivider();

        if (token.IsComma())  // we have "B" or "BF" here
        {
            token = GetNextTokenSkipDivider();
            if (token.type != TokenTypeIdentifier || (token.text != "B" && token.text != "BF"))
                MODEL_ERROR("\'B\' or \'BR\' expected.");

            //TODO: save to model

            token = GetNextTokenSkipDivider();
        }
    }

    if (!token.IsEolOrEof())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseCircle(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if ((token.type == TokenTypeSymbol && token.symbol == '@') ||
        (token.type == TokenTypeKeyword && token.keyword == KeywordSTEP))
    {
        model.relative = true;
        token = GetNextTokenSkipDivider();
    }

    if (!token.IsOpenBracket())
        MODEL_ERROR(MSG_OPEN_BRACKET_EXPECTED);

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr1 = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr1);
    model.args.push_back(expr1);

    SKIP_COMMA;

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr2 = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr2);
    model.args.push_back(expr2);

    token = GetNextTokenSkipDivider();
    if (!token.IsCloseBracket())
        MODEL_ERROR(MSG_CLOSE_BRACKET_EXPECTED);

    SKIP_COMMA;

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr3 = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr3);
    model.args.push_back(expr3);

    token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;

    if (token.IsComma())
    {
        token = PeekNextTokenSkipDivider();
        ExpressionModel expr4 = ParseExpression(model);
        CHECK_MODEL_ERROR;
        model.args.push_back(expr4);

        token = GetNextTokenSkipDivider();
        if (token.IsComma())
        {
            token = PeekNextTokenSkipDivider();
            ExpressionModel expr5 = ParseExpression(model);
            CHECK_MODEL_ERROR;
            model.args.push_back(expr5);

            token = GetNextTokenSkipDivider();
            if (token.IsComma())
            {
                token = PeekNextTokenSkipDivider();
                ExpressionModel expr6 = ParseExpression(model);
                CHECK_MODEL_ERROR;
                model.args.push_back(expr6);

                token = GetNextTokenSkipDivider();
                if (token.IsComma())
                {
                    token = PeekNextTokenSkipDivider();
                    ExpressionModel expr7 = ParseExpression(model);
                    CHECK_MODEL_ERROR;
                    model.args.push_back(expr7);

                    token = GetNextTokenSkipDivider();
                }
            }
        }
    }
    
    if (!token.IsEolOrEof())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParsePaint(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if ((token.type == TokenTypeSymbol && token.symbol == '@') ||
        (token.type == TokenTypeKeyword && token.keyword == KeywordSTEP))
    {
        model.relative = true;
        token = GetNextTokenSkipDivider();
    }

    if (!token.IsOpenBracket())
        MODEL_ERROR(MSG_OPEN_BRACKET_EXPECTED);

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr1 = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr1);
    model.args.push_back(expr1);

    SKIP_COMMA;

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr2 = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr2);
    model.args.push_back(expr2);

    token = GetNextTokenSkipDivider();
    if (!token.IsCloseBracket())
        MODEL_ERROR(MSG_CLOSE_BRACKET_EXPECTED);

    token = GetNextTokenSkipDivider();
    if (token.IsComma())
    {
        token = PeekNextTokenSkipDivider();
        ExpressionModel expr3 = ParseExpression(model);
        CHECK_MODEL_ERROR;
        model.args.push_back(expr3);

        token = GetNextTokenSkipDivider();
        if (token.IsComma())
        {
            token = PeekNextTokenSkipDivider();
            ExpressionModel expr4 = ParseExpression(model);
            CHECK_MODEL_ERROR;
            model.args.push_back(expr4);

            token = GetNextTokenSkipDivider();
        }
    }

    if (!token.IsEolOrEof())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseRead(SourceLineModel& model)
{
    Token token;
    while (true)
    {
        VariableModel var = ParseVariable(model);
        CHECK_MODEL_ERROR;
        model.variables.push_back(var);

        token = GetNextTokenSkipDivider();
        if (token.IsEolOrEof())
            return;  // End of the list

        if (!token.IsComma())
            MODEL_ERROR(MSG_COMMA_EXPECTED);
    }
}

void Parser::ParseRem(SourceLineModel& model)
{
    SkipTilEnd();
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
        MODEL_ERROR("Numeric argument expected.");
    //TODO: Check for valid range
    model.paramline = (int)token.dvalue;

    token = GetNextTokenSkipDivider();
    if (!token.IsEolOrEof())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseDef(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if (token.type == TokenTypeKeyword && token.keyword == KeywordFN)  // DEF FN
        ParseDefFn(model);
    else if (token.type == TokenTypeKeyword && token.keyword == KeywordUSR)  // DEF USR
        ParseDefUsr(model);
    else
        MODEL_ERROR("\'FN\' or \'USR\' expected.");
}

void Parser::ParseDefFn(SourceLineModel& model)
{
    model.deffnorusr = true;

    Token token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeIdentifier)
        MODEL_ERROR("Identifier expected.");
    model.ident = token;

    token = GetNextTokenSkipDivider();
    if (token.IsOpenBracket())  // Parse optional parameters
    {
        while (true)
        {
            token = GetNextTokenSkipDivider();
            if (token.type != TokenTypeIdentifier)
                MODEL_ERROR("Identifier expected.");
            model.params.push_back(token);

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
    ExpressionModel expr = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr);
    model.args.push_back(expr);

    token = GetNextTokenSkipDivider();
    if (!token.IsEolOrEof())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseDefUsr(SourceLineModel& model)
{
    model.deffnorusr = false;

    int usrnumber = 0;
    Token token = GetNextToken();
    if (token.type == TokenTypeNumber)
    {
        usrnumber = atoi(token.text.c_str());
    }
    else if (token.type != TokenTypeDivider)
        MODEL_ERROR(MSG_UNEXPECTED);
    model.paramline = usrnumber;

    token = GetNextTokenSkipDivider();
    if (!token.IsEqualSign())
        MODEL_ERROR("Equal sign expected.");

    token = PeekNextTokenSkipDivider();
    ExpressionModel expr = ParseExpression(model);
    CHECK_MODEL_ERROR;
    CHECK_EXPRESSION_NOT_EMPTY(expr);
    model.args.push_back(expr);

    token = GetNextTokenSkipDivider();
    if (!token.IsEolOrEof())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

void Parser::ParseScreen(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeNumber)
        MODEL_ERROR("Numeric argument expected.");

    model.params.push_back(token);

    token = GetNextTokenSkipDivider();
    if (!token.IsEolOrEof())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}

// Undocumented instruction
// WIDTH <Integer>, [<Integer>]
void Parser::ParseWidth(SourceLineModel& model)
{
    Token token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeNumber)
        MODEL_ERROR("Numeric argument expected.");
    model.params.push_back(token);

    token = GetNextTokenSkipDivider();
    if (token.IsEolOrEof())
        return;
    if (!token.IsComma())
        MODEL_ERROR(MSG_UNEXPECTED);

    token = GetNextTokenSkipDivider();
    if (token.type != TokenTypeNumber)
        MODEL_ERROR("Numeric argument expected.");
    model.params.push_back(token);

    token = GetNextTokenSkipDivider();
    if (!token.IsEolOrEof())
        MODEL_ERROR(MSG_UNEXPECTED_AT_END_OF_STATEMENT);
}


//////////////////////////////////////////////////////////////////////
