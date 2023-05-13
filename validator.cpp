
#include <cassert>
#include <iomanip>
#include <string>
#include <sstream>
#include <bitset>

#include "main.h"


//////////////////////////////////////////////////////////////////////


const ValidatorKeywordSpec Validator::m_keywordspecs[] =
{
    { KeywordBEEP,      &Validator::ValidateNothing },
    { KeywordBLOAD,     &Validator::ValidateNothing },
    { KeywordBSAVE,     &Validator::ValidateNothing },
    { KeywordCIRCLE,    &Validator::ValidateCircle },
    { KeywordCLEAR,     &Validator::ValidateClear },
    { KeywordCLOAD,     &Validator::ValidateNothing },
    { KeywordCLOSE,     &Validator::ValidateNothing },
    { KeywordCLS,       &Validator::ValidateNothing },
    { KeywordCOLOR,     &Validator::ValidateColor },
    { KeywordCSAVE,     &Validator::ValidateNothing },
    { KeywordDATA,      &Validator::ValidateData },
    { KeywordDIM,       &Validator::ValidateDim },
    { KeywordKEY,       &Validator::ValidateKey },
    { KeywordDRAW,      &Validator::ValidateDraw },
    { KeywordEND,       &Validator::ValidateNothing },
    { KeywordFOR,       &Validator::ValidateFor },
    { KeywordREAD,      &Validator::ValidateRead },
    { KeywordREM,       &Validator::ValidateNothing },
    { KeywordGOSUB,     &Validator::ValidateGotoGosub },
    { KeywordGOTO,      &Validator::ValidateGotoGosub },
    { KeywordIF,        &Validator::ValidateIf },
    { KeywordINPUT,     &Validator::ValidateInput },
    { KeywordLET,       &Validator::ValidateLet },
    { KeywordLINE,      &Validator::ValidateLine },
    { KeywordLOAD,      &Validator::ValidateNothing },
    { KeywordLOCATE,    &Validator::ValidateLocate },
    { KeywordNEXT,      &Validator::ValidateNext },
    { KeywordON,        &Validator::ValidateOn },
    { KeywordOPEN,      &Validator::ValidateOpen },
    { KeywordOUT,       &Validator::ValidateOut },
    { KeywordPAINT,     &Validator::ValidatePaint },
    { KeywordPOKE,      &Validator::ValidatePoke },
    { KeywordPRINT,     &Validator::ValidatePrint },
    { KeywordPSET,      &Validator::ValidatePset },
    { KeywordPRESET,    &Validator::ValidatePreset },
    { KeywordRESTORE,   &Validator::ValidateRestore },
    { KeywordRETURN,    &Validator::ValidateNothing },
    { KeywordSAVE,      &Validator::ValidateNothing },
    { KeywordSCREEN,    &Validator::ValidateScreen },
    { KeywordSTOP,      &Validator::ValidateNothing },
    { KeywordTROFF,     &Validator::ValidateNothing },
    { KeywordTRON,      &Validator::ValidateNothing },
    { KeywordDEF,       &Validator::ValidateDef },
    { KeywordWIDTH,     &Validator::ValidateWidth },
};

const ValidatorOperSpec Validator::m_operspecs[] =
{
    { "+",              &Validator::ValidateOperPlus },
    { "-",              &Validator::ValidateOperMinus },
    { "*",              &Validator::ValidateOperMul },
    { "/",              &Validator::ValidateOperDiv },
    { "\\",             &Validator::ValidateOperDivInt },
    { "MOD",            &Validator::ValidateOperMod },
    { "^",              &Validator::ValidateOperPower },
    { "=",              &Validator::ValidateOperEqual },
    { "<>",             &Validator::ValidateOperNotEqual },
    { "<",              &Validator::ValidateOperLess },
    { ">",              &Validator::ValidateOperGreater },
    { "<=",             &Validator::ValidateOperLessOrEqual },
    { ">=",             &Validator::ValidateOperGreaterOrEqual },
    { "=<",             &Validator::ValidateOperLessOrEqual },
    { "=>",             &Validator::ValidateOperGreaterOrEqual },
    { "AND",            &Validator::ValidateOperAnd },
    { "OR",             &Validator::ValidateOperOr },
    //TODO: XOR
    //TODO: EQV
    //TODO: IMP
};

const ValidatorFuncSpec Validator::m_funcspecs[] =
{
    { KeywordSIN,       &Validator::ValidateFuncSin },
    { KeywordCOS,       &Validator::ValidateFuncCos },
    { KeywordTAN,       &Validator::ValidateFuncTan },
    { KeywordATN,       &Validator::ValidateFuncAtn },
    { KeywordPI,        &Validator::ValidateFuncPi },
    { KeywordEXP,       &Validator::ValidateFuncExp },
    { KeywordLOG,       &Validator::ValidateFuncLog },
    { KeywordABS,       &Validator::ValidateFuncAbs },
    { KeywordFIX,       &Validator::ValidateFuncFix },
    { KeywordINT,       &Validator::ValidateFuncInt },
    { KeywordSGN,       &Validator::ValidateFuncSgn },
    { KeywordRND,       &Validator::ValidateFuncRnd },
    { KeywordFRE,       &Validator::ValidateFuncFre },
    { KeywordCINT,      &Validator::ValidateFuncCint },
    { KeywordCSNG,      &Validator::ValidateFuncCsng },
    { KeywordPEEK,      &Validator::ValidateFuncPeek },
    { KeywordINP,       &Validator::ValidateFuncInp },
    { KeywordASC,       &Validator::ValidateFuncAsc },
    { KeywordCHR,       &Validator::ValidateFuncChr },
    { KeywordLEN,       &Validator::ValidateFuncLen },
    { KeywordMID,       &Validator::ValidateFuncMid },
    { KeywordSTRING,    &Validator::ValidateFuncString },
    { KeywordVAL,       &Validator::ValidateFuncVal },
    { KeywordINKEY,     &Validator::ValidateFuncInkey },
    { KeywordSTR,       &Validator::ValidateFuncStr },
    { KeywordBIN,       &Validator::ValidateFuncBin },
    { KeywordOCT,       &Validator::ValidateFuncOct },
    { KeywordHEX,       &Validator::ValidateFuncHex },
    { KeywordCSRLIN,    &Validator::ValidateFuncCsrlinPosLpos },
    { KeywordPOS,       &Validator::ValidateFuncCsrlinPosLpos },
    { KeywordLPOS,      &Validator::ValidateFuncCsrlinPosLpos },
    { KeywordEOF,       &Validator::ValidateFuncEof },
    { KeywordPOINT,     &Validator::ValidateFuncPoint },
};

Validator::Validator(SourceModel* source)
{
    assert(source != nullptr);
    m_source = source;

    m_lineindex = -1;
    m_line = nullptr;
}

