
#include <cassert>
#include <string>

#include "main.h"


//////////////////////////////////////////////////////////////////////


const GeneratorKeywordSpec Generator::m_keywordspecs[] =
{
    { KeywordBEEP,	    &Generator::GenerateBeep },
    { KeywordCLS,	    &Generator::GenerateCls },
    { KeywordEND,	    &Generator::GenerateEnd },
    { KeywordFOR,	    &Generator::GenerateFor },
    { KeywordGOSUB,	    &Generator::GenerateGosub },
    { KeywordGOTO,	    &Generator::GenerateGoto },
    { KeywordIF,        &Generator::GenerateIf },
    { KeywordLET,	    &Generator::GenerateLet },
    { KeywordNEXT,	    &Generator::GenerateNext },
    { KeywordON,        &Generator::GenerateOn },
    { KeywordPRINT,	    &Generator::GeneratePrint },
    { KeywordREM,       &Generator::GenerateRem },
    { KeywordRETURN,    &Generator::GenerateReturn },
    { KeywordSTOP,      &Generator::GenerateStop },
    { KeywordTRON,      &Generator::GenerateTron },
    { KeywordTROFF,     &Generator::GenerateTroff },
};


Generator::Generator(SourceModel* source, IntermedModel* intermed)
    : m_source(source), m_intermed(intermed)
{
    assert(source != nullptr);
    assert(intermed != nullptr);

    m_lineindex = -1;
}

void Generator::ProcessBegin()
{
    //TODO: TITLE
    //TODO: MCALL
    m_intermed->intermeds.push_back("START:");
}

void Generator::ProcessEnd()
{
    m_intermed->intermeds.push_back("\t.END\tSTART");
}

bool Generator::ProcessLine()
{
    if (m_lineindex > MAX_LINE_NUMBER)
        return false;

    if (m_lineindex < 0)
    {
        ProcessBegin();
        m_lineindex = 0;
    }
    else
        m_lineindex++;
    
    if (m_lineindex >= (int)m_source->lines.size())
    {
        ProcessEnd();
        m_lineindex = INT_MAX;
        return false;
    }

    //TODO: Show full text of the source line in comment

    SourceLineModel& line = m_source->lines[m_lineindex];
    string linenumlabel = "L" + std::to_string(line.number) + ":";
    m_intermed->intermeds.push_back(linenumlabel);

    // Find keyword generator implementation
    KeywordIndex keyword = line.statement.keyword;
    GeneratorMethodRef methodref = nullptr;
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

void Generator::GenerateBeep(SourceLineModel& line)
{
    //TODO: Send code 007 to terminal
    m_intermed->intermeds.push_back("TODO BEEP");
}

void Generator::GenerateCls(SourceLineModel& line)
{
    //TODO: Send code Ctrl+L to terminal
    m_intermed->intermeds.push_back("TODO CLS");
}

void Generator::GenerateEnd(SourceLineModel& line)
{
    m_intermed->intermeds.push_back("\t.EXIT");
}

void Generator::GenerateFor(SourceLineModel& line)
{
    //TODO
    m_intermed->intermeds.push_back("TODO FOR");
}

void Generator::GenerateGosub(SourceLineModel& line)
{
    //TODO: Check if we have this line number

    string calllinenum = "\tCALL\tL" + std::to_string(line.paramline);
    m_intermed->intermeds.push_back(calllinenum);
}

void Generator::GenerateGoto(SourceLineModel& line)
{
    //TODO: Check if we have this line number

    string jmplinenum = "\tJMP\tL" + std::to_string(line.paramline);
    m_intermed->intermeds.push_back(jmplinenum);
}

void Generator::GenerateIf(SourceLineModel& line)
{
    //TODO
    m_intermed->intermeds.push_back("TODO IF");
}

void Generator::GenerateLet(SourceLineModel& line)
{
    //TODO
    m_intermed->intermeds.push_back("TODO LET");
}

void Generator::GenerateOn(SourceLineModel& line)
{
    //TODO
    m_intermed->intermeds.push_back("TODO ON");
}

void Generator::GenerateNext(SourceLineModel& line)
{
    //TODO
    m_intermed->intermeds.push_back("TODO NEXT");
}

void Generator::GeneratePrint(SourceLineModel& line)
{
    //TODO
    m_intermed->intermeds.push_back("TODO PRINT");
}

void Generator::GenerateRem(SourceLineModel& line)
{
    // Do nothing
}

void Generator::GenerateReturn(SourceLineModel& line)
{
    m_intermed->intermeds.push_back("\tRETURN");
}

void Generator::GenerateStop(SourceLineModel& line)
{
    //TODO
    m_intermed->intermeds.push_back("TODO STOP");
}

void Generator::GenerateTron(SourceLineModel& line)
{
    // Do nothing
}

void Generator::GenerateTroff(SourceLineModel& line)
{
    // Do nothing
}


//////////////////////////////////////////////////////////////////////
