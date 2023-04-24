
#include <cassert>
#include <iomanip>
#include <string>

#include "main.h"


//////////////////////////////////////////////////////////////////////


const ValidatorKeywordSpec Validator::m_keywordspecs[] =
{
    { KeywordBEEP,	    &Validator::ValidateNothing },
    { KeywordCLS,	    &Validator::ValidateNothing },
    { KeywordCOLOR,     &Validator::ValidateColor },
    { KeywordDIM,       &Validator::ValidateDim },
    { KeywordDRAW,      &Validator::ValidateDraw },
    { KeywordEND,	    &Validator::ValidateNothing },
    { KeywordREM,       &Validator::ValidateNothing },
    { KeywordGOSUB,     &Validator::ValidateGotoGosub },
    { KeywordGOTO,      &Validator::ValidateGotoGosub },
    { KeywordRESTORE,   &Validator::ValidateRestore },
    { KeywordRETURN,    &Validator::ValidateNothing },
    { KeywordSTOP,      &Validator::ValidateNothing },
    { KeywordTROFF,     &Validator::ValidateNothing },
    { KeywordTRON,      &Validator::ValidateNothing },
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

    if (methodref != nullptr)
    {
        (this->*methodref)(line);
    }
    else
    {
        //TODO error
    }

    return true;
}

void Validator::Error(SourceLineModel& line, string message)
{
    std::cerr << "ERROR at line " << line.number << " - " << message << std::endl;
    line.error = true;
    RegisterError();
}

void Validator::ValidateNothing(SourceLineModel& model)
{
    // Nothing to validate
}

void Validator::ValidateColor(SourceLineModel& model)
{
    //TODO: Check expr1 is empty, or valid and integer
    //TODO: Check expr2 is empty, or valid and integer
    //TODO: Check expr3 is empty, or valid and integer
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
    if (model.params.size() < 1)
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

void Validator::ValidateGotoGosub(SourceLineModel& model)
{
    m_source->CheckLineNumber(model, model.paramline);
}

void Validator::ValidateRestore(SourceLineModel& model)
{
    if (model.paramline != 0)  // optional param
        m_source->CheckLineNumber(model, model.paramline);
}


//////////////////////////////////////////////////////////////////////
