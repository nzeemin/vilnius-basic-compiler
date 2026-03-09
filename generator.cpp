
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

GeneratorMethodRef Generator::FindGeneratorMethodRef(KeywordIndex keyword)
{
    for (auto it = std::begin(m_keywordspecs); it != std::end(m_keywordspecs); ++it)
    {
        if (keyword == it->keyword)
            return it->methodref;
    }
    return nullptr;
}

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


//////////////////////////////////////////////////////////////////////


Generator::Generator(SourceModel* source, FinalModel* final)
    : m_source(source), m_final(final), m_lineindex(-1), m_line(nullptr), m_runtimeneeds()
{
    assert(source != nullptr);
    assert(final != nullptr);
}

void Generator::AddRuntimeCall(RuntimeSymbol rtsymbol, string comment)
{
    string rtsymbolname = GetRuntimeSymbolName(rtsymbol);

    if (comment.empty())
        m_final->AddLine("\tCALL\t" + rtsymbolname);
    else
        m_final->AddLine("\tCALL\t" + rtsymbolname + "\t;" + comment);
    
    m_runtimeneeds.insert(rtsymbol);
}

void Generator::ProcessBegin()
{
    //TODO: TITLE
    //AddLine("\t.MCALL\t.EXIT");
    AddLine("START:");
}

void Generator::ProcessEnd()
{
    AddLine("L" + std::to_string(MAX_LINE_NUMBER + 1) + ":");
    AddLine("\tEMT\t350\t; .EXIT");  // In case we run after last line

    GenerateStrings();

    GenerateVariables();

    //GenerateRuntimeNeeds();

    AddLine("\t.END\tSTART");
}

void Generator::GenerateStrings()
{
    if (m_source->conststrings.empty())
        return;

    AddComment("STRINGS");
    AddLine("\t.EVEN");
    for (size_t stno = 0; stno < m_source->conststrings.size(); ++stno)
    {
        string strdeco = "ST" + std::to_string(stno + 1);
        string& str = m_source->conststrings[stno];
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
                AddLine(oss.str());
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
            AddLine(oss.str());
        }
    }
}

void Generator::GenerateVariables()
{
    if (m_source->vars.empty())
        return;

    AddComment("VARIABLES");
    AddLine("\t.EVEN");

    std::sort(m_source->vars.begin(), m_source->vars.end(), CompareVariables);

    for (auto it = std::begin(m_source->vars); it != std::end(m_source->vars); ++it)
    {
        string deconame = DecorateVariableName(it->name);
        //TODO: Calculate number of array elements multiplying all indices
        ValueType vtype = it->GetValueType();
        switch (vtype)
        {
        case ValueTypeInteger:
            AddLine(deconame + ":\t.WORD\t0\t; " + it->name);
            break;
        case ValueTypeString:
            AddLine(deconame + ":\t.BLKB\t256.\t; " + it->name);
            break;
        default:  // Single
            AddLine(deconame + ":\t.WORD\t0,0\t; " + it->name);
            break;
        }
    }
}

void Generator::GenerateRuntimeNeeds()
{
    AddComment("RUNTIME NEEDS");

    for (RuntimeSymbol need : m_runtimeneeds)
    {
        string rtsymbolname = GetRuntimeSymbolName(need);
        AddLine(";\t" + rtsymbolname);
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
    AddComment(m_line->text);
    string linenumlabel = "L" + std::to_string(m_line->number) + ":";//TODO function GetLineNumberLabel
    AddLine(linenumlabel);

    // Find keyword generator implementation
    KeywordIndex keyword = m_line->statement.token.keyword;
    GeneratorMethodRef methodref = FindGeneratorMethodRef(keyword);
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
        AddComment("TODO calculate non-integer expression");
        return;
    }

    if (node.constval)
    {
        int ivalue = (int)std::floor(node.token.dvalue);
        if (ivalue == 0)
        {
            AddLine("\tCLR\tR0");
        }
        else
        {
            string svalue = "#" + std::to_string(ivalue) + ".";
            AddLine("\tMOV\t" + svalue + ", R0");
        }
        return;
    }

    if (node.token.type == TokenTypeKeyword && IsFunctionKeyword(node.token.keyword))
    {
        GenerateExprFunction(expr, node);
        return;
    }

    if (node.token.type == TokenTypeIdentifier)
    {
        string deconame = DecorateVariableName(GetCanonicVariableName(node.token.text));
        AddLine("\tMOV\t" + deconame + ", R0");
        return;
    }

    if (node.token.type == TokenTypeOperation && node.left >= 0 && node.right >= 0)
    {
        GenerateExprBinaryOperation(expr, node);
        return;
    }

    if (node.left != -1 || node.right != -1)
    {
        AddComment("TODO generate complex expression");
        return;
    }
}

