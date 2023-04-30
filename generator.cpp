
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

const GeneratorOperSpec Generator::m_operspecs[] =
{
    { "+",              &Generator::GenerateOperPlus },
    { "-",              &Generator::GenerateOperMinus },
    //{ "*",              &Generator::GenerateOperMul },
    //{ "/",              &Generator::GenerateOperDiv },
    //{ "\\",             &Generator::GenerateOperDivInt },
    //{ "^",              &Generator::GenerateOperPower },
};

const GeneratorFuncSpec Generator::m_funcspecs[] =
{
    { KeywordABS,       &Generator::GenerateFuncAbs },
    { KeywordRND,       &Generator::GenerateFuncRnd },
    { KeywordPEEK,      &Generator::GenerateFuncPeek },
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

void Generator::GenerateExpression(const ExpressionModel& expr)
{
    assert(!expr.IsEmpty());

    const ExpressionNode& root = expr.nodes[expr.root];

    GenerateExpression(expr, root);
}

// Generate code to calculate the expression; result will be in register R0
void Generator::GenerateExpression(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(!expr.IsEmpty());

    if (node.vtype != ValueTypeInteger && node.vtype != ValueTypeSingle)
    {
        m_final->AddLine("; TODO calculate non-integer expression");
        return;
    }

    if (node.constval)
    {
        int ivalue = (int)floor(node.node.dvalue);
        if (ivalue == 0)
        {
            m_final->AddLine("\tCLR\tR0");
        }
        else
        {
            string svalue = "#" + std::to_string(ivalue) + ".";
            m_final->AddLine("\tMOV\t" + svalue + ", R0");
        }
    }

    if (node.node.type == TokenTypeKeyword && IsFunctionKeyword(node.node.keyword))
    {
        GenerateExprFunction(expr, node);
        return;
    }

    if (node.node.type == TokenTypeIdentifier)
    {
        string deconame = DecorateVariableName(GetCanonicVariableName(node.node.text));
        m_final->AddLine("\tMOV\t" + deconame + "., R0");
        return;
    }

    if (node.node.type == TokenTypeOperation && node.left >= 0 && node.right >= 0)
    {
        const ExpressionNode& nodeleft = expr.nodes[node.left];
        const ExpressionNode& noderight = expr.nodes[node.right];

        if (nodeleft.vtype == ValueTypeNone || noderight.vtype == ValueTypeNone)
        {
            std::cerr << "ERROR in expression at " << node.node.line << ":" << node.node.pos << " - Cannot calculate value type for the node." << std::endl;
            exit(EXIT_FAILURE);
        }

        // Find operator implementation
        string text = node.node.text;
        GeneratorOperMethodRef methodref = nullptr;
        for (int i = 0; i < sizeof(m_operspecs) / sizeof(GeneratorFuncSpec); i++)
        {
            if (text == m_operspecs[i].text)
            {
                methodref = m_operspecs[i].methodref;
                break;
            }
        }

        if (methodref != nullptr)
            (this->*methodref)(expr, node, nodeleft, noderight);
        else
            std::cerr << "ERROR in expression at " << node.node.line << ":" << node.node.pos << " - TODO generate operator \'" + text + "\'." << std::endl;

        return;
    }

    if (node.left != -1 || node.right != -1)
    {
        m_final->AddLine("; TODO generate complex expression");
        return;
    }
}

void Generator::GenerateExprFunction(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(!node.constval);
    assert(node.node.keyword != KeywordNone);
    assert(node.node.type == TokenTypeKeyword && IsFunctionKeyword(node.node.keyword));

    KeywordIndex keyword = node.node.keyword;

    GeneratorFuncMethodRef methodref = nullptr;
    for (int i = 0; i < sizeof(m_funcspecs) / sizeof(GeneratorFuncSpec); i++)
    {
        if (keyword == m_funcspecs[i].keyword)
        {
            methodref = m_funcspecs[i].methodref;
            break;
        }
    }

    if (methodref != nullptr)
        (this->*methodref)(expr, node);
    else
        m_final->AddLine("; TODO generate function expression for " + GetKeywordString(keyword));
}

// Calculate expression and assign the result to variable
// To use in LET and FOR
void Generator::GenerateAssignment(SourceLineModel& line, VariableModel& var, ExpressionModel& expr)
{
    const string comment = "\t; assignment";

    ValueType vtype = var.GetValueType();
    string deconame = var.GetVariableDecoratedName();

    if (expr.IsConstExpression())
    {
        int ivalue = (int)floor(expr.GetConstExpressionDValue());
        if (ivalue == 0)
        {
            m_final->AddLine("\tCLR\t" + deconame + comment);
        }
        else {
            string svalue = "#" + std::to_string(ivalue) + ".";
            m_final->AddLine("\tMOV\t" + svalue + ", " + deconame + comment);
        }
        //TODO: version for Single type
    }
    else if (expr.IsVariableExpression())
    {
        string svalue = expr.GetVariableExpressionDecoratedName();
        m_final->AddLine("\tMOV\t" + svalue + ", " + deconame + comment);
    }
    else
    {
        ExpressionNode& root = expr.nodes[expr.root];

        // Convert "A% = A% + N" and "A% = A% - N" assignments into INC/DEC/ADD/SUB
        if (vtype == ValueTypeInteger && root.node.IsBinaryOperation() &&
            (root.node.text == "-" || root.node.text == "+") &&
            expr.nodes[root.left].node.type == TokenTypeIdentifier &&
            GetCanonicVariableName(expr.nodes[root.left].node.text) == var.name &&
            expr.nodes[root.right].constval &&
            (expr.nodes[root.right].vtype == ValueTypeInteger || expr.nodes[root.right].vtype == ValueTypeSingle))
        {
            bool plusminus = (root.node.text == "+");
            int ivalue = (int)floor(expr.nodes[root.right].node.dvalue);
            if (plusminus && ivalue == 1)
                m_final->AddLine("\tINC\t" + deconame + comment);
            else if (!plusminus && ivalue == 1)
                m_final->AddLine("\tDEC\t" + deconame + comment);
            else if (plusminus && ivalue != 1)
                m_final->AddLine("\tADD\t#" + std::to_string(ivalue) + "., " + deconame + comment);
            else //if (!plusminus && ivalue != 1)
                m_final->AddLine("\tSUB\t#" + std::to_string(ivalue) + "., " + deconame + comment);
        }
        else
        {
            GenerateExpression(expr);
            m_final->AddLine("\tMOV\tR0, " + deconame + comment);
        }
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

    assert(line.ident.type == TokenTypeIdentifier);
    VariableModel var;
    var.name = line.ident.text;
    string deconame = var.GetVariableDecoratedName();

    // Assign the expression to the loop variable
    GenerateAssignment(line, var, expr1);

    // Calculate expression for "to"
    string tovalue = "#0";
    ExpressionModel& expr2 = line.args[1];
    if (expr2.IsConstExpression())
    {
        tovalue = "#" + std::to_string((int)floor(expr2.GetConstExpressionDValue())) + ".";
    }
    else if (expr2.IsVariableExpression())
    {
        string svalue = expr2.GetVariableExpressionDecoratedName();
        m_final->AddLine("\tMOV\t" + svalue + ", @#<N" + std::to_string(line.number) + "+2>");
    }
    else
    {
        GenerateExpression(expr2);
        m_final->AddLine("\tMOV\tR0, @#<N" + std::to_string(line.number) + "+2>");  //  Save "to" value
    }

    if (line.args.size() > 2)  // has STEP expression
    {
        // Calculate expression for "step"
        ExpressionModel& expr3 = line.args[2];
        GenerateExpression(expr3);
        // Save "step" value
        m_final->AddLine("\tMOV\tR0, @#<L" + std::to_string(line.paramline) + "+2>");
    }

    int nextlinenum = m_source->GetNextLineNumber(line.number);
    m_final->AddLine("N" + std::to_string(line.number) + ":\tCMP\t" + tovalue + ", " + deconame);
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

    VariableModel& var = line.variables[0];

    GenerateAssignment(line, var, expr);
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


// Operation generation //////////////////////////////////////////////

void Generator::GenerateOperPlus(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    const string comment = "\t; Operation \'+\'";

    // Code to calculate left sub-expression, with result in R0
    GenerateExpression(expr, nodeleft);

    // Convert "XXX + N" into INC/ADD
    if (nodeleft.vtype == ValueTypeInteger &&
        noderight.constval && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle))
    {
        int ivalue = (int)floor(noderight.node.dvalue);
        if (ivalue == 0)
            ;  // Do nothing
        else if (ivalue == 1)
            m_final->AddLine("\tINC\tR0" + comment);
        else  // ivalue != 1
            m_final->AddLine("\tADD\t#" + std::to_string(ivalue) + "., R0" + comment);
        return;
    }

    // Special case for noderight as variable
    if (nodeleft.vtype == ValueTypeInteger && noderight.vtype == ValueTypeInteger && noderight.node.type == TokenTypeIdentifier)
    {
        string deconame = DecorateVariableName(GetCanonicVariableName( noderight.node.text));
        m_final->AddLine("\tADD\t" + deconame + ", R0" + comment);
        return;
    }

    m_final->AddLine("\tMOV\tR0, -(SP)");  // PUSH R0
    GenerateExpression(expr, noderight);
    m_final->AddLine("\tADD\t(SP)+, R0" + comment);  // POP & ADD
}

void Generator::GenerateOperMinus(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    const string comment = "\t; Operation \'-\'";

    // Code to calculate left sub-expression, with result in R0
    GenerateExpression(expr, nodeleft);

    // Convert "XXX - N" into DEC/SUB
    if (nodeleft.vtype == ValueTypeInteger &&
        noderight.constval && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle))
    {
        int ivalue = (int)floor(noderight.node.dvalue);
        if (ivalue == 0)
            ;  // Do nothing
        else if (ivalue == 1)
            m_final->AddLine("\tDEC\tR0" + comment);
        else  // ivalue != 1
            m_final->AddLine("\tSUB\t#" + std::to_string(ivalue) + "., R0" + comment);
        return;
    }

    // Special case for noderight as variable
    if (nodeleft.vtype == ValueTypeInteger && noderight.vtype == ValueTypeInteger && noderight.node.type == TokenTypeIdentifier)
    {
        string deconame = DecorateVariableName(GetCanonicVariableName(noderight.node.text));
        m_final->AddLine("\tSUB\t" + deconame + ", R0" + comment);
        return;
    }

    m_final->AddLine("\tMOV\tR0, -(SP)");  // PUSH R0
    GenerateExpression(expr, noderight);
    m_final->AddLine("\tMOV\tR0, R1");
    m_final->AddLine("\tMOV\t(SP)+, R0");  // POP R0
    m_final->AddLine("\tSUB\tR1, R0" + comment);
}


// Function generation ///////////////////////////////////////////////

void Generator::GenerateFuncAbs(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(node.args.size() == 1);

    const ExpressionModel& expr1 = node.args[0];
    GenerateExpression(expr1);

    m_final->AddLine("\tBPL\t1$");
    m_final->AddLine("\tNEG\tR0");
    m_final->AddLine("1$:");
}

void Generator::GenerateFuncRnd(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(node.args.size() == 1);

    const ExpressionModel& expr1 = node.args[0];
    GenerateExpression(expr1);
    //TODO: For Single expression, convert to Integer

    m_final->AddLine("\tCALL\tRND");
}

void Generator::GenerateFuncPeek(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(node.args.size() == 1);

    const ExpressionModel& expr1 = node.args[0];
    GenerateExpression(expr1);
    //TODO: For Single expression, convert to Integer

    m_final->AddLine("\tMOV\t(R0), R0\t; PEEK");
}


//////////////////////////////////////////////////////////////////////