bool Validator::ProcessLine()
{
    if (m_lineindex < 0)
    {
        //ProcessBegin();
        m_lineindex = 0;
    }
    else
        m_lineindex++;

    if (m_lineindex >= (int)m_source->lines.size())
    {
        //ProcessEnd();
        m_lineindex = INT_MAX;
        return false;
    }

    m_line = &(m_source->lines[m_lineindex]);

    ValidateStatement(m_line->statement);

    return true;
}

void Validator::ValidateStatement(StatementModel& statement)
{
    // Find validator implementation
    KeywordIndex keyword = statement.token.keyword;
    ValidatorMethodRef methodref = nullptr;
    for (auto it = std::begin(m_keywordspecs); it != std::end(m_keywordspecs); ++it)
    {
        if (keyword == it->keyword)
        {
            methodref = it->methodref;
            break;
        }
    }
    if (methodref == nullptr)
    {
        Error("Validator not found for keyword " + GetKeywordString(keyword) + ".");
        return;
    }

    (this->*methodref)(statement);
}

void Validator::Error(const string& message)
{
    std::cerr << "ERROR in line " << m_line->number << " - " << message << std::endl;
    m_line->error = true;
    RegisterError();
}

void Validator::Error(ExpressionModel& expr, const string& message)
{
    std::cerr << "ERROR in line " << m_line->number << " in expression - " << message << std::endl;
    m_line->error = true;
    RegisterError();
}

void Validator::Error(ExpressionModel& expr, const ExpressionNode& node, const string& message)
{
    std::cerr << "ERROR in line " << m_line->number << " at " << node.node.line << ":" << node.node.pos << " - " << message << std::endl;
    m_line->error = true;
    RegisterError();
}

void Validator::ValidateExpression(ExpressionModel& expr)
{
    if (expr.root < 0)
        return;

    ValidateExpression(expr, expr.root);
}

void Validator::ValidateExpression(ExpressionModel& expr, int index)
{
    if (index < 0)
        return;

    ExpressionNode& node = expr.nodes[index];

    if (node.left >= 0)
        ValidateExpression(expr, node.left);
    if (node.right >= 0)
        ValidateExpression(expr, node.right);

    if (node.node.type == TokenTypeIdentifier)
    {
        VariableModel var;
        var.name = node.node.text;
        m_source->RegisterVariable(var);
    }

    if (node.node.type == TokenTypeOperation && node.left < 0 && node.right >= 0)  // Unary operation, one operand
    {
        const ExpressionNode& noderight = expr.nodes[node.right];

        if (node.node.text == "+")
            ValidateUnaryPlus(expr, node, noderight);
        else if (node.node.text == "-")
            ValidateUnaryMinus(expr, node, noderight);
        //TODO: Unary NOT
        else
        {
            std::cerr << "ERROR in line " << m_line->number << " at " << node.node.line << ":" << node.node.pos << " - TODO validate unary operator " << node.node.text << std::endl;
            m_line->error = true;
            RegisterError();
            return;
        }
    }
    else if (node.node.type == TokenTypeOperation && node.left >= 0 && node.right >= 0)
    {
        const ExpressionNode& nodeleft = expr.nodes[node.left];
        const ExpressionNode& noderight = expr.nodes[node.right];

        if (nodeleft.vtype == ValueTypeNone || noderight.vtype == ValueTypeNone)
        {
            std::cerr << "ERROR in line " << m_line->number << " at " << node.node.line << ":" << node.node.pos << " - Cannot calculate value type for the node." << std::endl;
            m_line->error = true;
            RegisterError();
            return;
        }

        // Find validator implementation
        string text = node.node.text;
        ValidatorOperMethodRef methodref = nullptr;
        for (auto it = std::begin(m_operspecs); it != std::end(m_operspecs); ++it)
        {
            if (text == it->text)
            {
                methodref = it->methodref;
                break;
            }
        }

        if (methodref != nullptr)
            (this->*methodref)(expr, node, nodeleft, noderight);
        else
        {
            std::cerr << "ERROR in line " << m_line->number << " at " << node.node.line << ":" << node.node.pos << " - TODO validate operator \'" + text + "\'." << std::endl;
            m_line->error = true;
            RegisterError();
            return;
        }
    }

    if (node.node.type == TokenTypeKeyword)  // Check is it function, validate function
    {
        // Find validator implementation
        KeywordIndex keyword = node.node.keyword;
        ValidatorFuncMethodRef methodref = nullptr;
        for (auto it = std::begin(m_funcspecs); it != std::end(m_funcspecs); ++it)
        {
            if (keyword == it->keyword)
            {
                methodref = it->methodref;
                break;
            }
        }

        if (methodref == nullptr)
        {
            std::cerr << "ERROR in line " << m_line->number << " at " << node.node.line << ":" << node.node.pos << " - TODO validate function " + GetKeywordString(keyword) << std::endl;
            m_line->error = true;
            RegisterError();
            return;
        }

        (this->*methodref)(expr, node);
    }

    //TODO
}

bool Validator::CheckIntegerOrSingleExpression(ExpressionModel& expr)
{
    if (expr.IsEmpty())
    {
        Error(expr, "Expression should not be empty.");
        return false;
    }

    ValidateExpression(expr, expr.root);

    const ExpressionNode& root = expr.nodes[expr.root];
    if (root.vtype != ValueTypeInteger && root.vtype != ValueTypeSingle)
    {
        Error(expr, root, "Expression should be of type Integer or Single.");
        return false;
    }

    return true;
}

bool Validator::CheckStringExpression(ExpressionModel& expr)
{

    if (expr.IsEmpty())
    {
        Error(expr, "Expression should not be empty.");
        return false;
    }

    ValidateExpression(expr, expr.root);

    const ExpressionNode& root = expr.nodes[expr.root];
    if (root.vtype != ValueTypeString)
    {
        Error(expr, root, "Expression should be of type String.");
        return false;
    }

    return true;
}

// Statement validation //////////////////////////////////////////////

#define MODEL_ERROR(msg) \
    { Error(msg); return; }
#define CHECK_MODEL_ERROR \
    { if (m_line->error) return; }

void Validator::ValidateNothing(StatementModel& statement)
{
    // Nothing to validate
}

void Validator::ValidateClear(StatementModel& statement)
{
    if (statement.args.size() == 0)
        MODEL_ERROR("Expression expected.");

    ExpressionModel& expr1 = statement.args[0];
    if (!CheckIntegerOrSingleExpression(expr1))
        return;
    
    if (statement.args.size() > 1)
    {
        ExpressionModel& expr2 = statement.args[1];
        if (!CheckIntegerOrSingleExpression(expr2))
            return;
    }
    if (statement.args.size() > 2)
        MODEL_ERROR("Too many expressions.");
}

void Validator::ValidateData(StatementModel& statement)
{
    if (statement.params.size() == 0)
        MODEL_ERROR("Parameter(s) expected.");

    for (auto it = std::begin(statement.params); it != std::end(statement.params); ++it)
    {
        Token& token = *it;
        if (token.type != TokenTypeNumber && token.type != TokenTypeString)
            MODEL_ERROR("Parameter should be of type Number or String.");

        //TODO: put the const value into DATA structures
    }
}