void Generator::GenerateExprBinaryOperation(const ExpressionModel& expr, const ExpressionNode& node)
{
    const ExpressionNode& nodeleft = expr.nodes[node.left];
    const ExpressionNode& noderight = expr.nodes[node.right];

    if (nodeleft.vtype == ValueTypeNone || noderight.vtype == ValueTypeNone)
    {
        std::cerr << "ERROR in expression at " << node.token.line << ":" << node.token.pos << " - Cannot calculate value type for the node." << std::endl;
        m_line->error = true;
        RegisterError();
        return;
    }

    // Find operator implementation
    string text = node.token.text;
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
        std::cerr << "ERROR in expression at " << node.token.line << ":" << node.token.pos << " - TODO generate operator \'" + text + "\'." << std::endl;
        m_line->error = true;
        RegisterError();
        return;
    }
}

void Generator::GenerateExprFunction(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(!node.constval);
    assert(node.token.keyword != KeywordNone);
    assert(node.token.type == TokenTypeKeyword && IsFunctionKeyword(node.token.keyword));

    KeywordIndex keyword = node.token.keyword;

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
        AddComment("TODO generate function expression for " + GetKeywordString(keyword));
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
                AddLine("\tCLR\t" + deconame + comment);
            }
            else {
                string svalue = "#" + std::to_string(ivalue) + ".";
                AddLine("\tMOV\t" + svalue + ", " + deconame + comment);
            }
        }
        //TODO: version for Single type
        else if (vtype == ValueTypeString)
        {
            string svalue = expr.GetConstExpressionSValue();
            int sindex = m_source->GetConstStringIndex(svalue);
            //TODO: Special case for one-char string
            AddLine("\tMOV\t#ST" + std::to_string(sindex) + ", R0");
            AddLine("\tMOV\t#" + deconame + ", R1");
            AddRuntimeCall(RuntimeSTRCP, comment);
        }
    }
    else if (expr.IsVariableExpression())
    {
        string svalue = expr.GetVariableExpressionDecoratedName();
        AddLine("\tMOV\t" + svalue + ", " + deconame + comment);
    }
    else
    {
        ExpressionNode& root = expr.nodes[expr.root];

        // Convert "A% = A% + N" and "A% = A% - N" assignments into INC/DEC/ADD/SUB
        if (vtype == ValueTypeInteger && root.token.IsBinaryOperation() &&
            (root.token.text == "-" || root.token.text == "+") &&
            expr.nodes[root.left].token.type == TokenTypeIdentifier &&
            GetCanonicVariableName(expr.nodes[root.left].token.text) == var.name &&
            expr.nodes[root.right].constval &&
            (expr.nodes[root.right].vtype == ValueTypeInteger || expr.nodes[root.right].vtype == ValueTypeSingle))
        {
            bool plusminus = (root.token.text == "+");
            int ivalue = (int)std::floor(expr.nodes[root.right].token.dvalue);
            if (plusminus && ivalue == 1)
                AddLine("\tINC\t" + deconame + comment);
            else if (!plusminus && ivalue == 1)
                AddLine("\tDEC\t" + deconame + comment);
            else if (plusminus && ivalue != 1)
                AddLine("\tADD\t#" + std::to_string(ivalue) + "., " + deconame + comment);
            else //if (!plusminus && ivalue != 1)
                AddLine("\tSUB\t#" + std::to_string(ivalue) + "., " + deconame + comment);
        }
        else
        {
            GenerateExpression(expr);
            AddLine("\tMOV\tR0, " + deconame + comment);
        }
    }
}

void Generator::GenerateIgnoredStatement(StatementModel& statement)
{
    AddComment(statement.token.text + " statement is ignored");
}

void Generator::GenerateBeep(StatementModel&)
{
    AddLine("\tMOV\t#7, R0");
    AddRuntimeCall(RuntimeWRCHR);
}

void Generator::GenerateClear(StatementModel&)
{
    AddComment("CLEAR statement is ignored");
}

