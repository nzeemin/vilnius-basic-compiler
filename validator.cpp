
#include <cassert>
#include <iomanip>
#include <string>

#include "main.h"


//////////////////////////////////////////////////////////////////////


const ValidatorKeywordSpec Validator::m_keywordspecs[] =
{
    { KeywordBEEP,	    &Validator::ValidateNothing },
    { KeywordCLEAR,	    &Validator::ValidateClear },
    { KeywordCLS,	    &Validator::ValidateNothing },
    { KeywordCOLOR,     &Validator::ValidateColor },
    { KeywordDIM,       &Validator::ValidateDim },
    { KeywordDRAW,      &Validator::ValidateDraw },
    { KeywordEND,	    &Validator::ValidateNothing },
    { KeywordFOR,       &Validator::ValidateFor },
    { KeywordREM,       &Validator::ValidateNothing },
    { KeywordGOSUB,     &Validator::ValidateGotoGosub },
    { KeywordGOTO,      &Validator::ValidateGotoGosub },
    { KeywordIF,        &Validator::ValidateIf },
    { KeywordINPUT,     &Validator::ValidateInput },
    { KeywordLET,       &Validator::ValidateLet },
    { KeywordLOCATE,    &Validator::ValidateLocate },
    { KeywordNEXT,      &Validator::ValidateNext },
    { KeywordON,        &Validator::ValidateOn },
    { KeywordOUT,       &Validator::ValidateOut },
    { KeywordPOKE,      &Validator::ValidatePoke },
    { KeywordPRINT,     &Validator::ValidatePrint },
    { KeywordRESTORE,   &Validator::ValidateRestore },
    { KeywordRETURN,    &Validator::ValidateNothing },
    { KeywordSCREEN,    &Validator::ValidateScreen },
    { KeywordSTOP,      &Validator::ValidateNothing },
    { KeywordTROFF,     &Validator::ValidateNothing },
    { KeywordTRON,      &Validator::ValidateNothing },
    { KeywordWIDTH,     &Validator::ValidateWidth },
};

const ValidatorOperSpec Validator::m_operspecs[] =
{
    { "+",              &Validator::ValidateOperPlus },
    { "-",              &Validator::ValidateOperMinus },
    { "*",              &Validator::ValidateOperMul },
    { "/",              &Validator::ValidateOperDiv },
    { "\\",             &Validator::ValidateOperDivInt },
    { "^",              &Validator::ValidateOperPower },
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
    { KeywordCINT,      &Validator::ValidateFuncCint },
    { KeywordCSNG,      &Validator::ValidateFuncCsng },
    { KeywordPEEK,      &Validator::ValidateFuncPeek },
    { KeywordINP,       &Validator::ValidateFuncInp },
    { KeywordASC,       &Validator::ValidateFuncAsc },
    { KeywordCHR,       &Validator::ValidateFuncChr },
    { KeywordLEN,       &Validator::ValidateFuncLen },
    { KeywordMID,       &Validator::ValidateFuncMid },
    { KeywordSTRING,    &Validator::ValidateFuncString },
};

Validator::Validator(SourceModel* source)
{
    assert(source != nullptr);
    m_source = source;

    m_lineindex = -1;
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

    SourceLineModel& line = m_source->lines[m_lineindex];

    // Find validator implementation
    KeywordIndex keyword = line.statement.keyword;
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
        Error(line, "Validator not found for keyword " + GetKeywordString(keyword) + ".");
        return true;
    }

    (this->*methodref)(line);

    return true;
}

void Validator::Error(SourceLineModel& line, string message)
{
    std::cerr << "ERROR at line " << line.number << " - " << message << std::endl;
    line.error = true;
    RegisterError();
}

void Validator::Error(ExpressionModel& expr, string message)
{
    std::cerr << "ERROR in expression - " << message << std::endl;
    RegisterError();
}

