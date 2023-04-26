
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

bool Validator::CheckIntegerExpression(SourceLineModel& model, ExpressionModel& expr)
{
    if (expr.IsEmpty())
    {
        Error(model, "Expression should not be empty.");
        return false;
    }
    
    //TODO: Validate the expression
    
    //TODO: Check the expression result is integer

    return true;
}

void Validator::ValidateNothing(SourceLineModel& model)
{
    // Nothing to validate
}

void Validator::ValidateClear(SourceLineModel& model)
{
    if (model.args.size() == 0)
    {
        Error(model, "Expression expected.");
        return;
    }

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
    {
        Error(model, "Too many expressions.");
        return;
    }
}

void Validator::ValidateColor(SourceLineModel& model)
{
    if (model.args.size() == 0)
    {
        Error(model, "Expression expected.");
        return;
    }
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
    {
        Error(model, "Too many expressions.");
        return;
    }
}

void Validator::ValidateDim(SourceLineModel& model)
{
    for (size_t i = 0; i < model.variables.size(); i++)
    {
        VariableModel& var = model.variables[i];
        if (!m_source->RegisterVariable(var))
        {
            Error(model, "Variable redefinition for " + var.name + ".");
            return;
        }
    }
}

void Validator::ValidateDraw(SourceLineModel& model)
{
    if (model.params.size() == 0)
    {
        Error(model, "Parameter expected.");
        return;
    }
    Token& token = model.params[0];
    if (token.type != TokenTypeString)
    {
        Error(model, "String parameter expected.");
        return;
    }
}

void Validator::ValidateFor(SourceLineModel& model)
{
    if (model.ident.type != TokenTypeIdentifier)
    {
        Error(model, "Identifier expected.");
        return;
    }

    VariableModel var;
    var.name = GetCanonicVariableName(model.ident.text);
    m_source->RegisterVariable(var);

    //TODO: Add FOR to FOR/NEXT stack

    if (model.args.size() < 2)
    {
        Error(model, "Two expressions expected.");
        return;
    }

    ExpressionModel& expr1 = model.args[0];
    if (!CheckIntegerExpression(model, expr1))
        return;

    ExpressionModel& expr2 = model.args[1];
    if (!CheckIntegerExpression(model, expr2))
        return;

    if (model.args.size() > 2)
    {
        ExpressionModel& expr3 = model.args[1];
        if (!CheckIntegerExpression(model, expr3))
            return;

        if (model.args.size() > 3)
        {
            Error(model, "Too many expressions.");
            return;
        }
    }
}

void Validator::ValidateGotoGosub(SourceLineModel& model)
{
    if (!m_source->IsLineNumberExists(model.paramline))
        Error(model, "Invalid line number " + std::to_string(model.paramline) + ".");
}

void Validator::ValidateIf(SourceLineModel& model)
{
    if (model.params.size() == 0)
    {
        Error(model, "Parameter expected.");
        return;
    }
    Token& param1 = model.params[0];
    int linenum1 = (int)param1.dvalue;
    if (!m_source->IsLineNumberExists(linenum1))
    {
        Error(model, "Invalid line number " + std::to_string(linenum1) + ".");
        return;
    }

    if (model.params.size() > 1)
    {
        Token& param2 = model.params[1];
        int linenum2 = (int)param2.dvalue;
        if (!m_source->IsLineNumberExists(linenum2))
        {
            Error(model, "Invalid line number " + std::to_string(linenum2) + ".");
            return;
        }
    }
    
    if (model.params.size() > 2)
    {
        Error(model, "Too many parameters.");
        return;
    }
}

void Validator::ValidateInput(SourceLineModel& model)
{
    //TODO
}

void Validator::ValidateLet(SourceLineModel& model)
{
    //TODO
}

void Validator::ValidateLocate(SourceLineModel& model)
{
    if (model.args.size() == 0)
    {
        Error(model, "Expression expected.");
        return;
    }
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
    {
        Error(model, "Too many expressions.");
        return;
    }
}

void Validator::ValidateNext(SourceLineModel& model)
{
    //TODO: Process NEXT with FOR/NEXT stack

    for (size_t i = 0; i < model.params.size(); i++)
    {
        Token& param = model.params[i];
        string varname = GetCanonicVariableName(param.text);
        if (!m_source->IsVariableRegistered(varname))
        {
            Error(model, "Variable not found:" + varname + ".");
            return;
        }
    }
}

void Validator::ValidateOn(SourceLineModel& model)
{
    if (model.args.size() == 0)
    {
        Error(model, "Expression expected.");
        return;
    }
    if (model.args.size() > 1)
    {
        Error(model, "Too many expressions.");
        return;
    }

    ExpressionModel& expr1 = model.args[0];
    if (!expr1.IsEmpty() && !CheckIntegerExpression(model, expr1))
        return;

    if (model.params.size() == 0)
    {
        Error(model, "Parameters expected.");
        return;
    }

    for (size_t i = 0; i < model.params.size(); i++)
    {
        Token& param = model.params[i];
        if (param.type != TokenTypeNumber || !param.IsDValueInteger())
        {
            Error(model, "Integer parameter expected.");
            return;
        }
        int linenum = (int)param.dvalue;
        if (!m_source->IsLineNumberExists(linenum))
        {
            Error(model, "Invalid line number " + std::to_string(linenum));
            return;
        }
    }
}

void Validator::ValidateOut(SourceLineModel& model)
{
    if (model.args.size() != 3)
    {
        Error(model, "Three expressions expected.");
        return;
    }

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
    {
        Error(model, "Two expressions expected.");
        return;
    }

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
        {
            Error(model, "Invalid line number " + std::to_string(model.paramline));
            return;
        }
    }
}

void Validator::ValidateScreen(SourceLineModel& model)
{
    if (model.params.size() < 1)
    {
        Error(model, "Parameter expected.");
        return;
    }
    Token& token = model.params[0];
    if (token.type != TokenTypeNumber)
    {
        Error(model, "Numeric parameter expected.");
        return;
    }
}

void Validator::ValidateWidth(SourceLineModel& model)
{
    //TODO
}

//////////////////////////////////////////////////////////////////////