void Validator::ValidateRead(StatementModel& statement)
{
    //TODO
}

void Validator::ValidateColor(StatementModel& statement)
{
    if (statement.args.size() == 0)
        MODEL_ERROR("Expression expected.");

    {
        ExpressionModel& expr1 = statement.args[0];
        if (!expr1.IsEmpty() && !CheckIntegerOrSingleExpression(expr1))
            return;
    }
    if (statement.args.size() > 1)
    {
        ExpressionModel& expr2 = statement.args[1];
        if (!expr2.IsEmpty() && !CheckIntegerOrSingleExpression(expr2))
            return;
    }
    if (statement.args.size() > 2)
    {
        ExpressionModel& expr3 = statement.args[2];
        if (!expr3.IsEmpty() && !CheckIntegerOrSingleExpression(expr3))
            return;
    }

    if (statement.args.size() > 3)
        MODEL_ERROR("Too many expressions.");
}

void Validator::ValidateDim(StatementModel& statement)
{
    for (auto it = std::begin(statement.variables); it != std::end(statement.variables); ++it)
    {
        if (!m_source->RegisterVariable(*it))
            MODEL_ERROR("Variable redefinition for " + it->name + ".");
    }
}

void Validator::ValidateKey(StatementModel& statement)
{
    if (statement.args.size() != 2)
        MODEL_ERROR("Two expressions expected.");

    ExpressionModel& expr1 = statement.args[0];
    if (!CheckIntegerOrSingleExpression(expr1))
        return;

    ExpressionModel& expr2 = statement.args[1];
    if (!CheckStringExpression(expr2))
        return;
}

void Validator::ValidateDraw(StatementModel& statement)
{
    if (statement.args.size() != 1)
        MODEL_ERROR("One expression expected.");

    ExpressionModel& expr1 = statement.args[0];
    if (!CheckStringExpression(expr1))
        return;
}

void Validator::ValidateFor(StatementModel& statement)
{
    if (statement.ident.type != TokenTypeIdentifier)
        MODEL_ERROR("Identifier expected.");

    VariableModel var;
    var.name = GetCanonicVariableName(statement.ident.text);
    m_source->RegisterVariable(var);

    // Add FOR variable to FOR/NEXT stack
    ValidatorForSpec forspec;
    forspec.varname = var.name;
    forspec.linenum = m_line->number;
    m_fornextstack.push_back(forspec);

    if (statement.args.size() < 2)
        MODEL_ERROR("Two expressions expected.");

    ExpressionModel& expr1 = statement.args[0];
    if (!CheckIntegerOrSingleExpression(expr1))
        return;

    ExpressionModel& expr2 = statement.args[1];
    if (!CheckIntegerOrSingleExpression(expr2))
        return;

    if (statement.args.size() > 2)  // has STEP expression
    {
        ExpressionModel& expr3 = statement.args[1];
        if (!CheckIntegerOrSingleExpression(expr3))
            return;

        if (statement.args.size() > 3)
            MODEL_ERROR("Too many expressions.");
    }
}

void Validator::ValidateGotoGosub(StatementModel& statement)
{
    if (!m_source->IsLineNumberExists(statement.paramline))
        MODEL_ERROR("Invalid line number " + std::to_string(statement.paramline) + ".");
}

void Validator::ValidateIf(StatementModel& statement)
{
    if (statement.args.size() != 1)
        MODEL_ERROR("One expression expected.");
    ExpressionModel& expr = statement.args[0];
    ValidateExpression(expr);
    if (expr.IsEmpty())
        MODEL_ERROR("Expression should not be empty.");

    if (statement.params.size() < 1 || statement.params.size() > 2)
        MODEL_ERROR("One or two parameters expected.");

    if (statement.stthen == nullptr)
    {
        // Line number for THEN
        Token& param1 = statement.params[0];
        int linenum1 = (int)param1.dvalue;
        if (!m_source->IsLineNumberExists(linenum1))
            MODEL_ERROR("Invalid line number " + std::to_string(linenum1) + ".");
    }
    else
    {
        ValidateStatement(*statement.stthen);
        CHECK_MODEL_ERROR;
    }

    if (statement.stelse == nullptr)
    {
        if (statement.params.size() > 1)
        {
            // Line number for ELSE
            Token& param2 = statement.params[1];
            int linenum2 = (int)param2.dvalue;
            if (!m_source->IsLineNumberExists(linenum2))
                MODEL_ERROR("Invalid line number " + std::to_string(linenum2) + ".");
        }
    }
    else
    {
        ValidateStatement(*statement.stelse);
        CHECK_MODEL_ERROR;
    }
}

void Validator::ValidateInput(StatementModel& statement)
{
    if (statement.params.size() > 1)
        MODEL_ERROR("Too many parameters.");
    if (statement.params.size() > 0)
    {
        Token& param = statement.params[0];
        if (param.type != TokenTypeString)
            MODEL_ERROR("Parameter should be of type String.");
        m_source->RegisterConstString(param.text);
    }

    if (statement.variables.size() == 0)
        MODEL_ERROR("Variable(s) expected.");

    for (auto it = std::begin(statement.variables); it != std::end(statement.variables); ++it)
    {
        m_source->RegisterVariable(*it);
    }
}

void Validator::ValidateOpen(StatementModel& statement)
{
    if (statement.args.size() != 1)
        MODEL_ERROR("One expression expected.");

    ExpressionModel& expr1 = statement.args[0];
    if (!CheckStringExpression(expr1))
        return;
}

void Validator::ValidateLine(StatementModel& statement)
{
    //TODO
}

void Validator::ValidateCircle(StatementModel& statement)
{
    //TODO
}

void Validator::ValidatePaint(StatementModel& statement)
{
    //TODO
}

void Validator::ValidateLet(StatementModel& statement)
{
    if (statement.varexprs.size() != 1)
        MODEL_ERROR("One variable expected.");

    VariableExpressionModel& var = statement.varexprs[0];
    for (auto it = std::begin(var.args); it != std::end(var.args); it++)
        ValidateExpression(*it);

    m_source->RegisterVariable(var);

    if (statement.args.size() != 1)
        MODEL_ERROR("One expression expected.");

    ExpressionModel& expr = statement.args[0];
    ValidateExpression(expr);

    //TODO: Check types compatibility between variable and expression
}

void Validator::ValidateLocate(StatementModel& statement)
{
    if (statement.args.size() == 0)
        MODEL_ERROR("Expression expected.");

    ExpressionModel& expr1 = statement.args[0];
    if (!expr1.IsEmpty() && !CheckIntegerOrSingleExpression(expr1))
        return;

    if (statement.args.size() > 1)
    {
        ExpressionModel& expr2 = statement.args[1];
        if (!expr2.IsEmpty() && !CheckIntegerOrSingleExpression(expr2))
            return;
    }
    if (statement.args.size() > 2)
    {
        ExpressionModel& expr3 = statement.args[2];
        if (!expr3.IsEmpty() && !CheckIntegerOrSingleExpression(expr3))
            return;
    }

    if (statement.args.size() > 3)
        MODEL_ERROR("Too many expressions.");
}