void Validator::Error(ExpressionModel& expr, const ExpressionNode& node, string message)
{
    std::cerr << "ERROR in expression at " << node.node.line << ":" << node.node.pos << " - " << message << std::endl;
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

    //TODO: Unary plus/minus with one operand only
    if (node.node.type == TokenTypeOperation && node.left >= 0 && node.right >= 0)
    {
        const ExpressionNode& nodeleft = expr.nodes[node.left];
        const ExpressionNode& noderight = expr.nodes[node.right];

        if (nodeleft.vtype == ValueTypeNone || noderight.vtype == ValueTypeNone)
        {
            std::cerr << "ERROR in expression at " << node.node.line << ":" << node.node.pos << " - Cannot calculate value type for the node." << std::endl;
            exit(EXIT_FAILURE);
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
            std::cerr << "ERROR in expression at " << node.node.line << ":" << node.node.pos << " - TODO validate operator \'" + text + "\'." << std::endl;
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
            std::cerr << "ERROR in expression at " << node.node.line << ":" << node.node.pos << " - TODO validate function " + GetKeywordString(keyword) << std::endl;
            return;
        }

        (this->*methodref)(expr, node);
    }

    //TODO
}

bool Validator::CheckIntegerExpression(SourceLineModel& model, ExpressionModel& expr)
{
    if (expr.IsEmpty())
    {
        Error(model, "Expression should not be empty.");
        return false;
    }
    
    ValidateExpression(expr, expr.root);
    
    //TODO: Check the expression result is integer

    return true;
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
        Error(expr, "Expression type should be Integer or Single.");
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
    if (root.vtype != ValueTypeInteger && root.vtype != ValueTypeSingle)
    {
        Error(expr, "Expression type should be String.");
        return false;
    }

    return true;
}

// Statement validation //////////////////////////////////////////////

#define MODEL_ERROR(msg) \
    { Error(model, msg); return; }

void Validator::ValidateNothing(SourceLineModel& model)
{
    // Nothing to validate
}

void Validator::ValidateClear(SourceLineModel& model)
{
    if (model.args.size() == 0)
        MODEL_ERROR("Expression expected.");

    ExpressionModel& expr1 = model.args[0];
    if (!CheckIntegerExpression(model, expr1))
        return;
    
    if (model.args.size() > 1)
    {
        ExpressionModel& expr2 = model.args[1];
        if (!CheckIntegerExpression(model, expr2))
            return;
    }
    if (model.args.size() > 2)
        MODEL_ERROR("Too many expressions.");
}

void Validator::ValidateColor(SourceLineModel& model)
{
    if (model.args.size() == 0)
        MODEL_ERROR("Expression expected.");

    {
        ExpressionModel& expr1 = model.args[0];
        if (!expr1.IsEmpty() && !CheckIntegerExpression(model, expr1))
            return;
    }
    if (model.args.size() > 1)
    {
        ExpressionModel& expr2 = model.args[1];
        if (!expr2.IsEmpty() && !CheckIntegerExpression(model, expr2))
            return;
    }
    if (model.args.size() > 2)
    {
        ExpressionModel& expr3 = model.args[2];
        if (!expr3.IsEmpty() && !CheckIntegerExpression(model, expr3))
            return;
    }

    if (model.args.size() > 3)
        MODEL_ERROR("Too many expressions.");
}

void Validator::ValidateDim(SourceLineModel& model)
{
    for (auto it = std::begin(model.variables); it != std::end(model.variables); ++it)
    {
        if (!m_source->RegisterVariable(*it))
            MODEL_ERROR("Variable redefinition for " + it->name + ".");
    }
}

void Validator::ValidateDraw(SourceLineModel& model)
{
    if (model.params.size() == 0)
        MODEL_ERROR("Parameter expected.");

    Token& token = model.params[0];
    if (token.type != TokenTypeString)
        MODEL_ERROR("String parameter expected.");
}

void Validator::ValidateFor(SourceLineModel& model)
{
    if (model.ident.type != TokenTypeIdentifier)
        MODEL_ERROR("Identifier expected.");

    VariableModel var;
    var.name = GetCanonicVariableName(model.ident.text);
    m_source->RegisterVariable(var);

    // Add FOR variable to FOR/NEXT stack
    ValidatorForSpec forspec;
    forspec.varname = var.name;
    forspec.linenum = model.number;
    m_fornextstack.push_back(forspec);

    if (model.args.size() < 2)
        MODEL_ERROR("Two expressions expected.");

    ExpressionModel& expr1 = model.args[0];
    if (!CheckIntegerExpression(model, expr1))
        return;

    ExpressionModel& expr2 = model.args[1];
    if (!CheckIntegerExpression(model, expr2))
        return;

    if (model.args.size() > 2)  // has STEP expression
    {
        ExpressionModel& expr3 = model.args[1];
        if (!CheckIntegerExpression(model, expr3))
            return;

        if (model.args.size() > 3)
            MODEL_ERROR("Too many expressions.");
    }
}

