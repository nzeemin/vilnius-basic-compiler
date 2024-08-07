
#include <cassert>
#include <string>
#include <algorithm>
#include <sstream>

#include "main.h"


//////////////////////////////////////////////////////////////////////


const GeneratorKeywordSpec Generator::m_keywordspecs[] =
{
    { KeywordBEEP,      &Generator::GenerateBeep },
    { KeywordBLOAD,     &Generator::GenerateIgnoredStatement },
    { KeywordBSAVE,     &Generator::GenerateIgnoredStatement },
    { KeywordCIRCLE,    &Generator::GenerateCircle },
    { KeywordCLEAR,     &Generator::GenerateClear },
    { KeywordCLOAD,     &Generator::GenerateIgnoredStatement },
    { KeywordCLOSE,     &Generator::GenerateClose },
    { KeywordCLS,       &Generator::GenerateCls },
    { KeywordCOLOR,     &Generator::GenerateColor },
    { KeywordCSAVE,     &Generator::GenerateIgnoredStatement },
    { KeywordDATA,      &Generator::GenerateData },
    { KeywordDIM,       &Generator::GenerateDim },
    { KeywordDRAW,      &Generator::GenerateDraw },
    { KeywordEND,       &Generator::GenerateEnd },
    { KeywordFOR,       &Generator::GenerateFor },
    { KeywordGOSUB,     &Generator::GenerateGosub },
    { KeywordGOTO,      &Generator::GenerateGoto },
    { KeywordIF,        &Generator::GenerateIf },
    { KeywordINPUT,     &Generator::GenerateInput },
    { KeywordKEY,       &Generator::GenerateIgnoredStatement },
    { KeywordLET,       &Generator::GenerateLet },
    { KeywordLINE,      &Generator::GenerateLine },
    { KeywordLOAD,      &Generator::GenerateIgnoredStatement },
    { KeywordLOCATE,    &Generator::GenerateLocate },
    { KeywordNEXT,      &Generator::GenerateNext },
    { KeywordON,        &Generator::GenerateOn },
    { KeywordOPEN,      &Generator::GenerateOpen },
    { KeywordOUT,       &Generator::GenerateOut },
    { KeywordPAINT,     &Generator::GeneratePaint },
    { KeywordPOKE,      &Generator::GeneratePoke },
    { KeywordPRINT,     &Generator::GeneratePrint },
    { KeywordPSET,      &Generator::GeneratePset },
    { KeywordPRESET,    &Generator::GeneratePreset },
    { KeywordREAD,      &Generator::GenerateRead },
    { KeywordREM,       &Generator::GenerateRem },
    { KeywordRESTORE,   &Generator::GenerateRestore },
    { KeywordRETURN,    &Generator::GenerateReturn },
    { KeywordSAVE,      &Generator::GenerateIgnoredStatement },
    { KeywordSCREEN,    &Generator::GenerateScreen },
    { KeywordSTOP,      &Generator::GenerateStop },
    { KeywordTRON,      &Generator::GenerateIgnoredStatement },
    { KeywordTROFF,     &Generator::GenerateIgnoredStatement },
    { KeywordWIDTH,     &Generator::GenerateWidth },
};

const GeneratorOperSpec Generator::m_operspecs[] =
{
    { "+",              &Generator::GenerateOperPlus },
    { "-",              &Generator::GenerateOperMinus },
    { "*",              &Generator::GenerateOperMul },
    { "/",              &Generator::GenerateOperDiv },
    { "\\",             &Generator::GenerateOperDivInt },
    { "MOD",            &Generator::GenerateOperMod },
    { "^",              &Generator::GenerateOperPower },
    { "=",              &Generator::GenerateOperEqual },
    { "<>",             &Generator::GenerateOperNotEqual },
    { "><",             &Generator::GenerateOperNotEqual },
    { "<",              &Generator::GenerateOperLess },
    { ">",              &Generator::GenerateOperGreater },
    { "<=",             &Generator::GenerateOperLessOrEqual },
    { ">=",             &Generator::GenerateOperGreaterOrEqual },
    { "=<",             &Generator::GenerateOperLessOrEqual },
    { "=>",             &Generator::GenerateOperGreaterOrEqual },
    { "AND",            &Generator::GenerateOperAnd },
    { "OR",             &Generator::GenerateOperOr },
    { "XOR",            &Generator::GenerateOperXor },
    //TODO: EQV
    //TODO: IMP
};

