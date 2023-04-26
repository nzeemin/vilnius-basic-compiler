
#include <cassert>
#include <string>

#include "main.h"


//////////////////////////////////////////////////////////////////////


const GeneratorKeywordSpec Generator::m_keywordspecs[] =
{
    { KeywordBEEP,      &Generator::GenerateBeep },
    { KeywordCLEAR,     &Generator::GenerateClear },
    { KeywordCLS,       &Generator::GenerateCls },
    { KeywordCOLOR,     &Generator::GenerateColor },
    { KeywordDATA,      &Generator::GenerateData },
    { KeywordDIM,       &Generator::GenerateDim },
    { KeywordDRAW,      &Generator::GenerateDraw },
    { KeywordEND,       &Generator::GenerateEnd },
    { KeywordFOR,       &Generator::GenerateFor },
    { KeywordGOSUB,     &Generator::GenerateGosub },
    { KeywordGOTO,      &Generator::GenerateGoto },
    { KeywordIF,        &Generator::GenerateIf },
    { KeywordLET,       &Generator::GenerateLet },
    { KeywordLOCATE,    &Generator::GenerateLocate },
    { KeywordNEXT,      &Generator::GenerateNext },
    { KeywordON,        &Generator::GenerateOn },
    { KeywordPRINT,     &Generator::GeneratePrint },
    { KeywordREAD,      &Generator::GenerateRead },
    { KeywordREM,       &Generator::GenerateRem },
    { KeywordRESTORE,   &Generator::GenerateRestore },
    { KeywordRETURN,    &Generator::GenerateReturn },
    { KeywordSTOP,      &Generator::GenerateStop },
    { KeywordTRON,      &Generator::GenerateTron },
    { KeywordTROFF,     &Generator::GenerateTroff },
    { KeywordWIDTH,     &Generator::GenerateWidth },
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
    if (m_lineindex == INT_MAX)
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

    SourceLineModel& line = m_source->lines[m_lineindex];
    m_intermed->intermeds.push_back("; " + line.text);
    string linenumlabel = "L" + std::to_string(line.number) + ":";//TODO function GetLineNumberLabel
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

    if (methodref == nullptr)
    {
        Error(line, "Generator for keyword " + GetKeywordString(keyword) + " not found.");
        return true;
    }

    (this->*methodref)(line);

    return true;
}

void Generator::Error(SourceLineModel& line, string message)
{
    std::cerr << "ERROR at line " << line.number << " - " << message << std::endl;
    line.error = true;
    RegisterError();
}

void Generator::GenerateBeep(SourceLineModel& line)
{
    //TODO: Send code 007 to terminal
    m_intermed->intermeds.push_back("\tCALL\tBEEP");
}

void Generator::GenerateClear(SourceLineModel& line)
{
    m_intermed->intermeds.push_back("; CLEAR statement is ignored");
}

void Generator::GenerateCls(SourceLineModel& line)
{
    //TODO: Send code Ctrl+L to terminal
    m_intermed->intermeds.push_back("\tCALL\tCLS");
}

void Generator::GenerateColor(SourceLineModel& line)
{
    //TODO
    m_intermed->intermeds.push_back("; TODO COLOR");
}

void Generator::GenerateData(SourceLineModel& line)
{
    //TODO
    m_intermed->intermeds.push_back("; TODO DATA");
}

void Generator::GenerateDim(SourceLineModel& line)
{
    // Nothing to generate, DIM variables declared in ProcessBegin() / ProcessEnd()
}

void Generator::GenerateDraw(SourceLineModel& line)
{
    //TODO
    m_intermed->intermeds.push_back("; TODO DRAW");
}

void Generator::GenerateEnd(SourceLineModel& line)
{
    m_intermed->intermeds.push_back("\t.EXIT");
}

void Generator::GenerateFor(SourceLineModel& line)
{
    //TODO
    m_intermed->intermeds.push_back("; TODO FOR");
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
    m_intermed->intermeds.push_back("; TODO IF");
}

void Generator::GenerateLet(SourceLineModel& line)
{
    //TODO
    m_intermed->intermeds.push_back("; TODO LET");
}

void Generator::GenerateOn(SourceLineModel& line)
{
    //TODO
    m_intermed->intermeds.push_back("; TODO ON");
}

void Generator::GenerateLocate(SourceLineModel& line)
{
    //TODO
    m_intermed->intermeds.push_back("; TODO LOCATE");
}

void Generator::GenerateNext(SourceLineModel& line)
{
    //TODO
    m_intermed->intermeds.push_back("; TODO NEXT");
}

void Generator::GeneratePrint(SourceLineModel& line)
{
    //TODO
    m_intermed->intermeds.push_back("; TODO PRINT");
}

void Generator::GenerateRead(SourceLineModel& line)
{
    //TODO
    m_intermed->intermeds.push_back("; TODO READ");
}

void Generator::GenerateRem(SourceLineModel& line)
{
    // Do nothing
}

void Generator::GenerateRestore(SourceLineModel& line)
{
    //TODO
    m_intermed->intermeds.push_back("; TODO RESTORE");
}

void Generator::GenerateReturn(SourceLineModel& line)
{
    m_intermed->intermeds.push_back("\tRETURN");
}

void Generator::GenerateStop(SourceLineModel& line)
{
    m_intermed->intermeds.push_back("\tHALT");
}

void Generator::GenerateTron(SourceLineModel& line)
{
    m_intermed->intermeds.push_back("; TRON statement is ignored");
}

void Generator::GenerateTroff(SourceLineModel& line)
{
    m_intermed->intermeds.push_back("; TROFF statement is ignored");
}

void Generator::GenerateWidth(SourceLineModel& line)
{
    m_intermed->intermeds.push_back("; WIDTH statement is ignored");
}


//////////////////////////////////////////////////////////////////////