void Validator::ValidatePset(StatementModel& statement)
{
    //TODO
}

void Validator::ValidatePreset(StatementModel& statement)
{
    //TODO
}

void Validator::ValidateNext(StatementModel& statement)
{
    if (statement.params.empty())  // NEXT without parameters
    {
        if (m_fornextstack.empty())
            MODEL_ERROR("NEXT without FOR.");

        ValidatorForSpec forspec = m_fornextstack.back();
        m_fornextstack.pop_back();

        Token tokenvar;
        tokenvar.type = TokenTypeIdentifier;
        tokenvar.text = forspec.varname;
        statement.params.push_back(tokenvar);

        // link NEXT to the corresponding FOR
        statement.paramline = forspec.linenum;

        // link FOR to the NEXT line number
        SourceLineModel& linefor = m_source->GetSourceLine(forspec.linenum);
        linefor.statement.paramline = m_line->number;

        return;
    }

    //TODO: need to change the model for case of several NEXT variables
    for (auto it = std::begin(statement.params); it != std::end(statement.params); ++it)
    {
        string varname = GetCanonicVariableName(it->text);
        if (!m_source->IsVariableRegistered(varname))
            MODEL_ERROR("Variable not found:" + varname + ".");
        
        //TODO: Check for numeric variable type?

        ValidatorForSpec forspec = m_fornextstack.back();
        m_fornextstack.pop_back();

        if (forspec.varname != varname)
            MODEL_ERROR("NEXT variable expected: " + forspec.varname + ", found:" + varname + ".");

        // link NEXT to the corresponding FOR
        statement.paramline = forspec.linenum;

        // link FOR to the NEXT line number
        SourceLineModel& linefor = m_source->GetSourceLine(forspec.linenum);
        linefor.statement.paramline = m_line->number;
    }
}

void Validator::ValidateOn(StatementModel& statement)
{
    if (statement.args.size() != 1)
        MODEL_ERROR("One Expression expected.");

    ExpressionModel& expr = statement.args[0];
    if (!CheckIntegerOrSingleExpression(expr))
        return;

    if (statement.params.size() == 0)
        MODEL_ERROR("Parameters expected.");

    for (auto it = std::begin(statement.params); it != std::end(statement.params); ++it)
    {
        Token& param = *it;
        if (param.type != TokenTypeNumber || !param.IsDValueInteger())
            MODEL_ERROR("Integer parameter expected.");

        int linenum = (int)param.dvalue;
        if (!m_source->IsLineNumberExists(linenum))
            MODEL_ERROR("Invalid line number " + std::to_string(linenum));
    }
}

void Validator::ValidateOut(StatementModel& statement)
{
    if (statement.args.size() != 3)
        MODEL_ERROR("Three expressions expected.");

    ExpressionModel& expr1 = statement.args[0];
    if (!CheckIntegerOrSingleExpression(expr1))
        return;

    ExpressionModel& expr2 = statement.args[1];
    if (!CheckIntegerOrSingleExpression(expr2))
        return;

    ExpressionModel& expr3 = statement.args[2];
    if (!CheckIntegerOrSingleExpression(expr3))
        return;
}

void Validator::ValidatePoke(StatementModel& statement)
{
    if (statement.args.size() != 2)
        MODEL_ERROR("Two expressions expected.");

    ExpressionModel& expr1 = statement.args[0];
    if (!CheckIntegerOrSingleExpression(expr1))
        return;

    ExpressionModel& expr2 = statement.args[1];
    if (!CheckIntegerOrSingleExpression(expr2))
        return;
}

void Validator::ValidatePrint(StatementModel& statement)
{
    for (auto it = std::begin(statement.args); it != std::end(statement.args); ++it)
    {
        ExpressionModel& expr = *it;
        if (expr.IsEmpty())
            MODEL_ERROR("Expressions should not be empty.");
        ExpressionNode& root = expr.nodes[expr.root];
        if (root.node.IsComma())
        {
            // Don't validate Comma
        }
        else if (root.node.IsKeyword(KeywordAT))
        {
            if (root.args.size() != 2)
                MODEL_ERROR("Two expressions expected for AT function.");
            ExpressionModel& expr1 = root.args[0];
            if (!CheckIntegerOrSingleExpression(expr1))
                return;
            ExpressionModel& expr2 = root.args[1];
            if (!CheckIntegerOrSingleExpression(expr2))
                return;
        }
        else if (root.node.IsKeyword(KeywordTAB))
        {
            if (root.args.size() != 1)
                MODEL_ERROR("Ome expressions expected for TAB function.");
            ExpressionModel& expr1 = root.args[0];
            if (!CheckIntegerOrSingleExpression(expr1))
                return;
        }
        else if (root.node.IsKeyword(KeywordSPC))
        {
            if (root.args.size() != 1)
                MODEL_ERROR("Ome expressions expected for SPC function.");
            ExpressionModel& expr1 = root.args[0];
            if (!CheckIntegerOrSingleExpression(expr1))
                return;
        }
        else
        {
            ValidateExpression(expr);
        }
    }
}

void Validator::ValidateRestore(StatementModel& statement)
{
    if (statement.paramline != 0)  // optional param
    {
        if (!m_source->IsLineNumberExists(statement.paramline))
            MODEL_ERROR("Invalid line number " + std::to_string(statement.paramline));
    }
}

void Validator::ValidateDef(StatementModel& statement)
{
    if (statement.deffnorusr)  // DEF FN
    {
        //TODO

        if (statement.args.size() != 1)
            MODEL_ERROR("One expression expected.");

        ExpressionModel& expr1 = statement.args[0];
        ValidateExpression(expr1);
        //TODO
    }
    else  // DEF USR
    {
        if (statement.paramline < 0 || statement.paramline > 9)
            MODEL_ERROR("DEF USR number is out of range 0..9.");

        if (statement.args.size() != 1)
            MODEL_ERROR("One expression expected.");

        ExpressionModel& expr1 = statement.args[0];
        if (!CheckIntegerOrSingleExpression(expr1))
            return;
    }
}

void Validator::ValidateScreen(StatementModel& statement)
{
    if (statement.params.size() < 1)
        MODEL_ERROR("Parameter expected.");

    Token& token = statement.params[0];
    if (token.type != TokenTypeNumber)
        MODEL_ERROR("Numeric parameter expected.");
}

// Undocumented instruction
// WIDTH <Integer>, [<Integer>]
void Validator::ValidateWidth(StatementModel& statement)
{
    if (statement.params.size() < 1 || statement.params.size() > 2)
        MODEL_ERROR("One or two parameters expected.");

    //NOTE: Ignored for now
}


// Operation validation //////////////////////////////////////////////
// Every operation validator function should:
// 1. Validate operands
// 2. Calculate constval flag
// 3. Calculate result vtype
// 4. Calculate dvalue for const sub-expression