void Generator::GenerateCls(StatementModel&)
{
    AddLine("\tMOV\t#14, R0");
    AddRuntimeCall(RuntimeWRCHR);
}

void Generator::GenerateColor(StatementModel& statement)
{
    //TODO
    AddComment("TODO COLOR");
}

void Generator::GenerateData(StatementModel& statement)
{
    //TODO
    AddComment("TODO DATA");
}

void Generator::GenerateDim(StatementModel&)
{
    // Nothing to generate, DIM variables declared in ProcessEnd()
}

void Generator::GenerateDraw(StatementModel& statement)
{
    //TODO
    AddComment("TODO DRAW");
}

void Generator::GenerateEnd(StatementModel&)
{
    // END generates JMP 65536, but only if END is not on the last line
    int nextlinenum = m_source->GetNextLineNumber(m_line->number);
    if (nextlinenum != MAX_LINE_NUMBER + 1)
        AddLine("\tJMP\tL" + std::to_string(MAX_LINE_NUMBER + 1));
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
        AddLine("\tMOV\t" + svalue + ", @#<N" + std::to_string(m_line->number) + "+2>");
    }
    else
    {
        GenerateExpression(expr2);
        AddLine("\tMOV\tR0, @#<N" + std::to_string(m_line->number) + "+2>");  //  Save "to" value
    }

    if (statement.args.size() > 2)  // has STEP expression
    {
        // Calculate expression for "step"
        ExpressionModel& expr3 = statement.args[2];
        GenerateExpression(expr3);
        // Save "step" value
        AddLine("\tMOV\tR0, @#<L" + std::to_string(statement.paramline) + "+2>");
    }

    int nextlinenum = m_source->GetNextLineNumber(m_line->number);
    AddLine("N" + std::to_string(m_line->number) + ":\tCMP\t" + tovalue + ", " + deconame);
    AddLine("\tBHIS\tL" + std::to_string(nextlinenum));
    AddLine("\tJMP\tX" + std::to_string(m_line->number));  // label after NEXT
}

void Generator::GenerateGosub(StatementModel& statement)
{
    string linenum = "\tCALL\tL" + std::to_string(statement.paramline);
    AddLine(linenum);
}

void Generator::GenerateGoto(StatementModel& statement)
{
    string linenum = "\tJMP\tL" + std::to_string(statement.paramline);
    AddLine(linenum);
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
            if (statement.stthen == nullptr)  // THEN linenum
            {
                int linenum = (int)statement.params[0].dvalue;
                AddLine("\tJMP\tL" + std::to_string(linenum) + "\t; THEN");
            }
            else  // Statement under THEN
            {
                StatementModel* pstthen = statement.stthen;
                if (pstthen->token.keyword == KeywordGOTO)  // THEN GOTO linenum
                {
                    int linenum = (int)pstthen->paramline;
                    AddLine("\tJMP\tL" + std::to_string(linenum) + "\t; THEN GOTO");
                }
                else
                {
                    //TODO
                    AddComment("TODO statement under THEN");
                }
            }
        }
        else  // FALSE - generate ELSE only
        {
            if (statement.stelse == nullptr)  // ELSE linenum
            {
                if (statement.params.size() == 1)
                    AddLine("\t\t\t; ELSE do nothing");
                else
                {
                    int linenum2 = (int)statement.params[1].dvalue;
                    AddLine("\tJMP\tL" + std::to_string(linenum2) + "\t; ELSE");
                }
                //TODO
            }
            else  // Statement under ELSE
            {
                StatementModel* pstelse = statement.stelse;
                if (pstelse->token.keyword == KeywordGOTO)  // THEN GOTO linenum
                {
                    int linenum = (int)pstelse->paramline;
                    AddLine("\tJMP\tL" + std::to_string(linenum) + "\t; ELSE GOTO");
                }
                else
                {
                    //TODO
                    AddComment("TODO statement under ELSE");
                }
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
        AddLine("\tBEQ\tL" + std::to_string(linenumnext));
        int linenum = (int)statement.params[0].dvalue;
        AddLine("\tJMP\tL" + std::to_string(linenum));
    }
    else  // IF expr THEN linenum ELSE linenum
    {
        AddLine("\tBEQ\t10$");
        int linenum1 = (int)statement.params[0].dvalue;
        AddLine("\tJMP\tL" + std::to_string(linenum1));
        int linenum2 = (int)statement.params[1].dvalue;
        AddLine("10$:\tJMP\tL" + std::to_string(linenum2));
    }
}

