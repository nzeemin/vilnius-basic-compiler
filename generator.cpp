
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
    { KeywordPOKE,      &Generator::GeneratePoke },
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


Generator::Generator(SourceModel* source, FinalModel* intermed)
    : m_source(source), m_final(intermed)
{
    assert(source != nullptr);
    assert(intermed != nullptr);

    m_lineindex = -1;
}

void Generator::ProcessBegin()
{
    //TODO: TITLE
    m_final->AddLine("\t.MCALL\t.EXIT");
    m_final->AddLine("START:");
}

void Generator::ProcessEnd()
{
    m_final->AddLine("L" + std::to_string(MAX_LINE_NUMBER + 1) + ":");
    m_final->AddLine("\t.EXIT");  // In case we run after last line

    m_final->AddLine("; VARIABLES");
    for (size_t i = 0; i < m_source->vars.size(); i++)
    {
        VariableModel& var = m_source->vars[i];
        string deconame = DecorateVariableName(var.name);
        //TODO: Calculate number of array elements multiplying all indices
        ValueType vtype = var.GetValueType();
        switch (vtype)
        {
        case ValueTypeInteger:
            m_final->AddLine(deconame + ":\t.WORD\t0\t; " + var.name);
            break;
        case ValueTypeString:
            m_final->AddLine(deconame + ":\t.BLKB\t256.\t; " + var.name);
            break;
        default:  // Single
            m_final->AddLine(deconame + ":\t.WORD\t0,0\t; " + var.name);
            break;
        }
    }

    //m_intermed->intermeds.push_back("; STRINGS");

    m_final->AddLine("\t.END\tSTART");
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
    m_final->AddLine("; " + line.text);
    string linenumlabel = "L" + std::to_string(line.number) + ":";//TODO function GetLineNumberLabel
    m_final->AddLine(linenumlabel);

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
        m_final->AddLine("; TODO generate complex expression");
        return;
    }

    if (root.vtype != ValueTypeInteger && (root.vtype != ValueTypeSingle || !root.node.IsDValueInteger()))
    {
        m_final->AddLine("; TODO calculate non-integer expression");
        return;
    }

    if (root.node.type == TokenTypeNumber)
        m_final->AddLine("\tMOV\t#" + std::to_string((int)root.node.dvalue) + "., R0");
    else if (root.node.type == TokenTypeIdentifier)
    {
        string deconame = DecorateVariableName(GetCanonicVariableName(root.node.text));
        m_final->AddLine("\tMOV\t" + deconame + "., R0");
    }
}

void Generator::GenerateBeep(SourceLineModel& line)
{
    m_final->AddLine("\tCALL\tBEEP");
}

void Generator::GenerateClear(SourceLineModel& line)
{
    m_final->AddLine("; CLEAR statement is ignored");
}

void Generator::GenerateCls(SourceLineModel& line)
{
    m_final->AddLine("\tCALL\tCLS");
}

void Generator::GenerateColor(SourceLineModel& line)
{
    //TODO
    m_final->AddLine("; TODO COLOR");
}

void Generator::GenerateData(SourceLineModel& line)
{
    //TODO
    m_final->AddLine("; TODO DATA");
}

void Generator::GenerateDim(SourceLineModel& line)
{
    // Nothing to generate, DIM variables declared in ProcessEnd()
}

void Generator::GenerateDraw(SourceLineModel& line)
{
    //TODO
    m_final->AddLine("; TODO DRAW");
}

void Generator::GenerateEnd(SourceLineModel& line)
{
    // END generates JMP 65536, but only if END is not on the last line
    int nextlinenum = m_source->GetNextLineNumber(line.number);
    if (nextlinenum != MAX_LINE_NUMBER + 1)
        m_final->AddLine("\tJMP\tL" + std::to_string(MAX_LINE_NUMBER + 1));
}

void Generator::GenerateFor(SourceLineModel& line)
{
    // Calculate expression for "from"
    assert(line.args.size() > 1);
    ExpressionModel& expr1 = line.args[0];
    //TODO: Simplified version for const/var expression
    GenerateExpression(expr1);

    // Assign the expression to the loop variable
    assert(line.ident.type == TokenTypeIdentifier);
    string varname = line.ident.text;
    string deconame = DecorateVariableName(GetCanonicVariableName(varname));
    m_final->AddLine("\tMOV\tR0, " + deconame);

    // Calculate expression for "to"
    ExpressionModel& expr2 = line.args[1];
    //TODO: Simplified version for const/var expression
    GenerateExpression(expr2);
    // Save "to" value
    m_final->AddLine("\tMOV\tR0, @#<N" + std::to_string(line.number) + "+2>");

    if (line.args.size() > 2)  // has STEP expression
    {
        // Calculate expression for "step"
        ExpressionModel& expr3 = line.args[2];
        GenerateExpression(expr3);
        // Save "step" value
        m_final->AddLine("\tMOV\tR0, @#<L" + std::to_string(line.paramline) + "+2>");
    }

    int nextlinenum = m_source->GetNextLineNumber(line.number);
    m_final->AddLine("N" + std::to_string(line.number) + ":\tCMP\t#0, " + deconame);
    m_final->AddLine("\tBHIS\tL" + std::to_string(nextlinenum));
    m_final->AddLine("\tJMP\tX" + std::to_string(line.number));  // label after NEXT
}

