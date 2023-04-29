
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
    { "-",              &Validator::ValidateOperMinus },
    { "+",              &Validator::ValidateOperPlus },
};

const ValidatorFuncSpec Validator::m_funcspecs[] =
{
    { KeywordPEEK,      &Validator::ValidateFuncPeek },
    { KeywordRND,       &Validator::ValidateFuncRnd },
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
    for (int i = 0; i < sizeof(m_keywordspecs) / sizeof(m_keywordspecs[0]); i++)
    {
        if (keyword == m_keywordspecs[i].keyword)
        {
            methodref = m_keywordspecs[i].methodref;
            break;
        }
    }

    if (methodref == nullptr)
    {
        Error(line, "Validator not found for the keyword.");
    }
    else
    {
        (this->*methodref)(line);
    }

    return true;
}

void Validator::Error(SourceLineModel& line, string message)
{
    std::cerr << "ERROR at line " << line.number << " - " << message << std::endl;
    line.error = true;
    RegisterError();
}

void Validator::Error(ExpressionModel& expr, ExpressionNode& node, string message)
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
            std::cerr << "ERROR in expression at " << node.node.line << ":" << node.node.pos << " - Cannot calculate value type for the node.";
            exit(EXIT_FAILURE);
        }

        // Find validator implementation
        string text = node.node.text;
        ValidatorOperMethodRef methodref = nullptr;
        for (int i = 0; i < sizeof(m_operspecs) / sizeof(ValidatorFuncSpec); i++)
        {
            if (text == m_operspecs[i].text)
            {
                methodref = m_operspecs[i].methodref;
                break;
            }
        }

        if (methodref != nullptr)
            (this->*methodref)(expr, node, nodeleft, noderight);
    }

    if (node.node.type == TokenTypeKeyword)  // Check is it function, validate function
    {
        // Find validator implementation
        KeywordIndex keyword = node.node.keyword;
        ValidatorFuncMethodRef methodref = nullptr;
        for (int i = 0; i < sizeof(m_funcspecs) / sizeof(ValidatorFuncSpec); i++)
        {
            if (keyword == m_funcspecs[i].keyword)
            {
                methodref = m_funcspecs[i].methodref;
                break;
            }
        }

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
    for (size_t i = 0; i < model.variables.size(); i++)
    {
        VariableModel& var = model.variables[i];
        if (!m_source->RegisterVariable(var))
            MODEL_ERROR("Variable redefinition for " + var.name + ".");
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
    //TODO
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

    for (size_t i = 0; i < model.params.size(); i++)
    {
        Token& param = model.params[i];
        string varname = GetCanonicVariableName(param.text);
        if (!m_source->IsVariableRegistered(varname))
            MODEL_ERROR("Variable not found:" + varname + ".");

        //TODO: Process with FOR/NEXT stack
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

    for (size_t i = 0; i < model.params.size(); i++)
    {
        Token& param = model.params[i];
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
    //TODO
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

void Validator::ValidateOperMinus(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    if (nodeleft.vtype == ValueTypeNone)
        EXPR_ERROR("Operand vtype not defined.");
    if (noderight.vtype == ValueTypeNone)
        EXPR_ERROR("Operand vtype not defined.");

    if (nodeleft.vtype == ValueTypeString || noderight.vtype == ValueTypeString)
        EXPR_ERROR("Operation \'-\' not applicable to strings.");
    if (nodeleft.vtype == noderight.vtype)
        node.vtype = nodeleft.vtype;
    else if (nodeleft.vtype == ValueTypeSingle && noderight.vtype == ValueTypeInteger ||
        nodeleft.vtype == ValueTypeInteger && noderight.vtype == ValueTypeSingle)
        node.vtype = ValueTypeSingle;
    else
        EXPR_ERROR("Value types are incompatible.");

    node.constval = (nodeleft.constval && noderight.constval);
    if (node.constval)
    {
        node.node.dvalue = nodeleft.node.dvalue - noderight.node.dvalue;
    }
}

void Validator::ValidateOperPlus(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    if (nodeleft.vtype == ValueTypeNone)
        EXPR_ERROR("Operand vtype not defined.");
    if (noderight.vtype == ValueTypeNone)
        EXPR_ERROR("Operand vtype not defined.");

    if (nodeleft.vtype == noderight.vtype)
        node.vtype = nodeleft.vtype;
    else if (nodeleft.vtype == ValueTypeSingle && noderight.vtype == ValueTypeInteger ||
        nodeleft.vtype == ValueTypeInteger && noderight.vtype == ValueTypeSingle)
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


// Function validation ///////////////////////////////////////////////

void Validator::ValidateFuncPeek(SourceLineModel& model, ExpressionNode& node)
{
    if (model.args.size() != 1)
        MODEL_ERROR("One Expression expected.");

    ExpressionModel& expr = model.args[0];
    if (!CheckIntegerExpression(model, expr))
        return;

    node.vtype = ValueTypeInteger;
    node.constval = false;
}

void Validator::ValidateFuncRnd(SourceLineModel& model, ExpressionNode& node)
{
    if (model.args.size() != 1)
        MODEL_ERROR("One Expression expected.");

    ExpressionModel& expr = model.args[0];
    ValidateExpression(expr);
    //TODO: Expression type should be Integer of Single

    node.vtype = ValueTypeSingle;
    node.constval = false;
}


//////////////////////////////////////////////////////////////////////