void Validator::ValidateGotoGosub(SourceLineModel& model)
{
    if (!m_source->IsLineNumberExists(model.paramline))
        MODEL_ERROR("Invalid line number " + std::to_string(model.paramline) + ".");
}

void Validator::ValidateIf(SourceLineModel& model)
{
    if (model.args.size() == 0)
        MODEL_ERROR("Expression expected.");
    //TODO: Check for non-empty expression
    if (model.args.size() > 1)
        MODEL_ERROR("Too many expressions.");

    if (model.params.size() == 0)
        MODEL_ERROR("Parameter expected.");

    // Line number for THEN
    Token& param1 = model.params[0];
    int linenum1 = (int)param1.dvalue;
    if (!m_source->IsLineNumberExists(linenum1))
        MODEL_ERROR("Invalid line number " + std::to_string(linenum1) + ".");

    if (model.params.size() > 1)
    {
        // Line number for ELSE
        Token& param2 = model.params[1];
        int linenum2 = (int)param2.dvalue;
        if (!m_source->IsLineNumberExists(linenum2))
            MODEL_ERROR("Invalid line number " + std::to_string(linenum2) + ".");
    }
    
    if (model.params.size() > 2)
        MODEL_ERROR("Too many parameters.");
}

void Validator::ValidateInput(SourceLineModel& model)
{
    if (model.params.size() > 1)
        MODEL_ERROR("Too many parameters.");
    if (model.params.size() > 0)
    {
        Token& param = model.params[0];
        if (param.type != TokenTypeString)
            MODEL_ERROR("Parameter should be of type String.");
        m_source->RegisterConstString(param.text);
    }

    if (model.variables.size() == 0)
        MODEL_ERROR("Variable(s) expected.");

    for (auto it = std::begin(model.variables); it != std::end(model.variables); ++it)
    {
        m_source->RegisterVariable(*it);
    }
}

void Validator::ValidateLet(SourceLineModel& model)
{
    if (model.variables.size() != 1)
        MODEL_ERROR("One variable expected.");

    VariableModel& var = model.variables[0];
    m_source->RegisterVariable(var);

    if (model.args.size() != 1)
        MODEL_ERROR("One expression expected.");

    ExpressionModel& expr = model.args[0];
    ValidateExpression(expr);

    //TODO: Check types compatibility between variable and expression
}

void Validator::ValidateLocate(SourceLineModel& model)
{
    if (model.args.size() == 0)
        MODEL_ERROR("Expression expected.");

    ExpressionModel& expr1 = model.args[0];
    if (!expr1.IsEmpty() && !CheckIntegerExpression(model, expr1))
        return;

    if (model.args.size() > 1)
    {
        ExpressionModel& expr2 = model.args[1];
        if (!expr2.IsEmpty() && !CheckIntegerExpression(model, expr2))
            return;
    }
    if (model.args.size() > 2)
    {
        ExpressionModel& expr3 = model.args[2];
        if (!expr3.IsEmpty() && !CheckIntegerExpression(model, expr3))
            return;
    }

    if (model.args.size() > 3)
        MODEL_ERROR("Too many expressions.");
}

void Validator::ValidateNext(SourceLineModel& model)
{
    if (model.params.empty())  // NEXT without parameters
    {
        if (m_fornextstack.empty())
            MODEL_ERROR("NEXT without FOR.");

        ValidatorForSpec forspec = m_fornextstack.back();
        m_fornextstack.pop_back();

        Token tokenvar;
        tokenvar.type = TokenTypeIdentifier;
        tokenvar.text = forspec.varname;
        model.params.push_back(tokenvar);

        // link NEXT to the corresponding FOR
        model.paramline = forspec.linenum;

        // link FOR to the NEXT line number
        SourceLineModel& linefor = m_source->GetSourceLine(forspec.linenum);
        linefor.paramline = model.number;

        return;
    }

    //TODO: need to change the model for case of several NEXT variables
    for (auto it = std::begin(model.params); it != std::end(model.params); ++it)
    {
        string varname = GetCanonicVariableName(it->text);
        if (!m_source->IsVariableRegistered(varname))
            MODEL_ERROR("Variable not found:" + varname + ".");

        ValidatorForSpec forspec = m_fornextstack.back();
        m_fornextstack.pop_back();

        if (forspec.varname != varname)
            MODEL_ERROR("NEXT variable expected: " + forspec.varname + ", found:" + varname + ".");

        // link NEXT to the corresponding FOR
        model.paramline = forspec.linenum;

        // link FOR to the NEXT line number
        SourceLineModel& linefor = m_source->GetSourceLine(forspec.linenum);
        linefor.paramline = model.number;
    }
}