const GeneratorFuncSpec Generator::m_funcspecs[] =
{
    { KeywordABS,       &Generator::GenerateFuncAbs },
    { KeywordRND,       &Generator::GenerateFuncRnd },
    { KeywordPEEK,      &Generator::GenerateFuncPeek },
    { KeywordINP,       &Generator::GenerateFuncInp },
    { KeywordLEN,       &Generator::GenerateFuncLen },
    { KeywordINKEY,     &Generator::GenerateFuncInkey },
    { KeywordPOS,       &Generator::GenerateFuncPos },
};


// Comparison function to sort variables by decorated names
bool CompareVariables(const VariableModel& a, const VariableModel& b)
{
    string deconamea = a.GetVariableDecoratedName();
    string deconameb = b.GetVariableDecoratedName();
    return deconamea < deconameb;
}


Generator::Generator(SourceModel* source, FinalModel* final)
    : m_source(source), m_final(final)
{
    assert(source != nullptr);
    assert(final != nullptr);

    m_lineindex = -1;
    m_line = nullptr;
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

    GenerateStrings();

    GenerateVariables();

    m_final->AddLine("\t.END\tSTART");
}

void Generator::GenerateStrings()
{
    if (m_source->conststrings.empty())
        return;

    m_final->AddComment("STRINGS");
    m_final->AddLine("\t.EVEN");
    for (size_t i = 0; i < m_source->conststrings.size(); ++i)
    {
        string strdeco = "ST" + std::to_string(i + 1);
        string& str = m_source->conststrings[i];
        string strlen = std::to_string(str.length());
        if (str.length() > 7) strlen += '.';
        if (str.length() % 2 == 0) str += '\0';  // to align strings to word boundary
        // Mask special symbols, mask '/'
        std::ostringstream oss;
        oss << strdeco << ":\t.ASCII\t<" << strlen << ">";
        bool mode = false;  // false = out of brackets, true = inside brackets
        for (size_t i = 0; i < str.length(); i++)
        {
            char ch = str[i];
            if ((ch >= 0 && ch < 32) || ch == '/')
            {
                if (mode)
                {
                    oss << "/";
                    mode = false;
                }
                oss << "<" << std::oct << (unsigned int)ch << ">";
            }
            else
            {
                if (!mode)
                {
                    oss << "/";
                    mode = true;
                }
                oss << ch;
            }
            if (oss.tellp() >= 93 - 6)
            {
                if (mode)
                {
                    oss << "/";
                    mode = false;
                }
                m_final->AddLine(oss.str());
                oss.str("");
                oss.clear();
                if (i < str.length() - 1)
                    oss << "\t.ASCII\t";
            }
        }
        if (oss.tellp() > 0)
        {
            if (mode)
                oss << "/";
            m_final->AddLine(oss.str());
        }
    }
}

void Generator::GenerateVariables()
{
    if (m_source->vars.empty())
        return;

    m_final->AddComment("VARIABLES");

    std::sort(m_source->vars.begin(), m_source->vars.end(), CompareVariables);

    for (auto it = std::begin(m_source->vars); it != std::end(m_source->vars); ++it)
    {
        string deconame = DecorateVariableName(it->name);
        //TODO: Calculate number of array elements multiplying all indices
        ValueType vtype = it->GetValueType();
        switch (vtype)
        {
        case ValueTypeInteger:
            m_final->AddLine(deconame + ":\t.WORD\t0\t; " + it->name);
            break;
        case ValueTypeString:
            m_final->AddLine(deconame + ":\t.BLKB\t256.\t; " + it->name);
            break;
        default:  // Single
            m_final->AddLine(deconame + ":\t.WORD\t0,0\t; " + it->name);
            break;
        }
    }
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

    m_line = &(m_source->lines[m_lineindex]);
    m_final->AddComment(m_line->text);
    string linenumlabel = "L" + std::to_string(m_line->number) + ":";//TODO function GetLineNumberLabel
    m_final->AddLine(linenumlabel);

    // Find keyword generator implementation
    KeywordIndex keyword = m_line->statement.token.keyword;
    GeneratorMethodRef methodref = nullptr;
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
        Error("Generator for keyword " + GetKeywordString(keyword) + " not found.");
        return true;
    }

    (this->*methodref)(m_line->statement);

    return true;
}