#define EXPR_ERROR(msg) \
    { Error(expr, node, msg); return; }
#define EXPR_CHECK_OPERAND_VTYPE_NONE \
    { if (noderight.vtype == ValueTypeNone) { Error(expr, noderight, "Operand vtype not defined."); return; } }
#define EXPR_CHECK_OPERANDS_VTYPE_NONE \
    { if (nodeleft.vtype == ValueTypeNone) { Error(expr, nodeleft, "Operand vtype not defined."); return; } \
      if (noderight.vtype == ValueTypeNone) { Error(expr, noderight, "Operand vtype not defined."); return; } }

void Validator::ValidateUnaryPlus(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& noderight)
{
    EXPR_CHECK_OPERAND_VTYPE_NONE;

    if (noderight.vtype == ValueTypeString)
        EXPR_ERROR("Operation \'-\' not applicable to strings.");

    node.vtype = noderight.vtype;
    node.constval = noderight.constval;
    if (node.constval)
    {
        node.node.dvalue = noderight.node.dvalue;
    }
}

void Validator::ValidateUnaryMinus(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& noderight)
{
    EXPR_CHECK_OPERAND_VTYPE_NONE;

    if (noderight.vtype == ValueTypeString)
        EXPR_ERROR("Operation \'-\' not applicable to strings.");

    node.vtype = noderight.vtype;
    node.constval = noderight.constval;
    if (node.constval)
    {
        node.node.dvalue = -noderight.node.dvalue;  // invert sign
    }
}

void Validator::ValidateOperPlus(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    EXPR_CHECK_OPERANDS_VTYPE_NONE;

    if (nodeleft.vtype == noderight.vtype)
        node.vtype = nodeleft.vtype;
    else if ((nodeleft.vtype == ValueTypeSingle && noderight.vtype == ValueTypeInteger) ||
        (nodeleft.vtype == ValueTypeInteger && noderight.vtype == ValueTypeSingle))
        node.vtype = ValueTypeSingle;
    else
        EXPR_ERROR(" - Value types are incompatible.");

    node.constval = (nodeleft.constval && noderight.constval);
    if (node.constval)
    {
        node.node.vtype = node.vtype;
        if (node.vtype == ValueTypeInteger || node.vtype == ValueTypeSingle)
            node.node.dvalue = nodeleft.node.dvalue + noderight.node.dvalue;
        else if (node.vtype == ValueTypeString)
        {
            node.node.svalue = nodeleft.node.svalue + noderight.node.svalue;
            if (node.node.svalue.length() > 255)
                node.node.svalue.resize(255);
        }
    }
}

void Validator::ValidateOperMinus(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    EXPR_CHECK_OPERANDS_VTYPE_NONE;

    if (nodeleft.vtype == ValueTypeString || noderight.vtype == ValueTypeString)
        EXPR_ERROR("Operation \'-\' not applicable to strings.");
    if (nodeleft.vtype == noderight.vtype)
        node.vtype = nodeleft.vtype;
    else if ((nodeleft.vtype == ValueTypeSingle && noderight.vtype == ValueTypeInteger) ||
        (nodeleft.vtype == ValueTypeInteger && noderight.vtype == ValueTypeSingle))
        node.vtype = ValueTypeSingle;
    else
        EXPR_ERROR("Value types are incompatible.");

    node.constval = (nodeleft.constval && noderight.constval);
    if (node.constval)
    {
        node.node.dvalue = nodeleft.node.dvalue - noderight.node.dvalue;
    }
}

void Validator::ValidateOperMul(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    EXPR_CHECK_OPERANDS_VTYPE_NONE;

    if (nodeleft.vtype == ValueTypeString || noderight.vtype == ValueTypeString)
        EXPR_ERROR("Operation \'*\' not applicable to strings.");
    if (nodeleft.vtype == noderight.vtype)
        node.vtype = nodeleft.vtype;
    else if ((nodeleft.vtype == ValueTypeSingle && noderight.vtype == ValueTypeInteger) ||
        (nodeleft.vtype == ValueTypeInteger && noderight.vtype == ValueTypeSingle))
        node.vtype = ValueTypeSingle;
    else
        EXPR_ERROR("Value types are incompatible.");

    node.constval = (nodeleft.constval && noderight.constval);
    if (node.constval)
    {
        node.node.dvalue = nodeleft.node.dvalue * noderight.node.dvalue;
    }
}

void Validator::ValidateOperDiv(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    EXPR_CHECK_OPERANDS_VTYPE_NONE;

    if (nodeleft.vtype == ValueTypeString || noderight.vtype == ValueTypeString)
        EXPR_ERROR("Operation \'/\' not applicable to strings.");
    if (nodeleft.vtype == noderight.vtype)
        node.vtype = nodeleft.vtype;
    else if ((nodeleft.vtype == ValueTypeSingle && noderight.vtype == ValueTypeInteger) ||
        (nodeleft.vtype == ValueTypeInteger && noderight.vtype == ValueTypeSingle))
        node.vtype = ValueTypeSingle;
    else
        EXPR_ERROR("Value types are incompatible.");

    node.constval = (nodeleft.constval && noderight.constval);
    if (node.constval)
    {
        if (noderight.node.dvalue == 0)
            EXPR_ERROR("Division by zero.");

        node.node.dvalue = nodeleft.node.dvalue / noderight.node.dvalue;
    }
}

void Validator::ValidateOperDivInt(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    EXPR_CHECK_OPERANDS_VTYPE_NONE;

    if (nodeleft.vtype == ValueTypeString || noderight.vtype == ValueTypeString)
        EXPR_ERROR("Operation \'MOD\' not applicable to strings.");

    node.vtype = ValueTypeInteger;
    node.constval = (nodeleft.constval && noderight.constval);
    if (node.constval)
    {
        if (((int)noderight.node.dvalue) == 0)
            EXPR_ERROR("Division by zero.");

        node.node.dvalue = ((int)nodeleft.node.dvalue) / ((int)noderight.node.dvalue);
    }
}

void Validator::ValidateOperMod(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    EXPR_CHECK_OPERANDS_VTYPE_NONE;

    if (nodeleft.vtype == ValueTypeString || noderight.vtype == ValueTypeString)
        EXPR_ERROR("Operation \'\\\' not applicable to strings.");

    node.vtype = ValueTypeInteger;
    node.constval = (nodeleft.constval && noderight.constval);
    if (node.constval)
    {
        int ivalueright = (int)noderight.node.dvalue;
        if (ivalueright == 0)
            node.node.dvalue = 0;
        else
            node.node.dvalue = ((int)nodeleft.node.dvalue) % ivalueright;
    }
}