void Validator::ValidateOn(SourceLineModel& model)
{
    if (model.args.size() != 1)
        MODEL_ERROR("One Expression expected.");

    ExpressionModel& expr = model.args[0];
    if (!CheckIntegerExpression(model, expr))
        return;

    if (model.params.size() == 0)
        MODEL_ERROR("Parameters expected.");

    for (auto it = std::begin(model.params); it != std::end(model.params); ++it)
    {
        Token& param = *it;
        if (param.type != TokenTypeNumber || !param.IsDValueInteger())
            MODEL_ERROR("Integer parameter expected.");

        int linenum = (int)param.dvalue;
        if (!m_source->IsLineNumberExists(linenum))
            MODEL_ERROR("Invalid line number " + std::to_string(linenum));
    }
}

void Validator::ValidateOut(SourceLineModel& model)
{
    if (model.args.size() != 3)
        MODEL_ERROR("Three expressions expected.");

    ExpressionModel& expr1 = model.args[0];
    if (!CheckIntegerExpression(model, expr1))
        return;

    ExpressionModel& expr2 = model.args[1];
    if (!CheckIntegerExpression(model, expr2))
        return;

    ExpressionModel& expr3 = model.args[2];
    if (!CheckIntegerExpression(model, expr3))
        return;
}

void Validator::ValidatePoke(SourceLineModel& model)
{
    if (model.args.size() != 2)
        MODEL_ERROR("Two expressions expected.");

    ExpressionModel& expr1 = model.args[0];
    if (!CheckIntegerExpression(model, expr1))
        return;

    ExpressionModel& expr2 = model.args[1];
    if (!CheckIntegerExpression(model, expr2))
        return;
}

void Validator::ValidatePrint(SourceLineModel& model)
{
    for (auto it = std::begin(model.args); it != std::end(model.args); ++it)
    {
        const ExpressionModel& expr = *it;
        if (expr.IsEmpty())
            MODEL_ERROR("Expressions should not be empty.");
        //TODO
    }
}

void Validator::ValidateRestore(SourceLineModel& model)
{
    if (model.paramline != 0)  // optional param
    {
        if (!m_source->IsLineNumberExists(model.paramline))
            MODEL_ERROR("Invalid line number " + std::to_string(model.paramline));
    }
}

void Validator::ValidateScreen(SourceLineModel& model)
{
    if (model.params.size() < 1)
        MODEL_ERROR("Parameter expected.");

    Token& token = model.params[0];
    if (token.type != TokenTypeNumber)
        MODEL_ERROR("Numeric parameter expected.");
}

void Validator::ValidateWidth(SourceLineModel& model)
{
    //TODO
}


// Operation validation //////////////////////////////////////////////
// Every operation validator function should:
// 1. Validate operands
// 2. Calculate constval flag
// 3. Calculate result vtype
// 4. Calculate dvalue for const sub-expression

#define EXPR_ERROR(msg) \
    { Error(expr, node, msg); return; }
#define EXPR_CHECK_OPERANDS_VTYPE_NONE \
    { if (nodeleft.vtype == ValueTypeNone) { Error(expr, nodeleft, "Operand vtype not defined."); return; } \
      if (noderight.vtype == ValueTypeNone) { Error(expr, noderight, "Operand vtype not defined."); return; } }

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
        //TODO: Make sum for ValueTypeString
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
        EXPR_ERROR("Operation \'\\\' not applicable to strings.");

    node.vtype = ValueTypeInteger;
    node.constval = (nodeleft.constval && noderight.constval);
    if (node.constval)
    {
        if (((int)noderight.node.dvalue) == 0)
            EXPR_ERROR("Division by zero.");

        node.node.dvalue = ((int)nodeleft.node.dvalue) / ((int)noderight.node.dvalue);
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
        //TODO: Calculate value: node.node.dvalue = ...
    }
}

void Validator::ValidateFuncChr(ExpressionModel& expr, ExpressionNode& node)
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
        //TODO: Calculate value: node.node.dvalue = ...
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
        //TODO: Calculate value: node.node.dvalue = ...
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

    if (expr1.IsConstExpression())
    {
        node.constval = true;
        //TODO: Calculate value: node.node.dvalue = ...
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
}


//////////////////////////////////////////////////////////////////////