void Generator::Error(const string& message)
{
    std::cerr << "ERROR in line " << m_line->number << " - " << message << std::endl;
    m_line->error = true;
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
        m_final->AddComment("TODO calculate non-integer expression");
        return;
    }

    if (node.constval)
    {
        int ivalue = (int)std::floor(node.node.dvalue);
        if (ivalue == 0)
        {
            m_final->AddLine("\tCLR\tR0");
        }
        else
        {
            string svalue = "#" + std::to_string(ivalue) + ".";
            m_final->AddLine("\tMOV\t" + svalue + ", R0");
        }
        return;
    }

    if (node.node.type == TokenTypeKeyword && IsFunctionKeyword(node.node.keyword))
    {
        GenerateExprFunction(expr, node);
        return;
    }

    if (node.node.type == TokenTypeIdentifier)
    {
        string deconame = DecorateVariableName(GetCanonicVariableName(node.node.text));
        m_final->AddLine("\tMOV\t" + deconame + ", R0");
        return;
    }

    if (node.node.type == TokenTypeOperation && node.left >= 0 && node.right >= 0)
    {
        GenerateExprBinaryOperation(expr, node);
        return;
    }

    if (node.left != -1 || node.right != -1)
    {
        m_final->AddComment("TODO generate complex expression");
        return;
    }
}

void Generator::GenerateExprBinaryOperation(const ExpressionModel& expr, const ExpressionNode& node)
{
    const ExpressionNode& nodeleft = expr.nodes[node.left];
    const ExpressionNode& noderight = expr.nodes[node.right];

    if (nodeleft.vtype == ValueTypeNone || noderight.vtype == ValueTypeNone)
    {
        std::cerr << "ERROR in expression at " << node.node.line << ":" << node.node.pos << " - Cannot calculate value type for the node." << std::endl;
        m_line->error = true;
        RegisterError();
        return;
    }

    // Find operator implementation
    string text = node.node.text;
    GeneratorOperMethodRef methodref = nullptr;
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
        std::cerr << "ERROR in expression at " << node.node.line << ":" << node.node.pos << " - TODO generate operator \'" + text + "\'." << std::endl;
        m_line->error = true;
        RegisterError();
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
        m_final->AddComment("TODO generate function expression for " + GetKeywordString(keyword));
        return;
    }

    (this->*methodref)(expr, node);
}