void Generator::GenerateInput(StatementModel& statement)
{
    if (statement.params.size() > 0)  // Write the const string prompt
    {
        Token& param = statement.params[0];
        int strindex = m_source->GetConstStringIndex(param.text);
        string strdeco = "ST" + std::to_string(strindex);
        AddLine("\tMOV\t" + strdeco + ", R0");
        AddRuntimeCall(RuntimeWRSTR, "print the prompt");
    }

    for (auto it = std::begin(statement.variables); it != std::end(statement.variables); ++it)
    {
        ValueType vtype = it->GetValueType();
        string vardeco = it->GetVariableDecoratedName();
        if (vtype == ValueTypeInteger)
        {
            AddRuntimeCall(RuntimeREADI);
            AddLine("\tMOV\tR0, " + vardeco);
        }
        else
        {
            AddComment("TODO INPUT " + it->name);  //TODO
        }
    }
}

void Generator::GenerateOpen(StatementModel& statement)
{
    //TODO
    AddComment("TODO OPEN");
}

void Generator::GenerateClose(StatementModel& statement)
{
    //TODO
    AddComment("TODO CLOSE");
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
    AddLine("\tDEC\tR0");
    AddLine("\tBLO\t" + nextline);
    AddLine("\tCMP\t#" + std::to_string(numofcases) + ", R0");
    AddLine("\tBGE\t" + nextline);
    AddLine("\tASL\tR0");
    if (statement.gotogosub)
        AddLine("\tJMP\t@10$(R0)");
    else
    {
        AddLine("\tCALL\t@10$(R0)");
        AddLine("\tBR\t" + nextline);
    }
    int linenum = (int)statement.params[0].dvalue;
    AddLine("10$:\t.WORD\tL" + std::to_string(linenum));
    for (auto it = std::begin(statement.params); it != std::end(statement.params); ++it)
    {
        linenum = (int)it->dvalue;
        AddLine("\t.WORD\tL" + std::to_string(linenum));
    }
}

void Generator::GenerateLocate(StatementModel& statement)
{
    assert(statement.args.size() > 0);
    //const ExpressionModel& expr1 = statement.args[0];  // column, could be empty

    if (statement.args.size() > 1)
    {
        //const ExpressionModel& expr2 = statement.args[1];  // row, could be empty
        //TODO
    }

    if (statement.args.size() > 2)
    {
        //const ExpressionModel& expr3 = statement.args[2];  // on/off, could be empty
        //TODO
    }

    //TODO
    AddComment("TODO LOCATE");
}

void Generator::GeneratePset(StatementModel& statement)
{
    //TODO
    AddComment("TODO PSET");
}

void Generator::GeneratePreset(StatementModel& statement)
{
    //TODO
    AddComment("TODO PRESET");
}

void Generator::GenerateNext(StatementModel& statement)
{
    assert(statement.paramline != 0);
    SourceLineModel& linefor = m_source->GetSourceLine(statement.paramline);

    string varname = linefor.statement.ident.text;
    string deconame = DecorateVariableName(GetCanonicVariableName(varname));
    int forlinenum = statement.paramline;

    if (linefor.statement.args.size() < 3)
        AddLine("\tINC\t" + deconame);
    else
        AddLine("\tADD\t#1, " + deconame);

    // JMP to continue loop
    AddLine("\tJMP\tN" + std::to_string(forlinenum));
    // Label after NEXT
    AddLine("X" + std::to_string(forlinenum) + ":");
}

void Generator::GeneratePoke(StatementModel& statement)
{
    assert(statement.args.size() == 2);

    ExpressionModel& expr1 = statement.args[0];
    //TODO: Simplified version for const/var expression
    GenerateExpression(expr1);
    // Save value
    AddLine("\tMOV\tR0, @#<10$+2>");

    ExpressionModel& expr2 = statement.args[1];
    //TODO: Simplified version for const/var expression
    GenerateExpression(expr2);
    // Save value
    AddLine("\tMOV\tR0, @#<10$+4>");

    AddLine("10$:\tMOV\t#0, #0");
}

void Generator::GenerateOut(StatementModel& statement)
{
    assert(statement.args.size() == 3);

    //TODO
    AddComment("TODO OUT");  //TODO
}

void Generator::GenerateLine(StatementModel& statement)
{
    //TODO
    AddComment("TODO LINE");  //TODO
}

