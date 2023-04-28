
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
    { KeywordSCREEN,    &Generator::GenerateScreen },
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
    m_intermed->intermeds.push_back("\t.MCALL\t.EXIT");
    m_intermed->intermeds.push_back("START:");
}

void Generator::ProcessEnd()
{
    m_intermed->intermeds.push_back("L" + std::to_string(MAX_LINE_NUMBER + 1) + ":");
    m_intermed->intermeds.push_back("\t.EXIT");  // In case we run after last line

    m_intermed->intermeds.push_back("; VARIABLES");
    for (size_t i = 0; i < m_source->vars.size(); i++)
    {
        VariableModel& var = m_source->vars[i];
        string deconame = DecorateVariableName(var.name);
        //TODO: Calculate number of array elements multiplying all indices
        ValueType vtype = var.GetValueType();
        switch (vtype)
        {
        case ValueTypeInteger:
            m_intermed->intermeds.push_back(deconame + ":\t.WORD\t0\t; " + var.name);
            break;
        case ValueTypeString:
            m_intermed->intermeds.push_back(deconame + ":\t.BLKB\t256.\t; " + var.name);
            break;
        default:  // Single
            m_intermed->intermeds.push_back(deconame + ":\t.WORD\t0,0\t; " + var.name);
            break;
        }
    }

    //m_intermed->intermeds.push_back("; STRINGS");

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

void Generator::GenerateExpression(ExpressionModel& expr)
{
    assert(!expr.IsEmpty());
    
    ExpressionNode& root = expr.nodes[expr.root];
    if (root.left != -1 || root.right != -1)
    {
        m_intermed->intermeds.push_back("; TODO calculate complex expression");
        return;
    }

    if (root.vtype != ValueTypeInteger && (root.vtype != ValueTypeSingle || !root.node.IsDValueInteger()))
    {
        m_intermed->intermeds.push_back("; TODO calculate non-integer expression");
        return;
    }

    if (root.node.type == TokenTypeNumber)
        m_intermed->intermeds.push_back("\tMOV\t#" + std::to_string((int)root.node.dvalue) + "., R0");
    else if (root.node.type == TokenTypeIdentifier)
    {
        string deconame = DecorateVariableName(GetCanonicVariableName(root.node.text));
        m_intermed->intermeds.push_back("\tMOV\t" + deconame + "., R0");
    }
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
    int nextlinenum = m_source->GetNextLineNumber(line.number);
    if (nextlinenum != MAX_LINE_NUMBER + 1)
        m_intermed->intermeds.push_back("\tJMP\tL" + std::to_string(MAX_LINE_NUMBER + 1));
}

void Generator::GenerateFor(SourceLineModel& line)
{
    // Calculate expression for "from"
    assert(line.args.size() > 1);
    ExpressionModel& expr1 = line.args[0];
    GenerateExpression(expr1);

    // Assign the expression to the loop variable
    assert(line.ident.type == TokenTypeIdentifier);
    string varname = line.ident.text;
    string deconame = DecorateVariableName(GetCanonicVariableName(varname));
    m_intermed->intermeds.push_back("\tMOV\tR0, " + deconame);

    // Calculate expression for "to"
    ExpressionModel& expr2 = line.args[1];
    GenerateExpression(expr2);
    // Save "to" value
    m_intermed->intermeds.push_back("\tMOV\tR0, @#<N" + std::to_string(line.number) + "+2>");

    if (line.args.size() > 2)  // has STEP expression
    {
        // Calculate expression for "step"
        ExpressionModel& expr3 = line.args[2];
        GenerateExpression(expr3);
        // Save "step" value
        m_intermed->intermeds.push_back("\tMOV\tR0, @#<L" + std::to_string(line.paramline) + "+2>");
    }

    int nextlinenum = m_source->GetNextLineNumber(line.number);
    m_intermed->intermeds.push_back("N" + std::to_string(line.number) + ":\tCMP\t#0, " + deconame);
    m_intermed->intermeds.push_back("\tBHIS\tL" + std::to_string(nextlinenum));
    m_intermed->intermeds.push_back("\tJMP\tX" + std::to_string(line.number));  // label after NEXT
}

void Generator::GenerateGosub(SourceLineModel& line)
{
    string calllinenum = "\tCALL\tL" + std::to_string(line.paramline);
    m_intermed->intermeds.push_back(calllinenum);
}

void Generator::GenerateGoto(SourceLineModel& line)
{
    string jmplinenum = "\tJMP\tL" + std::to_string(line.paramline);
    m_intermed->intermeds.push_back(jmplinenum);
}

void Generator::GenerateIf(SourceLineModel& line)
{
    assert(line.args.size() > 0);
    ExpressionModel& expr = line.args[0];
    GenerateExpression(expr);
    //TODO: set flags: Z=0 for TRUE, Z=1 for FALSE

    if (line.params.size() == 1)  // IF expr THEN linenum
    {
        int linenumnext = m_source->GetNextLineNumber(line.number);
        m_intermed->intermeds.push_back("\tBEQ\tL" + std::to_string(linenumnext));
        int linenum = (int)line.params[0].dvalue;
        m_intermed->intermeds.push_back("\tJMP\tL" + std::to_string(linenum));
    }
    else  // IF expr THEN linenum ELSE linenum
    {
        m_intermed->intermeds.push_back("\tBEQ\t10$");
        int linenum1 = (int)line.params[0].dvalue;
        m_intermed->intermeds.push_back("\tJMP\tL" + std::to_string(linenum1));
        int linenum2 = (int)line.params[1].dvalue;
        m_intermed->intermeds.push_back("10$:\tJMP\tL" + std::to_string(linenum2));
    }
}

void Generator::GenerateLet(SourceLineModel& line)
{
    ExpressionModel& expr = line.args[0];
    GenerateExpression(expr);

    VariableModel& var = line.variables[0];
    string deconame = DecorateVariableName(GetCanonicVariableName(var.name));
    m_intermed->intermeds.push_back("\tMOV\tR0, " + deconame);
}

void Generator::GenerateOn(SourceLineModel& line)
{
    ExpressionModel& expr = line.args[0];
    GenerateExpression(expr);
    int numofcases = line.params.size();
    string nextline = "L" + std::to_string(m_source->GetNextLineNumber(line.number));
    m_intermed->intermeds.push_back("\tDEC\tR0");
    m_intermed->intermeds.push_back("\tBLO\t" + nextline);
    m_intermed->intermeds.push_back("\tCMP\t#" + std::to_string(numofcases) + ", R0");
    m_intermed->intermeds.push_back("\tBGE\t" + nextline);
    m_intermed->intermeds.push_back("\tASL\tR0");
    m_intermed->intermeds.push_back("\tJMP\t@10$(R0)");
    int linenum = (int)line.params[0].dvalue;
    m_intermed->intermeds.push_back("10$:\t.WORD\tL" + std::to_string(linenum));
    for (size_t i = 1; i < line.params.size(); i++)
    {
        linenum = (int)line.params[i].dvalue;
        m_intermed->intermeds.push_back("\t.WORD\tL" + std::to_string(linenum));
    }
}

void Generator::GenerateLocate(SourceLineModel& line)
{
    //TODO
    m_intermed->intermeds.push_back("; TODO LOCATE");
}

void Generator::GenerateNext(SourceLineModel& line)
{
    string varname = "A%";//STUB
    string deconame = DecorateVariableName(GetCanonicVariableName(varname));
    int forlinenum = line.paramline;

    SourceLineModel& linefor = m_source->GetSourceLine(line.paramline);

    if (linefor.args.size() < 3)
        m_intermed->intermeds.push_back("\tINC\t" + deconame);
    else
        m_intermed->intermeds.push_back("\tADD\t#1, " + deconame);

    // JMP to continue loop
    m_intermed->intermeds.push_back("\tJMP\tN" + std::to_string(forlinenum));
    // Label after NEXT
    m_intermed->intermeds.push_back("X" + std::to_string(forlinenum) + ":");
}

void Generator::GeneratePrint(SourceLineModel& line)
{
    for (size_t i = 0; i < line.args.size(); i++)
    {
        ExpressionModel& expr = line.args[i];
        ExpressionNode& root = expr.nodes[expr.root];
        if (root.vtype == ValueTypeString)
        {
            //TODO
        }
        else if (root.vtype == ValueTypeInteger)
        {
            GenerateExpression(expr);
            m_intermed->intermeds.push_back("\tCALL\tWRINT");
        }
        else if (root.vtype == ValueTypeSingle)
        {
            GenerateExpression(expr);
            m_intermed->intermeds.push_back("\tCALL\tWRSNG");
        }
        //TODO: AT/TAB/SPC
    }
 
    // CR/LF at end of PRINT
    m_intermed->intermeds.push_back("\tCALL\tWRCRLF");
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

void Generator::GenerateScreen(SourceLineModel& line)
{
    m_intermed->intermeds.push_back("; SCREEN statement is ignored");
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