// Calculate expression and assign the result to variable
// To use in LET and FOR
void Generator::GenerateAssignment(VariableExpressionModel& var, ExpressionModel& expr)
{
    const string comment = "\t; assignment";

    ValueType vtype = var.GetValueType();
    string deconame = var.GetVariableDecoratedName();

    if (expr.IsConstExpression())
    {
        if (vtype == ValueTypeInteger)
        {
            int ivalue = (int)std::floor(expr.GetConstExpressionDValue());
            if (ivalue == 0)
            {
                m_final->AddLine("\tCLR\t" + deconame + comment);
            }
            else {
                string svalue = "#" + std::to_string(ivalue) + ".";
                m_final->AddLine("\tMOV\t" + svalue + ", " + deconame + comment);
            }
        }
        //TODO: version for Single type
        else if (vtype == ValueTypeString)
        {
            string svalue = expr.GetConstExpressionSValue();
            int sindex = m_source->GetConstStringIndex(svalue);
            //TODO: Special case for one-char string
            m_final->AddLine("\tMOV\t#ST" + std::to_string(sindex) + ", R0");
            m_final->AddLine("\tMOV\t#" + deconame + ", R1");
            m_final->AddLine("\tCALL\tSTRCPY" + comment);
        }
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
            int ivalue = (int)std::floor(expr.nodes[root.right].node.dvalue);
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

void Generator::GenerateIgnoredStatement(StatementModel& statement)
{
    m_final->AddComment(statement.token.text + " statement is ignored");
}

void Generator::GenerateBeep(StatementModel& statement)
{
    m_final->AddLine("\tCALL\tBEEP");
}

void Generator::GenerateClear(StatementModel& statement)
{
    m_final->AddComment("CLEAR statement is ignored");
}

void Generator::GenerateCls(StatementModel& statement)
{
    m_final->AddLine("\tCALL\tCLS");
}

void Generator::GenerateColor(StatementModel& statement)
{
    //TODO
    m_final->AddComment("TODO COLOR");
}

void Generator::GenerateData(StatementModel& statement)
{
    //TODO
    m_final->AddComment("TODO DATA");
}

void Generator::GenerateDim(StatementModel& statement)
{
    // Nothing to generate, DIM variables declared in ProcessEnd()
}

void Generator::GenerateDraw(StatementModel& statement)
{
    //TODO
    m_final->AddComment("TODO DRAW");
}

void Generator::GenerateEnd(StatementModel& statement)
{
    // END generates JMP 65536, but only if END is not on the last line
    int nextlinenum = m_source->GetNextLineNumber(m_line->number);
    if (nextlinenum != MAX_LINE_NUMBER + 1)
        m_final->AddLine("\tJMP\tL" + std::to_string(MAX_LINE_NUMBER + 1));
}

void Generator::GenerateFor(StatementModel& statement)
{
    // Calculate expression for "from"
    assert(statement.args.size() > 1);
    ExpressionModel& expr1 = statement.args[0];

    assert(statement.ident.type == TokenTypeIdentifier);
    VariableExpressionModel var;
    var.name = statement.ident.text;
    string deconame = var.GetVariableDecoratedName();

    // Assign the expression to the loop variable
    GenerateAssignment(var, expr1);

    // Calculate expression for "to"
    string tovalue = "#0";
    ExpressionModel& expr2 = statement.args[1];
    if (expr2.IsConstExpression())
    {
        tovalue = "#" + std::to_string((int)std::floor(expr2.GetConstExpressionDValue())) + ".";
    }
    else if (expr2.IsVariableExpression())
    {
        string svalue = expr2.GetVariableExpressionDecoratedName();
        m_final->AddLine("\tMOV\t" + svalue + ", @#<N" + std::to_string(m_line->number) + "+2>");
    }
    else
    {
        GenerateExpression(expr2);
        m_final->AddLine("\tMOV\tR0, @#<N" + std::to_string(m_line->number) + "+2>");  //  Save "to" value
    }

    if (statement.args.size() > 2)  // has STEP expression
    {
        // Calculate expression for "step"
        ExpressionModel& expr3 = statement.args[2];
        GenerateExpression(expr3);
        // Save "step" value
        m_final->AddLine("\tMOV\tR0, @#<L" + std::to_string(statement.paramline) + "+2>");
    }

    int nextlinenum = m_source->GetNextLineNumber(m_line->number);
    m_final->AddLine("N" + std::to_string(m_line->number) + ":\tCMP\t" + tovalue + ", " + deconame);
    m_final->AddLine("\tBHIS\tL" + std::to_string(nextlinenum));
    m_final->AddLine("\tJMP\tX" + std::to_string(m_line->number));  // label after NEXT
}

void Generator::GenerateGosub(StatementModel& statement)
{
    string calllinenum = "\tCALL\tL" + std::to_string(statement.paramline);
    m_final->AddLine(calllinenum);
}

void Generator::GenerateGoto(StatementModel& statement)
{
    string jmplinenum = "\tJMP\tL" + std::to_string(statement.paramline);
    m_final->AddLine(jmplinenum);
}

void Generator::GenerateIf(StatementModel& statement)
{
    assert(statement.args.size() > 0);
    const ExpressionModel& expr = statement.args[0];
    
    if (expr.IsConstExpression())
    {
        int ivalue = (int)expr.GetConstExpressionDValue();
        if (ivalue != 0)  // TRUE - generate THEN only
        {
            //TODO: Statement under THEN
            int linenum = (int)statement.params[0].dvalue;
            m_final->AddLine("\tJMP\tL" + std::to_string(linenum) + "\t; THEN");
        }
        else  // FALSE - generate ELSE only
        {
            //TODO: Statement under ELSE
            if (statement.params.size() == 1)
                m_final->AddLine("\t\t\t; ELSE do nothing");
            else
            {
                int linenum2 = (int)statement.params[1].dvalue;
                m_final->AddLine("\tJMP\tL" + std::to_string(linenum2) + "\t; ELSE");
            }
        }
        return;
    }

    GenerateExpression(expr);
    //TODO: set flags: Z=0 for TRUE, Z=1 for FALSE

    //TODO: Statement under THEN
    //TODO: Statement under ELSE

    if (statement.params.size() == 1)  // IF expr THEN linenum
    {
        int linenumnext = m_source->GetNextLineNumber(m_line->number);
        m_final->AddLine("\tBEQ\tL" + std::to_string(linenumnext));
        int linenum = (int)statement.params[0].dvalue;
        m_final->AddLine("\tJMP\tL" + std::to_string(linenum));
    }
    else  // IF expr THEN linenum ELSE linenum
    {
        m_final->AddLine("\tBEQ\t10$");
        int linenum1 = (int)statement.params[0].dvalue;
        m_final->AddLine("\tJMP\tL" + std::to_string(linenum1));
        int linenum2 = (int)statement.params[1].dvalue;
        m_final->AddLine("10$:\tJMP\tL" + std::to_string(linenum2));
    }
}

void Generator::GenerateInput(StatementModel& statement)
{
    if (statement.params.size() > 0)  // Write the const string prompt
    {
        Token& param = statement.params[0];
        int strindex = m_source->GetConstStringIndex(param.text);
        string strdeco = "ST" + std::to_string(strindex);
        m_final->AddLine("\tMOV\t" + strdeco + ", R0");
        m_final->AddLine("\tCALL\tWRSTR\t; print the prompt");
    }

    for (auto it = std::begin(statement.variables); it != std::end(statement.variables); ++it)
    {
        ValueType vtype = it->GetValueType();
        string vardeco = it->GetVariableDecoratedName();
        if (vtype == ValueTypeInteger)
        {
            m_final->AddLine("\tCALL\tREADI");
            m_final->AddLine("\tMOV\tR0, " + vardeco);
        }
        else
        {
            m_final->AddComment("TODO INPUT " + it->name);  //TODO
        }
    }
}

void Generator::GenerateOpen(StatementModel& statement)
{
    //TODO
    m_final->AddComment("TODO OPEN");
}

void Generator::GenerateClose(StatementModel& statement)
{
    //TODO
    m_final->AddComment("TODO CLOSE");
}

void Generator::GenerateLet(StatementModel& statement)
{
    assert(statement.args.size() == 1);
    ExpressionModel& expr = statement.args[0];

    VariableExpressionModel& var = statement.varexprs[0];

    GenerateAssignment(var, expr);
}

void Generator::GenerateOn(StatementModel& statement)
{
    ExpressionModel& expr = statement.args[0];
    GenerateExpression(expr);
    int numofcases = statement.params.size();
    string nextline = "L" + std::to_string(m_source->GetNextLineNumber(m_line->number));
    m_final->AddLine("\tDEC\tR0");
    m_final->AddLine("\tBLO\t" + nextline);
    m_final->AddLine("\tCMP\t#" + std::to_string(numofcases) + ", R0");
    m_final->AddLine("\tBGE\t" + nextline);
    m_final->AddLine("\tASL\tR0");
    if (statement.gotogosub)
        m_final->AddLine("\tJMP\t@10$(R0)");
    else
    {
        m_final->AddLine("\tCALL\t@10$(R0)");
        m_final->AddLine("\tBR\t" + nextline);
    }
    int linenum = (int)statement.params[0].dvalue;
    m_final->AddLine("10$:\t.WORD\tL" + std::to_string(linenum));
    for (auto it = std::begin(statement.params); it != std::end(statement.params); ++it)
    {
        linenum = (int)it->dvalue;
        m_final->AddLine("\t.WORD\tL" + std::to_string(linenum));
    }
}

void Generator::GenerateLocate(StatementModel& statement)
{
    assert(statement.args.size() > 0);
    const ExpressionModel& expr1 = statement.args[0];  // column, could be empty

    if (statement.args.size() > 1)
    {
        const ExpressionModel& expr2 = statement.args[1];  // row, could be empty
        //TODO
    }

    if (statement.args.size() > 2)
    {
        const ExpressionModel& expr3 = statement.args[2];  // on/off, could be empty
        //TODO
    }

    //TODO
    m_final->AddComment("TODO LOCATE");
}

void Generator::GeneratePset(StatementModel& statement)
{
    //TODO
    m_final->AddComment("TODO PSET");
}

void Generator::GeneratePreset(StatementModel& statement)
{
    //TODO
    m_final->AddComment("TODO PRESET");
}

void Generator::GenerateNext(StatementModel& statement)
{
    assert(statement.paramline != 0);
    SourceLineModel& linefor = m_source->GetSourceLine(statement.paramline);

    string varname = linefor.statement.ident.text;
    string deconame = DecorateVariableName(GetCanonicVariableName(varname));
    int forlinenum = statement.paramline;

    if (linefor.statement.args.size() < 3)
        m_final->AddLine("\tINC\t" + deconame);
    else
        m_final->AddLine("\tADD\t#1, " + deconame);

    // JMP to continue loop
    m_final->AddLine("\tJMP\tN" + std::to_string(forlinenum));
    // Label after NEXT
    m_final->AddLine("X" + std::to_string(forlinenum) + ":");
}

void Generator::GeneratePoke(StatementModel& statement)
{
    assert(statement.args.size() == 2);

    ExpressionModel& expr1 = statement.args[0];
    //TODO: Simplified version for const/var expression
    GenerateExpression(expr1);
    // Save value
    m_final->AddLine("\tMOV\tR0, @#<10$+2>");

    ExpressionModel& expr2 = statement.args[1];
    //TODO: Simplified version for const/var expression
    GenerateExpression(expr2);
    // Save value
    m_final->AddLine("\tMOV\tR0, @#<10$+4>");

    m_final->AddLine("10$:\tMOV\t#0, #0");
}

void Generator::GenerateOut(StatementModel& statement)
{
    assert(statement.args.size() == 3);

    //TODO
    m_final->AddComment("TODO OUT");  //TODO
}

void Generator::GenerateLine(StatementModel& statement)
{
    //TODO
    m_final->AddComment("TODO LINE");  //TODO
}

void Generator::GenerateCircle(StatementModel& statement)
{
    //TODO
    m_final->AddComment("TODO CIRCLE");  //TODO
}

void Generator::GeneratePaint(StatementModel& statement)
{
    //TODO
    m_final->AddComment("TODO PAINT");  //TODO
}

void Generator::GeneratePrint(StatementModel& statement)
{
    for (auto it = std::begin(statement.args); it != std::end(statement.args); ++it)
    {
        const ExpressionModel& expr = *it;
        assert(!it->IsEmpty());
        const ExpressionNode& root = expr.nodes[expr.root];
        if (root.node.IsKeyword(KeywordAT))
        {
            GeneratePrintAt(expr);
        }
        else if (root.node.IsKeyword(KeywordTAB))
        {
            assert(root.args.size() == 1);
            const ExpressionModel& expr1 = root.args[0];
            GenerateExpression(expr1);
            m_final->AddLine("\tCALL\tWRTAB");
        }
        else if (root.node.IsKeyword(KeywordSPC))
        {
            assert(root.args.size() == 1);
            const ExpressionModel& expr1 = root.args[0];
            if (!expr1.IsConstExpression() || (int)expr1.GetConstExpressionDValue() > 0)  // skip SPC(0)
            {
                GenerateExpression(expr1);
                m_final->AddLine("\tCALL\tWRSPC");
            }
        }
        else if (root.vtype == ValueTypeString)
        {
            GeneratePrintString(expr);
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
        //TODO: Comma
    }
 
    // CR/LF at end of PRINT
    if (!statement.nocrlf)
        m_final->AddLine("\tCALL\tWRCRLF");
}

void Generator::GeneratePrintAt(const ExpressionModel& expr)
{
    const ExpressionNode& root = expr.nodes[expr.root];
    assert(root.args.size() == 2);

    const ExpressionModel& expr1 = root.args[0];
    GenerateExpression(expr1);
    const ExpressionModel& expr2 = root.args[1];
    GenerateExpression(expr2);//TODO: should be in R1

    m_final->AddLine("\tCALL\tPRAT");
}

void Generator::GeneratePrintString(const ExpressionModel& expr)
{
    assert(!expr.IsEmpty());
    const ExpressionNode& root = expr.nodes[expr.root];

    if (root.constval)
    {
        string svalue = root.node.svalue;

        if (svalue.length() == 0)
            return;  // Empty string, nothing to print

        if (svalue.length() == 1)  // one-char string, no use of const string
        {
            //TODO: char to int conversion depends on encoding
            m_final->AddLine("\tMOV\t#" + std::to_string((unsigned int)svalue[0]) + "., R0");
            m_final->AddLine("\tCALL\tWRCHR");
            return;
        }

        int sindex = m_source->GetConstStringIndex(svalue);
        if (sindex < 0)
        {
            Error("Failed to find index for const string \"" + svalue + "\".");
            return;
        }

        m_final->AddLine("\tMOV\t#ST" + std::to_string(sindex) + ", R0");
        m_final->AddLine("\tCALL\tWRSTR");
        return;
    }

    if (root.node.type == TokenTypeIdentifier)
    {
        string deconame = DecorateVariableName(GetCanonicVariableName(root.node.text));
        m_final->AddLine("\tMOV\t#" + deconame + ", R0");
        m_final->AddLine("\tCALL\tWRSTR");
        return;
    }

    //TODO
    m_final->AddComment("TODO PRINT string expression");
}

void Generator::GenerateRead(StatementModel& statement)
{
    //TODO
    m_final->AddComment("TODO READ");
}

void Generator::GenerateRem(StatementModel& statement)
{
    // Do nothing
}

void Generator::GenerateRestore(StatementModel& statement)
{
    //TODO
    m_final->AddComment("TODO RESTORE");
}

void Generator::GenerateReturn(StatementModel& statement)
{
    m_final->AddLine("\tRETURN");
}

void Generator::GenerateScreen(StatementModel& statement)
{
    m_final->AddComment("SCREEN statement is ignored");
}

void Generator::GenerateStop(StatementModel& statement)
{
    m_final->AddLine("\tHALT");
}

void Generator::GenerateWidth(StatementModel& statement)
{
    m_final->AddComment("WIDTH statement is ignored");
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
        int ivalue = (int)std::floor(noderight.node.dvalue);
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
        int ivalue = (int)std::floor(noderight.node.dvalue);
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

void Generator::GenerateOperMul(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    const string comment = "\t; Operation \'*\'";

    // Code to calculate left sub-expression, with result in R0
    GenerateExpression(expr, nodeleft);

    //TODO
    m_final->AddComment("TODO operation multiply");
}

void Generator::GenerateOperDiv(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    const string comment = "\t; Operation \'/\'";

    // Code to calculate left sub-expression, with result in R0
    GenerateExpression(expr, nodeleft);

    //TODO
    m_final->AddComment("TODO operation division");
}

void Generator::GenerateOperDivInt(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    const string comment = "\t; Operation \'\\\'";

    // Code to calculate left sub-expression, with result in R0
    GenerateExpression(expr, nodeleft);

    //TODO
    m_final->AddComment("TODO operation divint");
}

void Generator::GenerateOperMod(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    const string comment = "\t; Operation \'MOD\'";

    // Code to calculate left sub-expression, with result in R0
    GenerateExpression(expr, nodeleft);

    //TODO
    m_final->AddComment("TODO operation MOD");
}

void Generator::GenerateOperPower(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    const string comment = "\t; Operation \'^\'";

    // Code to calculate left sub-expression, with result in R0
    GenerateExpression(expr, nodeleft);

    //TODO
    m_final->AddComment("TODO operation power");
}

void Generator::GenerateLogicOperIntegerArguments(const ExpressionModel& expr, const ExpressionNode& nodeleft, const ExpressionNode& noderight, const string& comment)
{
    // Code to calculate left sub-expression, with result in R0
    GenerateExpression(expr, nodeleft);

    // Convert "XXX < N" into CMP #N., R0
    if (nodeleft.vtype == ValueTypeInteger &&
        noderight.constval && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle))
    {
        int ivalue = (int)std::floor(noderight.node.dvalue);
        m_final->AddLine("\tCMP\t#" + std::to_string(ivalue) + "., R0" + comment);
    }
    else if (noderight.node.type == TokenTypeIdentifier && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle))
    {
        string deconame = DecorateVariableName(GetCanonicVariableName(noderight.node.text));
        m_final->AddLine("\tCMP\t" + deconame + "., R0" + comment);
    }
    else
    {
        m_final->AddLine("\tMOV\tR0, -(SP)");  // PUSH R0
        GenerateExpression(expr, noderight);
        m_final->AddLine("\tMOV\t(SP)+, R0");  // POP R0
        m_final->AddLine("\tCMP\tR1, R0" + comment);
    }
}

void Generator::GenerateOperEqual(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    const string comment = "\t; Operation \'=\'";

    GenerateLogicOperIntegerArguments(expr, nodeleft, noderight, comment);

    //m_final->AddLine("\tBEQ\t");

    //TODO
    m_final->AddComment("TODO operation equal");
}

void Generator::GenerateOperNotEqual(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    const string comment = "\t; Operation \'<>\'";

    GenerateLogicOperIntegerArguments(expr, nodeleft, noderight, comment);

    //m_final->AddLine("\tBNE\t");

    //TODO
    m_final->AddComment("TODO operation not-equal");
}

void Generator::GenerateOperLess(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    const string comment = "\t; Operation \'<\'";

    GenerateLogicOperIntegerArguments(expr, nodeleft, noderight, comment);

    //m_final->AddLine("\tBLO\t");

    //TODO
    m_final->AddComment("TODO operation less");
}

void Generator::GenerateOperGreater(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    const string comment = "\t; Operation \'>\'";

    GenerateLogicOperIntegerArguments(expr, nodeleft, noderight, comment);

    //m_final->AddLine("\tBHI\t");

    //TODO
    m_final->AddComment("TODO operation greater");
}

void Generator::GenerateOperLessOrEqual(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    const string comment = "\t; Operation \'<=\'";

    GenerateLogicOperIntegerArguments(expr, nodeleft, noderight, comment);

    //m_final->AddLine("\tBLO\t");

    //TODO
    m_final->AddComment("TODO operation less or equal");
}

void Generator::GenerateOperGreaterOrEqual(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    const string comment = "\t; Operation \'>=\'";

    GenerateLogicOperIntegerArguments(expr, nodeleft, noderight, comment);

    //m_final->AddLine("\tBHI\t");

    //TODO
    m_final->AddComment("TODO operation greater or equal");
}

void Generator::GenerateOperAnd(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    //const string comment = "\t; Operation \'AND\'";

    //TODO
    m_final->AddComment("TODO operation AND");
}

void Generator::GenerateOperOr(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    //const string comment = "\t; Operation \'OR\'";

    //TODO
    m_final->AddComment("TODO operation OR");
}

void Generator::GenerateOperXor(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    //const string comment = "\t; Operation \'XOR\'";

    //TODO
    m_final->AddComment("TODO operation XOR");
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

    //TODO: Special case for const expression and variable expression
    const ExpressionModel& expr1 = node.args[0];
    GenerateExpression(expr1);
    //TODO: For Single expression, convert to Integer

    m_final->AddLine("\tMOV\t(R0), R0\t; PEEK");
}

void Generator::GenerateFuncInp(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(node.args.size() == 2);

    //TODO: Special case for const expression and variable expression
    const ExpressionModel& expr1 = node.args[0];
    GenerateExpression(expr1);
    //TODO: For Single expression, convert to Integer

    m_final->AddLine("\tMOV\t(R0), R1\t; INP value");

    const ExpressionModel& expr2 = node.args[1];
    GenerateExpression(expr1);
    //TODO: For Single expression, convert to Integer
    //TODO: Invert the mask

    m_final->AddLine("\tBIC\tR0, R1\t; INP mask");
    m_final->AddLine("\tMOV\tR1, R0\t; INP");
}

void Generator::GenerateFuncLen(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(node.args.size() == 1);

    //TODO: Special case for const expression and variable expression
    const ExpressionModel& expr1 = node.args[0];
    GenerateExpression(expr1);

    m_final->AddLine("\tMOV\tR0, R1\t");
    m_final->AddLine("\tCLR\tR0\t");
    m_final->AddLine("\tBISB\t(R1), R0\t; LEN");  // get byte of the string length
}

void Generator::GenerateFuncInkey(const ExpressionModel& expr, const ExpressionNode& node)
{
    //TODO
    m_final->AddComment("TODO INKEY$");
}

void Generator::GenerateFuncPos(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(node.args.size() <= 1);

    // If we have non-const expression then calculate it
    if (node.args.size() > 0)
    {
        const ExpressionModel& expr1 = node.args[0];
        if (!expr.IsConstExpression())
            GenerateExpression(expr1);
    }

    m_final->AddComment("TODO POS");
}


//////////////////////////////////////////////////////////////////////