void Generator::GenerateCircle(StatementModel& statement)
{
    //TODO
    AddComment("TODO CIRCLE");  //TODO
}

void Generator::GeneratePaint(StatementModel& statement)
{
    //TODO
    AddComment("TODO PAINT");  //TODO
}

void Generator::GeneratePrint(StatementModel& statement)
{
    for (auto it = std::begin(statement.args); it != std::end(statement.args); ++it)
    {
        const ExpressionModel& expr = *it;
        assert(!it->IsEmpty());
        const ExpressionNode& root = expr.nodes[expr.root];
        if (root.token.IsKeyword(KeywordAT))
        {
            GeneratePrintAt(expr);
        }
        else if (root.token.IsKeyword(KeywordTAB))
        {
            assert(root.args.size() == 1);
            const ExpressionModel& expr1 = root.args[0];
            GenerateExpression(expr1);
            AddRuntimeCall(RuntimeWRTAB);
        }
        else if (root.token.IsKeyword(KeywordSPC))
        {
            assert(root.args.size() == 1);
            const ExpressionModel& expr1 = root.args[0];
            if (!expr1.IsConstExpression() || (int)expr1.GetConstExpressionDValue() > 0)  // skip SPC(0)
            {
                GenerateExpression(expr1);
                AddRuntimeCall(RuntimeWRSPC);
            }
        }
        else if (root.vtype == ValueTypeString)
        {
            GeneratePrintString(expr);
        }
        else if (root.vtype == ValueTypeInteger)
        {
            GenerateExpression(expr);
            AddRuntimeCall(RuntimeWRINT);
        }
        else if (root.vtype == ValueTypeSingle)
        {
            GenerateExpression(expr);
            AddRuntimeCall(RuntimeWRSNG);
        }
        //TODO: Comma
    }
 
    // CR/LF at end of PRINT
    if (!statement.nocrlf)
        AddRuntimeCall(RuntimeWREOL);
}

void Generator::GeneratePrintAt(const ExpressionModel& expr)
{
    const ExpressionNode& root = expr.nodes[expr.root];
    assert(root.args.size() == 2);

    const ExpressionModel& expr1 = root.args[0];
    GenerateExpression(expr1);
    const ExpressionModel& expr2 = root.args[1];
    GenerateExpression(expr2);//TODO: should be in R1

    AddRuntimeCall(RuntimeWRAT);
}

void Generator::GeneratePrintString(const ExpressionModel& expr)
{
    assert(!expr.IsEmpty());
    const ExpressionNode& root = expr.nodes[expr.root];

    if (root.constval)
    {
        string svalue = root.token.svalue;

        if (svalue.length() == 0)
            return;  // Empty string, nothing to print

        if (svalue.length() == 1)  // one-char string, no use of const string
        {
            //TODO: char to int conversion depends on encoding
            AddLine("\tMOV\t#" + std::to_string((unsigned int)svalue[0]) + "., R0");
            AddRuntimeCall(RuntimeWREOL);
            return;
        }

        int sindex = m_source->GetConstStringIndex(svalue);
        if (sindex < 0)
        {
            Error("Failed to find index for const string \"" + svalue + "\".");
            return;
        }

        AddLine("\tMOV\t#ST" + std::to_string(sindex) + ", R0");
        AddRuntimeCall(RuntimeWRSTR);
        return;
    }

    if (root.token.type == TokenTypeIdentifier)
    {
        string deconame = DecorateVariableName(GetCanonicVariableName(root.token.text));
        AddLine("\tMOV\t#" + deconame + ", R0");
        AddRuntimeCall(RuntimeWRSTR);
        return;
    }

    //TODO
    AddComment("TODO PRINT string expression");
}

void Generator::GenerateRead(StatementModel& statement)
{
    //TODO
    AddComment("TODO READ");
}

void Generator::GenerateRem(StatementModel& statement)
{
    // Do nothing
}

void Generator::GenerateRestore(StatementModel& statement)
{
    //TODO
    AddComment("TODO RESTORE");
}

void Generator::GenerateReturn(StatementModel& statement)
{
    AddLine("\tRETURN");
}

void Generator::GenerateScreen(StatementModel& statement)
{
    AddComment("SCREEN statement is ignored");
}

void Generator::GenerateStop(StatementModel& statement)
{
    AddLine("\tHALT");
}