void Validator::ValidateOperPower(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    EXPR_CHECK_OPERANDS_VTYPE_NONE;

    if (nodeleft.vtype == ValueTypeString || noderight.vtype == ValueTypeString)
        EXPR_ERROR("Operation \'\\\' not applicable to strings.");
    if (nodeleft.vtype == noderight.vtype)
        node.vtype = nodeleft.vtype;
    else if ((nodeleft.vtype == ValueTypeSingle && noderight.vtype == ValueTypeInteger) ||
        (nodeleft.vtype == ValueTypeInteger && noderight.vtype == ValueTypeSingle))
        node.vtype = ValueTypeSingle;
    else
        EXPR_ERROR("Value types are incompatible.");

    node.constval = (nodeleft.constval && noderight.constval);
    if (node.constval)
    {
        node.node.dvalue = pow(nodeleft.node.dvalue, noderight.node.dvalue);
        if (!std::isfinite(node.node.dvalue))
            EXPR_ERROR("Bad result of power operation in const expression.");
    }
}

void Validator::ValidateOperEqual(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    EXPR_CHECK_OPERANDS_VTYPE_NONE;

    node.vtype = ValueTypeInteger;
    node.constval = (nodeleft.constval && noderight.constval);
    if (node.constval)
    {
        node.node.dvalue = (nodeleft.node.dvalue == noderight.node.dvalue) ? -1 : 0;
    }
}

void Validator::ValidateOperNotEqual(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    EXPR_CHECK_OPERANDS_VTYPE_NONE;

    node.vtype = ValueTypeInteger;
    node.constval = (nodeleft.constval && noderight.constval);
    if (node.constval)
    {
        node.node.dvalue = (nodeleft.node.dvalue != noderight.node.dvalue) ? -1 : 0;
    }
}

void Validator::ValidateOperLess(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    EXPR_CHECK_OPERANDS_VTYPE_NONE;

    node.vtype = ValueTypeInteger;
    node.constval = (nodeleft.constval && noderight.constval);
    if (node.constval)
    {
        node.node.dvalue = (nodeleft.node.dvalue < noderight.node.dvalue) ? -1 : 0;
    }
}

void Validator::ValidateOperLessOrEqual(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    EXPR_CHECK_OPERANDS_VTYPE_NONE;

    node.vtype = ValueTypeInteger;
    node.constval = (nodeleft.constval && noderight.constval);
    if (node.constval)
    {
        node.node.dvalue = (nodeleft.node.dvalue <= noderight.node.dvalue) ? -1 : 0;
    }
}

void Validator::ValidateOperGreater(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    EXPR_CHECK_OPERANDS_VTYPE_NONE;

    node.vtype = ValueTypeInteger;
    node.constval = (nodeleft.constval && noderight.constval);
    if (node.constval)
    {
        node.node.dvalue = (nodeleft.node.dvalue > noderight.node.dvalue) ? -1 : 0;
    }
}

void Validator::ValidateOperGreaterOrEqual(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    EXPR_CHECK_OPERANDS_VTYPE_NONE;

    node.vtype = ValueTypeInteger;
    node.constval = (nodeleft.constval && noderight.constval);
    if (node.constval)
    {
        node.node.dvalue = (nodeleft.node.dvalue >= noderight.node.dvalue) ? -1 : 0;
    }
}

void Validator::ValidateOperAnd(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    EXPR_CHECK_OPERANDS_VTYPE_NONE;

    if (nodeleft.vtype == ValueTypeString || noderight.vtype == ValueTypeString)
        EXPR_ERROR("Operation \'AND\' not applicable to strings.");

    node.vtype = ValueTypeInteger;
    node.constval = (nodeleft.constval && noderight.constval);
    if (node.constval)
    {
        int ivalueleft = (int)nodeleft.node.dvalue;
        int ivalueright = (int)noderight.node.dvalue;
        node.node.dvalue = ivalueleft & ivalueright;
    }
}

void Validator::ValidateOperOr(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    EXPR_CHECK_OPERANDS_VTYPE_NONE;

    if (nodeleft.vtype == ValueTypeString || noderight.vtype == ValueTypeString)
        EXPR_ERROR("Operation \'AND\' not applicable to strings.");

    node.vtype = ValueTypeInteger;
    node.constval = (nodeleft.constval && noderight.constval);
    if (node.constval)
    {
        int ivalueleft = (int)nodeleft.node.dvalue;
        int ivalueright = (int)noderight.node.dvalue;
        node.node.dvalue = ivalueleft | ivalueright;
    }
}


// Function validation ///////////////////////////////////////////////

void Validator::ValidateFuncSin(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() != 1)
        EXPR_ERROR("One argument expected.");

    ExpressionModel& expr1 = node.args[0];
    if (!CheckIntegerOrSingleExpression(expr1))
        return;

    node.vtype = ValueTypeSingle;
    node.constval = false;

    if (expr1.IsConstExpression())
    {
        node.constval = true;
        node.node.dvalue = sin(expr1.GetConstExpressionDValue());
    }
}

void Validator::ValidateFuncCos(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() != 1)
        EXPR_ERROR("One argument expected.");

    ExpressionModel& expr1 = node.args[0];
    if (!CheckIntegerOrSingleExpression(expr1))
        return;

    node.vtype = ValueTypeSingle;
    node.constval = false;

    if (expr1.IsConstExpression())
    {
        node.constval = true;
        node.node.dvalue = cos(expr1.GetConstExpressionDValue());
    }
}

void Validator::ValidateFuncTan(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() != 1)
        EXPR_ERROR("One argument expected.");

    ExpressionModel& expr1 = node.args[0];
    if (!CheckIntegerOrSingleExpression(expr1))
        return;

    node.vtype = ValueTypeSingle;
    node.constval = false;

    if (expr1.IsConstExpression())
    {
        node.constval = true;
        node.node.dvalue = tan(expr1.GetConstExpressionDValue());
    }
}

void Validator::ValidateFuncAtn(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() != 1)
        EXPR_ERROR("One argument expected.");

    ExpressionModel& expr1 = node.args[0];
    if (!CheckIntegerOrSingleExpression(expr1))
        return;

    node.vtype = ValueTypeSingle;
    node.constval = false;

    if (expr1.IsConstExpression())
    {
        node.constval = true;
        node.node.dvalue = atan(expr1.GetConstExpressionDValue());
    }
}

void Validator::ValidateFuncPi(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() != 0)
        EXPR_ERROR("No arguments expected.");

    node.vtype = ValueTypeSingle;
    node.constval = true;
    node.node.dvalue = 3.141593;
}

void Validator::ValidateFuncExp(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() != 1)
        EXPR_ERROR("One argument expected.");

    ExpressionModel& expr1 = node.args[0];
    if (!CheckIntegerOrSingleExpression(expr1))
        return;

    node.vtype = ValueTypeSingle;
    node.constval = false;

    if (expr1.IsConstExpression())
    {
        node.constval = true;
        node.node.dvalue = exp(expr1.GetConstExpressionDValue());
    }
}

void Validator::ValidateFuncLog(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() != 1)
        EXPR_ERROR("One argument expected.");

    ExpressionModel& expr1 = node.args[0];
    if (!CheckIntegerOrSingleExpression(expr1))
        return;

    node.vtype = ValueTypeSingle;
    node.constval = false;

    if (expr1.IsConstExpression())
    {
        node.constval = true;
        node.node.dvalue = log(expr1.GetConstExpressionDValue());
    }
}