void Generator::GenerateGosub(SourceLineModel& line)
{
    string calllinenum = "\tCALL\tL" + std::to_string(line.paramline);
    m_final->AddLine(calllinenum);
}

void Generator::GenerateGoto(SourceLineModel& line)
{
    string jmplinenum = "\tJMP\tL" + std::to_string(line.paramline);
    m_final->AddLine(jmplinenum);
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
        m_final->AddLine("\tBEQ\tL" + std::to_string(linenumnext));
        int linenum = (int)line.params[0].dvalue;
        m_final->AddLine("\tJMP\tL" + std::to_string(linenum));
    }
    else  // IF expr THEN linenum ELSE linenum
    {
        m_final->AddLine("\tBEQ\t10$");
        int linenum1 = (int)line.params[0].dvalue;
        m_final->AddLine("\tJMP\tL" + std::to_string(linenum1));
        int linenum2 = (int)line.params[1].dvalue;
        m_final->AddLine("10$:\tJMP\tL" + std::to_string(linenum2));
    }
}

void Generator::GenerateLet(SourceLineModel& line)
{
    assert(line.args.size() == 1);
    ExpressionModel& expr = line.args[0];
    //TODO: Simplified version for const/var expression
    GenerateExpression(expr);

    VariableModel& var = line.variables[0];
    string deconame = DecorateVariableName(GetCanonicVariableName(var.name));
    m_final->AddLine("\tMOV\tR0, " + deconame);
}

void Generator::GenerateOn(SourceLineModel& line)
{
    ExpressionModel& expr = line.args[0];
    GenerateExpression(expr);
    int numofcases = line.params.size();
    string nextline = "L" + std::to_string(m_source->GetNextLineNumber(line.number));
    m_final->AddLine("\tDEC\tR0");
    m_final->AddLine("\tBLO\t" + nextline);
    m_final->AddLine("\tCMP\t#" + std::to_string(numofcases) + ", R0");
    m_final->AddLine("\tBGE\t" + nextline);
    m_final->AddLine("\tASL\tR0");
    m_final->AddLine("\tJMP\t@10$(R0)");
    int linenum = (int)line.params[0].dvalue;
    m_final->AddLine("10$:\t.WORD\tL" + std::to_string(linenum));
    for (size_t i = 1; i < line.params.size(); i++)
    {
        linenum = (int)line.params[i].dvalue;
        m_final->AddLine("\t.WORD\tL" + std::to_string(linenum));
    }
}

void Generator::GenerateLocate(SourceLineModel& line)
{
    //TODO
    m_final->AddLine("; TODO LOCATE");
}

void Generator::GenerateNext(SourceLineModel& line)
{
    assert(line.paramline != 0);
    SourceLineModel& linefor = m_source->GetSourceLine(line.paramline);

    string varname = linefor.ident.text;
    string deconame = DecorateVariableName(GetCanonicVariableName(varname));
    int forlinenum = line.paramline;

    if (linefor.args.size() < 3)
        m_final->AddLine("\tINC\t" + deconame);
    else
        m_final->AddLine("\tADD\t#1, " + deconame);

    // JMP to continue loop
    m_final->AddLine("\tJMP\tN" + std::to_string(forlinenum));
    // Label after NEXT
    m_final->AddLine("X" + std::to_string(forlinenum) + ":");
}

void Generator::GeneratePoke(SourceLineModel& line)
{
    assert(line.args.size() == 2);

    ExpressionModel& expr1 = line.args[0];
    //TODO: Simplified version for const/var expression
    GenerateExpression(expr1);
    // Save value
    m_final->AddLine("\tMOV\tR0, @#<10$+2>");

    ExpressionModel& expr2 = line.args[1];
    //TODO: Simplified version for const/var expression
    GenerateExpression(expr2);
    // Save value
    m_final->AddLine("\tMOV\tR0, @#<10$+4>");

    m_final->AddLine("10$:\tMOV\t#0, #0");
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
            m_final->AddLine("\tCALL\tWRINT");
        }
        else if (root.vtype == ValueTypeSingle)
        {
            GenerateExpression(expr);
            m_final->AddLine("\tCALL\tWRSNG");
        }
        //TODO: AT/TAB/SPC
    }
 
    // CR/LF at end of PRINT
    m_final->AddLine("\tCALL\tWRCRLF");
}

void Generator::GenerateRead(SourceLineModel& line)
{
    //TODO
    m_final->AddLine("; TODO READ");
}

void Generator::GenerateRem(SourceLineModel& line)
{
    // Do nothing
}

void Generator::GenerateRestore(SourceLineModel& line)
{
    //TODO
    m_final->AddLine("; TODO RESTORE");
}

void Generator::GenerateReturn(SourceLineModel& line)
{
    m_final->AddLine("\tRETURN");
}

void Generator::GenerateScreen(SourceLineModel& line)
{
    m_final->AddLine("; SCREEN statement is ignored");
}

void Generator::GenerateStop(SourceLineModel& line)
{
    m_final->AddLine("\tHALT");
}

void Generator::GenerateTron(SourceLineModel& line)
{
    m_final->AddLine("; TRON statement is ignored");
}

void Generator::GenerateTroff(SourceLineModel& line)
{
    m_final->AddLine("; TROFF statement is ignored");
}

void Generator::GenerateWidth(SourceLineModel& line)
{
    m_final->AddLine("; WIDTH statement is ignored");
}


//////////////////////////////////////////////////////////////////////