void Generator::GenerateWidth(StatementModel& statement)
{
    AddComment("WIDTH statement is ignored");
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
        int ivalue = (int)std::floor(noderight.token.dvalue);
        if (ivalue == 0)
            ;  // Do nothing
        else if (ivalue == 1)
            AddLine("\tINC\tR0" + comment);
        else  // ivalue != 1
            AddLine("\tADD\t#" + std::to_string(ivalue) + "., R0" + comment);
        return;
    }

    // Special case for noderight as variable
    if (nodeleft.vtype == ValueTypeInteger && noderight.vtype == ValueTypeInteger && noderight.token.type == TokenTypeIdentifier)
    {
        string deconame = DecorateVariableName(GetCanonicVariableName(noderight.token.text));
        AddLine("\tADD\t" + deconame + ", R0" + comment);
        return;
    }

    AddLine("\tMOV\tR0, -(SP)");  // PUSH R0
    GenerateExpression(expr, noderight);
    AddLine("\tADD\t(SP)+, R0" + comment);  // POP & ADD
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
        int ivalue = (int)std::floor(noderight.token.dvalue);
        if (ivalue == 0)
            ;  // Do nothing
        else if (ivalue == 1)
            AddLine("\tDEC\tR0" + comment);
        else  // ivalue != 1
            AddLine("\tSUB\t#" + std::to_string(ivalue) + "., R0" + comment);
        return;
    }

    // Special case for noderight as variable
    if (nodeleft.vtype == ValueTypeInteger && noderight.vtype == ValueTypeInteger && noderight.token.type == TokenTypeIdentifier)
    {
        string deconame = DecorateVariableName(GetCanonicVariableName(noderight.token.text));
        AddLine("\tSUB\t" + deconame + ", R0" + comment);
        return;
    }

    AddLine("\tMOV\tR0, -(SP)");  // PUSH R0
    GenerateExpression(expr, noderight);
    AddLine("\tMOV\tR0, R1");
    AddLine("\tMOV\t(SP)+, R0");  // POP R0
    AddLine("\tSUB\tR1, R0" + comment);
}

void Generator::GenerateOperMul(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    const string comment = "\t; Operation \'*\'";

    // Code to calculate left sub-expression, with result in R0
    GenerateExpression(expr, nodeleft);

    //TODO
    AddComment("TODO operation multiply");
}

void Generator::GenerateOperDiv(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    const string comment = "\t; Operation \'/\'";

    // Code to calculate left sub-expression, with result in R0
    GenerateExpression(expr, nodeleft);

    //TODO
    AddComment("TODO operation division");
}

void Generator::GenerateOperDivInt(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    const string comment = "\t; Operation \'\\\'";

    // Code to calculate left sub-expression, with result in R0
    GenerateExpression(expr, nodeleft);

    //TODO
    AddComment("TODO operation divint");
}

void Generator::GenerateOperMod(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    const string comment = "\t; Operation \'MOD\'";

    // Code to calculate left sub-expression, with result in R0
    GenerateExpression(expr, nodeleft);

    //TODO
    AddComment("TODO operation MOD");
}

void Generator::GenerateOperPower(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    const string comment = "\t; Operation \'^\'";

    // Code to calculate left sub-expression, with result in R0
    GenerateExpression(expr, nodeleft);

    //TODO
    AddComment("TODO operation power");
}

void Generator::GenerateLogicOperIntegerArguments(const ExpressionModel& expr, const ExpressionNode& nodeleft, const ExpressionNode& noderight, const string& comment)
{
    // Code to calculate left sub-expression, with result in R0
    GenerateExpression(expr, nodeleft);

    // Convert "XXX < N" into CMP #N., R0
    if (nodeleft.vtype == ValueTypeInteger &&
        noderight.constval && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle))
    {
        int ivalue = (int)std::floor(noderight.token.dvalue);
        AddLine("\tCMP\t#" + std::to_string(ivalue) + "., R0" + comment);
    }
    else if (noderight.token.type == TokenTypeIdentifier && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle))
    {
        string deconame = DecorateVariableName(GetCanonicVariableName(noderight.token.text));
        AddLine("\tCMP\t" + deconame + "., R0" + comment);
    }
    else
    {
        AddLine("\tMOV\tR0, -(SP)");  // PUSH R0
        GenerateExpression(expr, noderight);
        AddLine("\tMOV\t(SP)+, R0");  // POP R0
        AddLine("\tCMP\tR1, R0" + comment);
    }
}