void Validator::ValidateFuncAbs(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() != 1)
        EXPR_ERROR("One argument expected.");

    ExpressionModel& expr1 = node.args[0];
    if (!CheckIntegerOrSingleExpression(expr1))
        return;

    ValueType vtype = expr1.GetExpressionValueType();
    node.vtype = vtype;
    node.constval = false;

    if (expr1.IsConstExpression())
    {
        node.constval = true;
        node.node.dvalue = std::abs(expr1.GetConstExpressionDValue());
    }
}

void Validator::ValidateFuncFix(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() != 1)
        EXPR_ERROR("One argument expected.");

    ExpressionModel& expr1 = node.args[0];
    if (!CheckIntegerOrSingleExpression(expr1))
        return;

    node.vtype = ValueTypeInteger;
    node.constval = false;

    if (expr1.IsConstExpression())
    {
        node.constval = true;
        node.node.dvalue = (int)(expr1.GetConstExpressionDValue());
    }
}

void Validator::ValidateFuncInt(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() != 1)
        EXPR_ERROR("One argument expected.");

    ExpressionModel& expr1 = node.args[0];
    if (!CheckIntegerOrSingleExpression(expr1))
        return;

    node.vtype = ValueTypeInteger;
    node.constval = false;

    if (expr1.IsConstExpression())
    {
        node.constval = true;
        node.node.dvalue = std::floor(expr1.GetConstExpressionDValue());
    }
}

void Validator::ValidateFuncSgn(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() != 1)
        EXPR_ERROR("One argument expected.");

    ExpressionModel& expr1 = node.args[0];
    if (!CheckIntegerOrSingleExpression(expr1))
        return;

    node.vtype = ValueTypeInteger;
    node.constval = false;

    if (expr1.IsConstExpression())
    {
        node.constval = true;
        double dvalue = expr1.GetConstExpressionDValue();
        if (dvalue == 0)
            node.node.dvalue = 0;
        else
            node.node.dvalue = dvalue > 0 ? 1 : -1;
    }
}

void Validator::ValidateFuncRnd(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() != 1)
        EXPR_ERROR("One argument expected.");

    ExpressionModel& expr1 = node.args[0];
    if (!CheckIntegerOrSingleExpression(expr1))
        return;

    node.vtype = ValueTypeSingle;
    node.constval = false;
}

void Validator::ValidateFuncFre(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() > 1)
        EXPR_ERROR("Zero or one arguments expected.");

    if (node.args.size() > 0)
    {
        ExpressionModel& expr1 = node.args[0];
        ValidateExpression(expr1);
        //NOTE: Could be of type Integer/Single or String
    }
}

void Validator::ValidateFuncCint(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() != 1)
        EXPR_ERROR("One argument expected.");

    ExpressionModel& expr1 = node.args[0];
    if (!CheckIntegerOrSingleExpression(expr1))
        return;

    node.vtype = ValueTypeInteger;
    node.constval = false;

    if (expr1.IsConstExpression())
    {
        node.constval = true;
        node.node.dvalue = (int)(expr1.GetConstExpressionDValue());
    }
}

void Validator::ValidateFuncCsng(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() != 1)
        EXPR_ERROR("One argument expected.");

    ExpressionModel& expr1 = node.args[0];
    if (!CheckIntegerOrSingleExpression(expr1))
        return;

    node.vtype = ValueTypeSingle;
    node.constval = false;

    if (expr1.IsConstExpression())
    {
        node.constval = true;
        node.node.dvalue = expr1.GetConstExpressionDValue();
    }
}

void Validator::ValidateFuncPeek(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() != 1)
        EXPR_ERROR("One argument expected.");

    ExpressionModel& expr1 = node.args[0];
    if (!CheckIntegerOrSingleExpression(expr1))
        return;

    node.vtype = ValueTypeInteger;
    node.constval = false;
}

void Validator::ValidateFuncInp(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() != 2)
        EXPR_ERROR("Two arguments expected.");

    ExpressionModel& expr1 = node.args[0];
    if (!CheckIntegerOrSingleExpression(expr1))
        return;

    ExpressionModel& expr2 = node.args[1];
    if (!CheckIntegerOrSingleExpression(expr2))
        return;

    node.vtype = ValueTypeInteger;
    node.constval = false;
}

void Validator::ValidateFuncAsc(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() != 1)
        EXPR_ERROR("One argument expected.");

    ExpressionModel& expr1 = node.args[0];
    if (!CheckStringExpression(expr1))
        return;

    node.vtype = ValueTypeInteger;
    node.constval = false;

    if (expr1.IsConstExpression())
    {
        node.constval = true;

        string svalue = expr1.GetConstExpressionSValue();
        if (svalue.empty())
            EXPR_ERROR("Function ASC parameter is empty.");
        node.node.dvalue = (int)svalue[0];  //TODO: depends on charset
    }
}

void Validator::ValidateFuncChr(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() != 1)
        EXPR_ERROR("One argument expected.");

    ExpressionModel& expr1 = node.args[0];
    if (!CheckIntegerOrSingleExpression(expr1))
        return;

    node.vtype = ValueTypeString;
    node.constval = false;

    if (expr1.IsConstExpression())
    {
        node.constval = true;
        int ivalue = (int)expr1.GetConstExpressionDValue();
        if (ivalue < 0 || ivalue > 255)
            EXPR_ERROR("Function CHR$ parameter is out of range.");

        node.node.svalue = (char)ivalue;

        m_source->RegisterConstString(node.node.svalue);
    }
}

void Validator::ValidateFuncLen(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() != 1)
        EXPR_ERROR("One argument expected.");

    ExpressionModel& expr1 = node.args[0];
    if (!CheckStringExpression(expr1))
        return;

    node.vtype = ValueTypeInteger;
    node.constval = false;

    if (expr1.IsConstExpression())
    {
        node.constval = true;
        node.node.dvalue = expr1.GetConstExpressionSValue().length();
    }
}

void Validator::ValidateFuncMid(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() < 2 || node.args.size() > 3)
        EXPR_ERROR("Two or three arguments expected.");

    ExpressionModel& expr1 = node.args[0];
    if (!CheckStringExpression(expr1))
        return;

    ExpressionModel& expr2 = node.args[1];
    if (!CheckIntegerOrSingleExpression(expr2))
        return;

    if (node.args.size() > 2)
    {
        ExpressionModel& expr3 = node.args[2];
        if (!CheckIntegerOrSingleExpression(expr3))
            return;
    }

    node.vtype = ValueTypeString;
    node.constval = false;

    if (expr1.IsConstExpression() && expr2.IsConstExpression() &&
        (node.args.size() < 3 || node.args[2].IsConstExpression()))
    {
        node.constval = true;

        string svalue1 = expr1.GetConstExpressionSValue();

        int ivalue2 = (int)expr2.GetConstExpressionDValue();
        if (ivalue2 < 1 || ivalue2 > 255)
            EXPR_ERROR("Function MID$ second parameter out of range 1..255.");

        if (svalue1.empty() || ivalue2 - 1 >= (int)svalue1.length())
            node.node.svalue.clear();
        else if (node.args.size() < 3)
            node.node.svalue = svalue1.substr(ivalue2 - 1);
        else
        {
            ExpressionModel& expr3 = node.args[2];
            int ivalue3 = (int)expr3.GetConstExpressionDValue();
            if (ivalue3 < 0)
                EXPR_ERROR("Function MID$ third parameter should not be negative.");
            node.node.svalue = svalue1.substr(ivalue2 - 1, ivalue3);
        }

        m_source->RegisterConstString(node.node.svalue);
    }
}