void Generator::GenerateOperEqual(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    const string comment = "\t; Operation \'=\'";

    GenerateLogicOperIntegerArguments(expr, nodeleft, noderight, comment);

    //AddLine("\tBEQ\t");

    //TODO
    AddComment("TODO operation equal");
}

void Generator::GenerateOperNotEqual(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    const string comment = "\t; Operation \'<>\'";

    GenerateLogicOperIntegerArguments(expr, nodeleft, noderight, comment);

    //AddLine("\tBNE\t");

    //TODO
    AddComment("TODO operation not-equal");
}

void Generator::GenerateOperLess(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    const string comment = "\t; Operation \'<\'";

    GenerateLogicOperIntegerArguments(expr, nodeleft, noderight, comment);

    //AddLine("\tBLO\t");

    //TODO
    AddComment("TODO operation less");
}

void Generator::GenerateOperGreater(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    const string comment = "\t; Operation \'>\'";

    GenerateLogicOperIntegerArguments(expr, nodeleft, noderight, comment);

    //AddLine("\tBHI\t");

    //TODO
    AddComment("TODO operation greater");
}

void Generator::GenerateOperLessOrEqual(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    const string comment = "\t; Operation \'<=\'";

    GenerateLogicOperIntegerArguments(expr, nodeleft, noderight, comment);

    //AddLine("\tBLO\t");

    //TODO
    AddComment("TODO operation less or equal");
}

void Generator::GenerateOperGreaterOrEqual(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    const string comment = "\t; Operation \'>=\'";

    GenerateLogicOperIntegerArguments(expr, nodeleft, noderight, comment);

    //AddLine("\tBHI\t");

    //TODO
    AddComment("TODO operation greater or equal");
}

void Generator::GenerateOperAnd(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    //const string comment = "\t; Operation \'AND\'";

    //TODO
    AddComment("TODO operation AND");
}

void Generator::GenerateOperOr(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    //const string comment = "\t; Operation \'OR\'";

    //TODO
    AddComment("TODO operation OR");
}

void Generator::GenerateOperXor(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    //const string comment = "\t; Operation \'XOR\'";

    //TODO
    AddComment("TODO operation XOR");
}


// Function generation ///////////////////////////////////////////////

void Generator::GenerateFuncAbs(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(node.args.size() == 1);

    const ExpressionModel& expr1 = node.args[0];
    GenerateExpression(expr1);

    AddLine("\tBPL\t1$");
    AddLine("\tNEG\tR0");
    AddLine("1$:");
}

void Generator::GenerateFuncRnd(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(node.args.size() == 1);

    const ExpressionModel& expr1 = node.args[0];
    GenerateExpression(expr1);
    //TODO: For Single expression, convert to Integer

    AddRuntimeCall(RuntimeRND);
}

void Generator::GenerateFuncPeek(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(node.args.size() == 1);

    //TODO: Special case for const expression and variable expression
    const ExpressionModel& expr1 = node.args[0];
    GenerateExpression(expr1);
    //TODO: For Single expression, convert to Integer

    AddLine("\tMOV\t(R0), R0\t; PEEK");
}

void Generator::GenerateFuncInp(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(node.args.size() == 2);

    //TODO: Special case for const expression and variable expression
    const ExpressionModel& expr1 = node.args[0];
    GenerateExpression(expr1);
    //TODO: For Single expression, convert to Integer

    AddLine("\tMOV\t(R0), R1\t; INP value");

    const ExpressionModel& expr2 = node.args[1];
    GenerateExpression(expr2);
    //TODO: For Single expression, convert to Integer
    //TODO: Invert the mask

    AddLine("\tBIC\tR0, R1\t; INP mask");
    AddLine("\tMOV\tR1, R0\t; INP");
}

void Generator::GenerateFuncLen(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(node.args.size() == 1);

    //TODO: Special case for const expression and variable expression
    const ExpressionModel& expr1 = node.args[0];
    GenerateExpression(expr1);

    AddLine("\tMOV\tR0, R1\t");
    AddLine("\tCLR\tR0\t");
    AddLine("\tBISB\t(R1), R0\t; LEN");  // get byte of the string length
}

void Generator::GenerateFuncInkey(const ExpressionModel& expr, const ExpressionNode& node)
{
    //TODO
    AddComment("TODO INKEY$");
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

    AddComment("TODO POS");
}


//////////////////////////////////////////////////////////////////////