void Validator::ValidateFuncString(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() != 2)
        EXPR_ERROR("Two arguments expected.");

    ExpressionModel& expr1 = node.args[0];
    if (!CheckIntegerOrSingleExpression(expr1))
        return;

    ExpressionModel& expr2 = node.args[1];
    ValidateExpression(expr2);
    //NOTE: Integer/Single OR String expression

    node.vtype = ValueTypeString;
    node.constval = false;

    if (expr1.IsConstExpression() && expr2.IsConstExpression())
    {
        node.constval = true;

        int ivalue1 = (int)expr1.GetConstExpressionDValue();
        if (ivalue1 < 0 || ivalue1 > 255)
            EXPR_ERROR("Function STRING$ first parameter is not in range 0..255.");

        if (ivalue1 == 0)
            node.node.svalue.clear();
        if (expr2.GetExpressionValueType() == ValueTypeString)
        {
            string svalue2 = expr2.GetConstExpressionSValue();
            if (svalue2.empty())
                EXPR_ERROR("Function STRING$ second parameter is empty string.");
            char filler = svalue2[0];
            node.node.svalue = string(ivalue1, filler);
        }
        else  // Integer/Single
        {
            int ivalue2 = (int)expr2.GetConstExpressionDValue();
            if (ivalue2 < 0 || ivalue2 > 255)
                EXPR_ERROR("Function STRING$ second parameter is not in range 0..255.");
            char filler = (char)ivalue2;  //TODO: depends on charset
            node.node.svalue = string(ivalue1, filler);
        }

        m_source->RegisterConstString(node.node.svalue);
    }
}

void Validator::ValidateFuncVal(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() != 1)
        EXPR_ERROR("One argument expected.");

    ExpressionModel& expr1 = node.args[0];
    if (!CheckStringExpression(expr1))
        return;

    node.vtype = ValueTypeString;
    node.constval = false;

    if (expr1.IsConstExpression())
    {
        node.constval = true;

        //TODO
    }
}

void Validator::ValidateFuncInkey(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() != 0)
        EXPR_ERROR("No arguments expected.");

    node.vtype = ValueTypeString;
    node.constval = false;
}

void Validator::ValidateFuncStr(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() != 1)
        EXPR_ERROR("One argument expected.");

    ExpressionModel& expr1 = node.args[0];
    if (!CheckIntegerOrSingleExpression(expr1))
        return;

    node.vtype = ValueTypeString;
    node.constval = false;

    if (expr1.IsConstExpression())
    {
        node.constval = true;

        //TODO
    }
}

void Validator::ValidateFuncBin(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() != 1)
        EXPR_ERROR("One argument expected.");

    ExpressionModel& expr1 = node.args[0];
    if (!CheckIntegerOrSingleExpression(expr1))
        return;
    int ivalue = (int)expr1.GetConstExpressionDValue();
    if (ivalue < -32768 || ivalue > 32767)
        EXPR_ERROR("Function BIN$ parameter is out of range.");
    if (ivalue < 0)
        ivalue = 65536 - ivalue;  // 0..65535

    node.vtype = ValueTypeString;
    node.constval = false;

    if (expr1.IsConstExpression())
    {
        node.constval = true;

        std::stringstream ss;
        std::bitset<16> bits(ivalue);
        ss << bits;
        string svalue = ss.str();
        while (svalue.length() > 0 && svalue[0] == '0')
            svalue.erase(0, 1);
        node.node.svalue = svalue;
    }
}

void Validator::ValidateFuncOct(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() != 1)
        EXPR_ERROR("One argument expected.");

    ExpressionModel& expr1 = node.args[0];
    if (!CheckIntegerOrSingleExpression(expr1))
        return;
    int ivalue = (int)expr1.GetConstExpressionDValue();
    if (ivalue < -32768 || ivalue > 32767)
        EXPR_ERROR("Function OCT$ parameter is out of range.");
    if (ivalue < 0)
        ivalue = 65536 - ivalue;  // 0..65535

    node.vtype = ValueTypeString;
    node.constval = false;

    if (expr1.IsConstExpression())
    {
        node.constval = true;

        std::stringstream ss;
        ss << std::oct << ivalue;
        node.node.svalue = ss.str();
    }
}

void Validator::ValidateFuncHex(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() != 1)
        EXPR_ERROR("One argument expected.");

    ExpressionModel& expr1 = node.args[0];
    if (!CheckIntegerOrSingleExpression(expr1))
        return;
    int ivalue = (int)expr1.GetConstExpressionDValue();
    if (ivalue < -32768 || ivalue > 32767)
        EXPR_ERROR("Function HEX$ parameter is out of range.");
    if (ivalue < 0)
        ivalue = 65536 - ivalue;  // 0..65535

    node.vtype = ValueTypeString;
    node.constval = false;

    if (expr1.IsConstExpression())
    {
        node.constval = true;

        std::stringstream ss;
        ss << std::hex << ivalue;
        string svalue = ss.str();
        std::transform(svalue.begin(), svalue.end(), svalue.begin(), ::toupper);
        node.node.svalue = svalue;
    }
}

void Validator::ValidateFuncCsrlinPosLpos(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() > 1)
        EXPR_ERROR("Zero or one arguments expected.");

    if (node.args.size() > 0)
    {
        ExpressionModel& expr1 = node.args[0];
        ValidateExpression(expr1);
    }

    node.vtype = ValueTypeInteger;
    node.constval = false;
}

void Validator::ValidateFuncEof(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() != 0)
        EXPR_ERROR("No arguments expected.");

    node.vtype = ValueTypeInteger;
    node.constval = false;
}

void Validator::ValidateFuncPoint(ExpressionModel& expr, ExpressionNode& node)
{
    if (node.args.size() != 2)
        EXPR_ERROR("Two arguments expected.");

    ExpressionModel& expr1 = node.args[0];
    if (!CheckIntegerOrSingleExpression(expr1))
        return;

    ExpressionModel& expr2 = node.args[1];
    if (!CheckIntegerOrSingleExpression(expr2))
        return;

    node.vtype = ValueTypeInteger;
    node.constval = false;
}


//////////////////////////////////////////////////////////////////////
