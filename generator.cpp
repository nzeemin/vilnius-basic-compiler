
#include <cassert>
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
    { "EQV",            &Generator::GenerateOperEqv },
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
    { KeywordCSRLIN,    &Generator::GenerateFuncCsrlin },
    { KeywordPOS,       &Generator::GenerateFuncPos },
    { KeywordSQR,       &Generator::GenerateFuncSqr },
    { KeywordSIN,       &Generator::GenerateFuncSin },
    { KeywordCOS,       &Generator::GenerateFuncCos },
    { KeywordTAN,       &Generator::GenerateFuncTan },
    { KeywordATN,       &Generator::GenerateFuncAtn },
    { KeywordEXP,       &Generator::GenerateFuncExp },
    { KeywordLOG,       &Generator::GenerateFuncLog },
    { KeywordCINT,      &Generator::GenerateFuncCint },
    { KeywordFIX,       &Generator::GenerateFuncFix },
    { KeywordINT,       &Generator::GenerateFuncInt },
    { KeywordSGN,       &Generator::GenerateFuncSgn },
    { KeywordCSNG,      &Generator::GenerateFuncCsng },
    { KeywordASC,       &Generator::GenerateFuncAsc },
    { KeywordIIF,       &Generator::GenerateFuncIif },
};


// Comparison function to sort variables by decorated names
static bool CompareVariables(const VariableModel& a, const VariableModel& b)
{
    string deconamea = a.GetVariableDecoratedName();
    string deconameb = b.GetVariableDecoratedName();
    return deconamea < deconameb;
}

static string to_string_octal(uint16_t value)
{
    string result;
    for (int i = 0; i < 6; i++)
    {
        result.insert(0, 1, '0' + (value & 7));
        value >>= 3;
    }
    return result;
}

static string to_string_float(float value)
{
    string result = std::to_string(value);
    while (result[result.size() - 1] == '0')  // trim ending zeroes
        result.erase(result.size() - 1);
    return result;
}

static uint32_t float_to_dec_float(float fvalue)
{
    uint32_t bits;  std::memcpy(&bits, &fvalue, sizeof(uint32_t));
    if (bits != 0)
    {
        int exp = (((bits >> 24) & 0x7F) + 1) & 0x7F;
        bits = (bits & 0x80FFFFFF) | (exp << 24);
    }
    return bits;
}


//////////////////////////////////////////////////////////////////////


// Get expression value as integer, put in register R0.
// Use only when we know expr.IsConstExpression() == true, and it can't be ValueTypeString.
#define GET_CONSTEXPR_INT_VALUE_IN_R0(expr) { \
    int ivalue = (int)std::floor(expr.GetConstExpressionDValue()); \
    if (ivalue == 0) \
        AddLine("\tCLR\tR0"); \
    else \
        AddLine("\tMOV\t#" + std::to_string(ivalue) + "., R0"); \
}
// Get expression value as integer, put in register R1.
// Use only when we know expr.IsConstExpression() == true, and it can't be ValueTypeString.
#define GET_CONSTEXPR_INT_VALUE_IN_R1(expr) { \
    int ivalue = (int)std::floor(expr.GetConstExpressionDValue()); \
    if (ivalue == 0) \
        AddLine("\tCLR\tR1"); \
    else \
        AddLine("\tMOV\t#" + std::to_string(ivalue) + "., R1"); \
}

// For constant Integer/Single expression expr, returns one of:
//   " CLR "
//   " MOV #NNNNN, "
static string GET_CONSTEXPR_INT_VALUE_AS_CLRMOV(ExpressionModel expr)
{
    int ivalue = (int)std::floor(expr.GetConstExpressionDValue());
    if (ivalue == 0)
        return "\tCLR\t";
    else \
        return "\tMOV\t#" + std::to_string(ivalue) + "., ";
}


//////////////////////////////////////////////////////////////////////


Generator::Generator(SourceModel* source, FinalModel* final)
    : m_source(source), m_final(final), m_lineindex(-1), m_line(nullptr),
    m_runtimeneeds(), m_notimplemented()
{
    assert(source != nullptr);
    assert(final != nullptr);
}

void Generator::AddRuntimeCall(RuntimeSymbol rtsymbol, string comment)
{
    string rtsymbolname = GetRuntimeSymbolName(rtsymbol);

    // FIS implemented on hardware
    bool hardwarefis = (g_platform == PlatformUKNC) && 
        (rtsymbol >= RuntimeFADD && rtsymbol <= RuntimeFDIV);
    string statement = hardwarefis
        ? "\t" + rtsymbolname + "\tSP"
        : "\tCALL\t" + rtsymbolname;

    if (comment.empty())
        m_final->AddLine(statement);
    else
        m_final->AddLine(statement + "\t; " + comment);
    
    if (!hardwarefis)
        m_runtimeneeds.insert(rtsymbol);
}

void Generator::ProcessBegin()
{
    AddLine("START:");
    if (g_platform == PlatformUKNC)
    {
        AddLine("\tMTPS\t#340\t; disable interrupts");
        AddLine("\tCLR\t@#177560");
        AddLine("\tMTPS\t#0\t; enable interrupts");
    }
    AddLine("\tMOV\tSP, SAVESP");
}

void Generator::ProcessEnd()
{
    AddLine("LEND:");
    AddLine("SAVESP = . + 2");
    AddLine("\tMOV\t#776, SP\t; restore SP");
    if (g_platform == PlatformBK0010)
        AddLine("\tRETURN\t; return to Monitor/OS/etc.");
    else if (g_platform == PlatformUKNC)
        AddLine("\tEMT\t350\t; .EXIT");

    // Enumerate all the prepared lines to format them properly
    for (string& line : m_final->lines)
    {
        int pos = 0;
        for (size_t i = 0; i < line.size(); i++)
        {
            char ch = line[i];
            if (ch == '\t')
            {
                pos = (pos + 8) / 8 * 8;
                if (pos == 24)
                {
                    line.insert(i, 1, '\t');
                    i++;
                }
                else if (pos > 32)
                {
                    line[i] = ' ';  // replace tab with space char
                }
            }
            else
                pos++;
        }
    }

    GenerateStrings();

    GenerateVariables();

    GenerateRuntimeNeeds();

    //NOTE: .END instruction will be generated in main.cpp

    // Show list of statements/functions not implemented yet
    if (!m_notimplemented.empty())
    {
        std::cerr << "WARNING: The following statements/functions have not yet been implemented:" << std::endl;
        bool needcomma = false;
        for (KeywordIndex keyword : m_notimplemented)
        {
            if (needcomma)
                std::cerr << ", ";
            std::cerr << GetKeywordString(keyword);
            needcomma = true;
        }
        std::cerr << std::endl;
    }
}

void Generator::GenerateStrings()
{
    if (m_source->conststrings.empty())
        return;

    AddComment("STRINGS");
    AddLine("\t.EVEN");
    AddLine("ST0:\t.WORD\t0\t; empty string");

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
    AddComment("RUNTIME CALLS");

    int countinline = 0;
    string line;
    for (RuntimeSymbol need : m_runtimeneeds)
    {
        if (line.empty())
            line = g_turbo8 ? ";\t" : "\t.GLOBL\t";
        if (countinline > 0)
            line += ", ";
        line += GetRuntimeSymbolName(need);
        countinline++;
        if (countinline >= 4)
        {
            AddLine(line);
            line.clear();
            countinline = 0;
        }
    }
    if (!line.empty())
        AddLine(line);
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
    m_local = 0;  // reset local labels counter

    // Show the line text and line number, unless it's a comment line without line number
    if (m_line->linenum != 0 ||
        m_line->statement.token.keyword != KeywordREM)
    {
        AddComment(m_line->text);
        string linenumlabel = m_line->GetLineNumberLabel() + ":";
        AddLine(linenumlabel);
    }

    GenerateStatement(m_line->statement);

    return true;
}

void Generator::Error(const string& message)
{
    std::cerr << "ERROR in line " << m_line->linenum << " - " << message << std::endl;
    m_line->error = true;
    RegisterError();
}

void Generator::Warning(const Token& token, const string& message)
{
    std::cerr << "WARNING: at " << token.line << ":" << token.pos;
    if (m_line->linenum != 0)
        std::cerr << " line " << m_line->linenum;
    std::cerr << " - " << message << std::endl;
}

void Generator::GenerateStatement(StatementModel& statement)
{
    // Find keyword generator implementation
    KeywordIndex keyword = statement.token.keyword;
    GeneratorMethodRef methodref = FindGeneratorMethodRef(keyword);
    if (methodref == nullptr)
    {
        Error("Generator for keyword " + GetKeywordString(keyword) + " not found.");
        return;
    }

    (this->*methodref)(statement);
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

    if (node.constval)
    {
        switch (node.vtype)
        {
        case ValueTypeInteger:
        {
            int ivalue = (int)std::floor(node.token.dvalue);
            if (ivalue == 0)
                AddLine("\tCLR\tR0");
            else
            {
                string svalue = "#" + std::to_string(ivalue) + ".";
                AddLine("\tMOV\t" + svalue + ", R0");
            }
            return;
        }
        case ValueTypeSingle:
        {
            float fvalue = static_cast<float>(node.token.dvalue);
            string comment = "const " + to_string_float(fvalue);
            uint32_t bits = float_to_dec_float(fvalue);
            uint16_t wordlo = bits & 0xFFFF;
            uint16_t wordhi = bits >> 16;
            AddLine((wordlo == 0 ? "\tCLR\t" : "\tMOV\t#" + to_string_octal(wordlo) + ", ") + "-(SP)\t; " + comment);
            AddLine((wordhi == 0 ? "\tCLR\t" : "\tMOV\t#" + to_string_octal(wordhi) + ", ") + "-(SP)");
            return;
        }
        case ValueTypeString:
            AddComment("TODO constval String");
            return;
        }
    }

    if (node.vtype == ValueTypeString)
    {
        AddComment("TODO calculate string expression");
        return;
    }

    // Function
    if (node.token.type == TokenTypeKeyword && IsFunctionKeyword(node.token.keyword))
    {
        GenerateExprFunction(expr, node);
        return;
    }

    // Variable
    if (node.token.type == TokenTypeIdentifier)
    {
        string canoname = GetCanonicVariableName(node.token.text);
        string deconame = DecorateVariableName(canoname);
        if (node.vtype == ValueTypeSingle)
        {
            AddLine("\tMOV\t" + deconame + ",   -(SP)\t; var " + canoname);  // lower
            AddLine("\tMOV\t" + deconame + "+2, -(SP)");  // higher
        }
        else  // Integer, String
        {
            AddLine("\tMOV\t" + deconame + ", R0\t; var " + canoname);
        }
        return;
    }

    // Binary operation
    if (node.token.type == TokenTypeOperation && node.left >= 0 && node.right >= 0)
    {
        GenerateExprBinaryOperation(expr, node);
        return;
    }
    // Unary operation
    else if (node.token.type == TokenTypeOperation && node.left == -1 && node.right >= 0)
    {
        if (node.token.keyword == KeywordNOT)
            GenerateExprUnaryNot(expr, node);
        else if (node.token.text == "-")  // unary '-'
            GenerateExprUnaryMinus(expr, node);
        //TODO: unary +
        else
            AddComment("TODO generate unary operation " + node.token.text);
        return;
    }

    if (node.left != -1 || node.right != -1)
    {
        AddComment("TODO generate complex expression");
        return;
    }
}

void Generator::GenerateExprUnaryNot(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(node.left == -1);
    assert(node.right >= 0);

    const ExpressionNode& noderight = expr.nodes[node.right];
    assert(noderight.vtype != ValueTypeString);

    GenerateExpression(expr, noderight);

    AddLine("\tCOM\tR0\t; NOT");
}

void Generator::GenerateExprUnaryMinus(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(node.left == -1);
    assert(node.right >= 0);

    const string comment = "\t; unary \'-\'";

    const ExpressionNode& noderight = expr.nodes[node.right];
    assert(noderight.vtype != ValueTypeString);

    GenerateExpression(expr, noderight);

    if (noderight.vtype == ValueTypeInteger)
        AddLine("\tNEG\tR0" + comment);
    else if (noderight.vtype == ValueTypeSingle)
        AddLine("\tADD\t#100000, (SP)" + comment);  // invert sign
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
        m_notimplemented.insert(keyword);
        return;
    }

    (this->*methodref)(expr, node);
}

// Calculate expression and assign the result to variable
// To use in LET and FOR
void Generator::GenerateAssignment(VariableExpressionModel& var, ExpressionModel& expr)
{
    ValueType vtype = var.GetValueType();
    string canoname = var.GetVariableCanonicName();
    string deconame = var.GetVariableDecoratedName();

    const string comment = "\t; var " + canoname + " assignment";

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
        else if (vtype == ValueTypeSingle)
        {
            float fvalue = static_cast<float>(expr.GetConstExpressionDValue());
            string comment = "\t; var " + canoname + " = const " + to_string_float(static_cast<float>(expr.GetConstExpressionDValue()));
            uint32_t bits = float_to_dec_float(fvalue);
            uint16_t wordlo = bits & 0xFFFF;
            uint16_t wordhi = bits >> 16;
            AddLine((wordlo == 0 ? "\tCLR\t" : "\tMOV\t#" + to_string_octal(wordlo) + ", ") + deconame + comment);
            AddLine((wordhi == 0 ? "\tCLR\t" : "\tMOV\t#" + to_string_octal(wordhi) + ", ") + deconame + "+2");
        }
        else if (vtype == ValueTypeString)
        {
            string svalue = expr.GetConstExpressionSValue();
            int sindex = m_source->GetConstStringIndex(svalue);
            //TODO: Special case for one-char string
            AddLine("\tMOV\t#ST" + std::to_string(sindex) + ", R0");
            AddLine("\tMOV\t#" + deconame + ", R1");
            AddRuntimeCall(RuntimeSTCP, comment);
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
        else if (vtype == ValueTypeSingle)
        {
            GenerateExpression(expr);
            if (expr.GetExpressionValueType() == ValueTypeInteger)
                AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack
            AddLine("\tMOV\t(SP)+, " + deconame + "+2" + comment);
            AddLine("\tMOV\t(SP)+, " + deconame);
        }
        else if (vtype == ValueTypeInteger)
        {
            GenerateExpression(expr);
            if (expr.GetExpressionValueType() == ValueTypeSingle)
                AddRuntimeCall(RuntimeFTOI, "to Integer");  // result in R0
            AddLine("\tMOV\tR0, " + deconame + comment);
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
    Warning(statement.token, statement.token.text + " statement is ignored");
}

void Generator::GenerateBeep(StatementModel&)
{
    AddLine("\tMOV\t#7, R0\t; bell");
    AddRuntimeCall(RuntimeWRCH, "PRINT char");
}

void Generator::GenerateClear(StatementModel& statement)
{
    AddComment("CLEAR statement is ignored");
    Warning(statement.token, "CLEAR statement is ignored");
}

void Generator::GenerateCls(StatementModel&)
{
    AddLine("\tMOV\t#14, R0");
    AddRuntimeCall(RuntimeWRCH, "PRINT char");
}

void Generator::GenerateColor(StatementModel& statement)
{
    assert(statement.args.size() > 0);

    ExpressionModel& expr1 = statement.args[0];  // foreground color number
    assert(expr1.GetExpressionValueType() != ValueTypeString);
    string stat1;
    if (expr1.IsEmpty())
        stat1 = "\tMOV\t#-1, ";
    else
    {
        if (expr1.IsConstExpression())
            stat1 = GET_CONSTEXPR_INT_VALUE_AS_CLRMOV(expr1);
        else if (expr1.IsVariableExpression() && expr1.GetExpressionValueType() == ValueTypeInteger)
            stat1 = "\tMOV\t" + expr1.GetVariableExpressionDecoratedName() + ", ";
        else
        {
            GenerateExpression(expr1);
            stat1 = "\tMOV\tR0, ";
        }
    }

    string stat2;
    if (statement.args.size() < 2 || statement.args[1].IsEmpty())
        stat2 = "\tMOV\t#-1, ";
    else
    {
        ExpressionModel& expr2 = statement.args[1];
        assert(expr2.GetExpressionValueType() != ValueTypeString);
        if (expr2.IsConstExpression())
            stat2 = GET_CONSTEXPR_INT_VALUE_AS_CLRMOV(expr2);
        else if (expr2.IsVariableExpression() && expr2.GetExpressionValueType() == ValueTypeInteger)
            stat2 = "\tMOV\t" + expr2.GetVariableExpressionDecoratedName() + ", ";
        else
        {
            AddLine(stat1 + "-(SP)");  // PUSH
            stat1 = "\tMOV\t(SP)+, ";
            GenerateExpression(expr2);  // result in R0
            stat2 = "\tMOV\tR0, ";
        }
    }

    if (statement.args.size() < 3 || statement.args[2].IsEmpty())
    {
        AddLine(stat1 + "R0");
        AddLine(stat2 + "R1");
        AddLine("\tMOV\t#-1, R2");
    }
    else
    {
        ExpressionModel& expr3 = statement.args[2];
        assert(expr3.GetExpressionValueType() != ValueTypeString);
        if (expr3.IsConstExpression())
        {
            AddLine(stat1 + "R0");
            AddLine(stat2 + "R1");
            AddLine(GET_CONSTEXPR_INT_VALUE_AS_CLRMOV(expr3) + "R2");
        }
        else if (expr3.IsVariableExpression() && expr3.GetExpressionValueType() == ValueTypeInteger)
        {
            AddLine(stat1 + "R0");
            AddLine(stat2 + "R1");
            AddLine("\tMOV\t" + expr3.GetVariableExpressionDecoratedName() + ", R2");
        }
        else
        {
            AddLine(stat2 + "-(SP)");  // PUSH
            GenerateExpression(expr3);  // result in R0
            AddLine("\tMOV\tR0, R2");
            AddLine("\tMOV\t(SP)+, R1");  // POP R1
            AddLine(stat1 + "R0");
        }
    }

    AddRuntimeCall(RuntimeCOLR, "COLOR");
}

void Generator::GenerateData(StatementModel& statement)
{
    //TODO
    AddComment("TODO DATA");
    m_notimplemented.insert(KeywordDATA);
}

void Generator::GenerateDim(StatementModel&)
{
    // Nothing to generate, DIM variables declared in ProcessEnd()
}

void Generator::GenerateDraw(StatementModel& statement)
{
    //TODO
    AddComment("TODO DRAW");
    m_notimplemented.insert(KeywordDRAW);
}

void Generator::GenerateEnd(StatementModel&)
{
    // END generates JMP LEND, but only if END is not on the last line
    string nextlinelabel = m_source->GetNextLineLabel(m_line->linenum);
    if (nextlinelabel != "LEND")
        AddLine("\tJMP\tLEND");
}

// FOR <ПАРАМЕТР>=<АРГУМЕНТ1> TO <АРГУМЕНТ2>
void Generator::GenerateFor(StatementModel& statement)
{
    // Calculate expression for "from"
    assert(statement.args.size() > 1);
    ExpressionModel& expr1 = statement.args[0];
    assert(expr1.GetExpressionValueType() != ValueTypeString);

    assert(statement.ident.type == TokenTypeIdentifier);
    VariableExpressionModel var;
    var.name = statement.ident.text;
    string deconame = var.GetVariableDecoratedName();

    // Assign the expression to the loop variable
    GenerateAssignment(var, expr1);

    // Calculate expression for "to"
    string tovalue = "#0";
    ExpressionModel& expr2 = statement.args[1];
    assert(expr2.GetExpressionValueType() != ValueTypeString);
    if (expr2.IsConstExpression())
    {
        tovalue = "#" + std::to_string((int)std::floor(expr2.GetConstExpressionDValue())) + ".";
    }
    else if (expr2.IsVariableExpression())
    {
        //TODO: register variable
        string svalue = expr2.GetVariableExpressionDecoratedName();
        AddLine("\tMOV\t" + svalue + ", @#<R" + std::to_string(m_line->linenum) + "+2>");
    }
    else
    {
        GenerateExpression(expr2);
        if (expr2.GetExpressionValueType() == ValueTypeSingle)
            AddRuntimeCall(RuntimeFTOI, "to Integer");  // result in R0
        AddLine("\tMOV\tR0, @#<R" + std::to_string(m_line->linenum) + "+2>");  //  Save "to" value
    }

    if (statement.args.size() > 2)  // has STEP expression
    {
        ExpressionModel& expr3 = statement.args[2];
        assert(expr3.GetExpressionValueType() != ValueTypeString);
        if (expr3.IsConstExpression())
            ;  //NOTE: const STEP expression will be set in NEXT statement
        else
        {
            // Calculate expression for "step"
            GenerateExpression(expr3);
            if (expr3.GetExpressionValueType() == ValueTypeSingle)
                AddRuntimeCall(RuntimeFTOI, "to Integer");  // result in R0
            // Save "step" value
            AddLine("\tMOV\tR0, @#<S" + std::to_string(m_line->linenum) + "+2>");
        }
    }

    string nextlinelabel = m_source->GetNextLineLabel(m_line->linenum);
    AddLine("R" + std::to_string(m_line->linenum) + ":\tCMP\t" + tovalue + ", " + deconame);
    AddLine("\tBGE\t.+6\t; to loop body");
    AddLine("\tJMP\tX" + std::to_string(m_line->linenum));  // label after NEXT
}

// NEXT [<ПАРАМЕТР>[,< ПАРАМЕТР >...]]
void Generator::GenerateNext(StatementModel& statement)
{
    assert(statement.variables.size() > 0);

    for (VariableModel& variable : statement.variables)
    {
        SourceLineModel* plinefor = variable.psourceline;
        assert(plinefor != nullptr);
        StatementModel& forstatement = plinefor->statement;

        string canoname = variable.GetVariableCanonicName();
        string deconame = DecorateVariableName(canoname);
        int forlinenum = plinefor->linenum;
        assert(forlinenum > 0);
        string comment = "NEXT " + canoname;

        // Increment FOR variable by 1 or by STEP value
        //TODO: Single variable increment or STEP
        if (forstatement.args.size() < 3)
            AddLine("\tINC\t" + deconame + "\t; " + comment);
        else
        {
            ExpressionModel& forexpr3 = forstatement.args[2];
            if (forexpr3.IsConstExpression())
            {
                //TODO: Warning if Single STEP value for Integer FOR variable
                int ivalue = (int)std::floor(forexpr3.GetConstExpressionDValue());
                AddLine("\tADD\t#" + std::to_string(ivalue) + "., " + deconame + "\t; " + comment);
            }
            else
            {
                //NOTE: "#1" here will be replaced at run-time with calculated STEP value
                AddLine("S" + std::to_string(forlinenum) + ":\tADD\t#1, " + deconame + "\t; " + comment);
            }
        }

        // JMP to continue loop
        AddLine("\tJMP\tR" + std::to_string(forlinenum) + "\t; continue loop");
        // Label after NEXT
        AddLine("X" + std::to_string(forlinenum) + ":\t; FOR exit addr");
    }
}

void Generator::GenerateGosub(StatementModel& statement)
{
    string linenum = "\tCALL\tN" + std::to_string(statement.paramline);
    AddLine(linenum);
}

void Generator::GenerateGoto(StatementModel& statement)
{
    string linestr = std::to_string(statement.paramline);
    AddLine("\tJMP\tN" + linestr + "\t; GOTO " + linestr);
}

void Generator::GenerateIf(StatementModel& statement)
{
    assert(statement.args.size() > 0);
    const ExpressionModel& expr = statement.args[0];
    assert(expr.GetExpressionValueType() != ValueTypeString);
    
    if (expr.IsConstExpression())
    {
        Warning(statement.token, "Constant condition under IF.");

        int ivalue = (int)expr.GetConstExpressionDValue();
        if (ivalue != 0)  // TRUE - generate THEN only
        {
            if (statement.stthen == nullptr)  // THEN linenum
            {
                int linenum = (int)statement.params[0].dvalue;
                AddLine("\tJMP\tN" + std::to_string(linenum) + "\t; THEN " + std::to_string(linenum));
            }
            else  // Statement under THEN
            {
                StatementModel* pstthen = statement.stthen;
                if (pstthen->token.keyword == KeywordGOTO)  // THEN GOTO linenum
                {
                    int linenum = (int)pstthen->paramline;
                    AddLine("\tJMP\tN" + std::to_string(linenum) + "\t; THEN GOTO");
                }
                else
                {
                    AddComment("THEN");
                    GenerateStatement(*pstthen);
                }
            }
        }
        else  // FALSE - generate ELSE only
        {
            if (statement.stelse == nullptr)  // ELSE linenum
            {
                if (statement.params.size() == 1)
                    AddLine("\t\t; ELSE do nothing");
                else
                {
                    int linenum2 = (int)statement.params[1].dvalue;
                    AddLine("\tJMP\tN" + std::to_string(linenum2) + "\t; ELSE " + std::to_string(linenum2));
                }
                //TODO
            }
            else  // Statement under ELSE
            {
                StatementModel* pstelse = statement.stelse;
                if (pstelse->token.keyword == KeywordGOTO)  // THEN GOTO linenum
                {
                    int linenum = (int)pstelse->paramline;
                    AddLine("\tJMP\tN" + std::to_string(linenum) + "\t; ELSE GOTO");
                }
                else
                {
                    AddComment("ELSE");
                    GenerateStatement(*pstelse);
                }
            }
        }
        return;
    }

    bool haveelse = (statement.stelse != nullptr) || (statement.params.size() >= 2);
    string labelelse = GetNextLocalLabel();  // local label for ELSE branch
    string labelend = GetNextLocalLabel();  // local label for end of IF statement address

    GenerateExpression(expr);
    if (expr.GetExpressionValueType() == ValueTypeSingle)
        AddLine("\tTST\t(SP)");  // check float value high word for 0
    // set flags: Z=0 for TRUE, Z=1 for FALSE
    AddLine("\tBEQ\t" + (haveelse ? labelelse : labelend));
    AddComment("THEN");

    if (statement.stthen == nullptr)  // no THEN statement, so it's THEN linenum
    {
        assert(statement.params.size() >= 1);
        int linenum1 = (int)std::floor(statement.params[0].dvalue);  // THEN line number
        AddLine("\tJMP\tN" + std::to_string(linenum1) + "\t; THEN " + std::to_string(linenum1));
    }
    else  // have THEN statement
    {
        StatementModel* pstthen = statement.stthen;
        GenerateStatement(*pstthen);
        if (haveelse)
            AddLine("\tBR\t" + labelend);
    }

    if (haveelse)
        AddLine(labelelse + ":\t; ELSE");

    if (statement.stelse == nullptr)  // no ELSE, or ELSE linenum
    {
        if (statement.params.size() >= 2)  // ELSE linenum
        {
            int linenum2 = (int)std::floor(statement.params[1].dvalue);
            AddLine("\tJMP\tN" + std::to_string(linenum2) + "\t; ELSE " + std::to_string(linenum2));
        }
    }
    else  // have ELSE statement
    {
        StatementModel* pstelse = statement.stelse;
        GenerateStatement(*pstelse);
    }

    AddLine(labelend + ":\t; end IF");
}

void Generator::GenerateInput(StatementModel& statement)
{
    if (statement.params.size() > 0)  // Write the const string prompt
    {
        Token& param = statement.params[0];
        int strindex = m_source->GetConstStringIndex(param.text);
        string strdeco = "#ST" + std::to_string(strindex);
        AddLine("\tMOV\t" + strdeco + ", R0");
        AddRuntimeCall(RuntimeWRST, "PRINT the prompt");
    }

    for (auto it = std::begin(statement.variables); it != std::end(statement.variables); ++it)
    {
        ValueType vtype = it->GetValueType();
        string deconame = it->GetVariableDecoratedName();
        switch (vtype)
        {
        case ValueTypeInteger:
            AddRuntimeCall(RuntimeINPI, "input Integer");
            AddLine("\tMOV\tR0, " + deconame);
            break;
        case ValueTypeSingle:
            AddRuntimeCall(RuntimeINPF, "input Single");
            AddLine("\tMOV\t(SP)+, " + deconame + "+2");
            AddLine("\tMOV\t(SP)+, " + deconame);
            break;
        case ValueTypeString:
            AddComment("TODO INPUT " + it->name);  //TODO
            break;
        default:
            assert(false);
        }
    }
}

void Generator::GenerateOpen(StatementModel& statement)
{
    //TODO
    AddComment("TODO OPEN");
    m_notimplemented.insert(KeywordOPEN);
}

void Generator::GenerateClose(StatementModel& statement)
{
    //TODO
    AddComment("TODO CLOSE");
    m_notimplemented.insert(KeywordCLOSE);
}

void Generator::GenerateLet(StatementModel& statement)
{
    assert(statement.args.size() == 1);
    ExpressionModel& expr = statement.args[0];

    VariableExpressionModel& var = statement.varexprs[0];

    GenerateAssignment(var, expr);
}

// ON <ВЫРАЖЕНИЕ> GOTO <СПИСОК>
// ON <ВЫРАЖЕНИЕ> GOSUB <СПИСОК>
void Generator::GenerateOn(StatementModel& statement)
{
    string comment = string("ON..") + (statement.gotogosub ? "GOTO" : "GOSUB");

    ExpressionModel& expr = statement.args[0];
    assert(expr.GetExpressionValueType() != ValueTypeString);

    GenerateExpression(expr);

    string labeltable = GetNextLocalLabel();  // local label for jump table
    string labelend = GetNextLocalLabel();  // local label for statement end address

    int numofcases = statement.params.size();
    AddLine("\tDEC\tR0");
    AddLine("\tBMI\t" + labelend);
    AddLine("\tCMP\tR0, #" + std::to_string(numofcases) + ".");
    AddLine("\tBGE\t" + labelend);
    AddLine("\tASL\tR0");
    AddLine("\tMOV\t" + labeltable + "(R0), R0\t; get jump addr");
    if (!statement.gotogosub)
        AddLine("\tMOV\t#" + labelend + ", -(SP)\t; return address");
    AddLine("\tJMP\t@R0\t; " + comment);
    int linenum = (int)statement.params[0].dvalue;
    AddLine(labeltable + ":\t; " + comment + " jump table");
    for (auto it = std::begin(statement.params); it != std::end(statement.params); ++it)
    {
        linenum = (int)it->dvalue;
        AddLine("\t.WORD\tN" + std::to_string(linenum));
    }
    AddLine(labelend + ":\t; " + comment + " end");
}

// LOCATE [<АРГ1>][,<АРГ2>][,<АРГ3>]
void Generator::GenerateLocate(StatementModel& statement)
{
    assert(statement.args.size() > 0);

    // 1st and 2nd arguments: column and row, same as for PRINT AT(col,row)
    //NOTE: any of the arguments could be missing
    const ExpressionModel& expr1 = statement.args[0];  // column, could be empty
    assert(expr1.GetExpressionValueType() != ValueTypeString);

    // First case: both 1st and 2nd arguments are present - just call AT(col,row)
    if (statement.args.size() >= 2 && (!expr1.IsEmpty() && !statement.args[1].IsEmpty()))
    {
        string stat1;
        if (expr1.IsConstExpression())
            stat1 = GET_CONSTEXPR_INT_VALUE_AS_CLRMOV(expr1);
        else if (expr1.IsVariableExpression() && expr1.GetExpressionValueType() == ValueTypeInteger)
            stat1 = "\tMOV\t" + expr1.GetVariableExpressionDecoratedName() + ", ";
        else
        {
            GenerateExpression(expr1);
            stat1 = "\tMOV\tR0, ";
        }

        const ExpressionModel& expr2 = statement.args[1];  // row, not empty
        assert(expr2.GetExpressionValueType() != ValueTypeString);
        if (expr2.IsConstExpression())
        {
            AddLine(stat1 + "R1\t; column");  // column -> R1
            GET_CONSTEXPR_INT_VALUE_IN_R0(expr2)
        }
        else if (expr2.IsVariableExpression() && expr2.GetExpressionValueType() == ValueTypeInteger)
        {
            AddLine(stat1 + "R1\t; column");  // column -> R1
            string svalue = expr2.GetVariableExpressionDecoratedName();
            AddLine("\tMOV\t" + svalue + ", R0\t; row");
        }
        else
        {
            AddLine(stat1 + "-(SP)\t; PUSH column");
            GenerateExpression(expr2);  // R0
            AddLine("\tMOV\t(SP)+, R1\t; POP R1 column");  // column -> R1
        }

        // R1 = column, R0 = row
        AddRuntimeCall(RuntimeWRAT, "LOCATE");
    }
    // Second case: 1st argument present, no 2nd argument
    else if (statement.args.size() >= 1 && !expr1.IsEmpty() &&
        (statement.args.size() == 1 || statement.args[1].IsEmpty()))
    {
        AddRuntimeCall(RuntimeGETCR, "get cursor pos");  // R1 = column, R2 = row
        if (expr1.IsConstExpression())
        {
            GET_CONSTEXPR_INT_VALUE_IN_R1(expr1)  // column -> R1
            AddLine("\tMOV\tR2, R0\t; row");  // row -> R0
        }
        else if (expr1.IsVariableExpression() && expr1.GetExpressionValueType() == ValueTypeInteger)
        {
            AddLine("\tMOV\t" + expr1.GetVariableExpressionDecoratedName() + ", R1\t; column");
            AddLine("\tMOV\tR2, R0\t; row");  // row -> R0
        }
        else
        {
            AddLine("\tMOV\tR2, -(SP)\t; PUSH row");
            GenerateExpression(expr1);  // result in R0
            AddLine("\tMOV\tR0, R1\t; column");
            AddLine("\tMOV\t(SP)+, R0\t; POP R0 row");  // row -> R0
        }

        // R1 = column, R0 = row
        AddRuntimeCall(RuntimeWRAT, "LOCATE");
    }
    // Third case: no 1st argument, 2nd argument present
    else if (statement.args.size() >= 2 && expr1.IsEmpty() && !statement.args[1].IsEmpty())
    {
        const ExpressionModel& expr2 = statement.args[1];  // row
        assert(expr2.GetExpressionValueType() != ValueTypeString);

        AddRuntimeCall(RuntimeGETCR, "get cursor pos");  // R1 = column, R2 = row
        if (expr2.IsConstExpression())
        {
            GET_CONSTEXPR_INT_VALUE_IN_R0(expr2)  // row -> R0
        }
        else if (expr2.IsVariableExpression() && expr2.GetExpressionValueType() == ValueTypeInteger)
        {
            AddLine("\tMOV\t" + expr2.GetVariableExpressionDecoratedName() + ", R0\t; row");
        }
        else
        {
            AddLine("\tMOV\tR1, -(SP)\t; PUSH column");
            GenerateExpression(expr2);  // result in R0 = row
            AddLine("\tMOV\t(SP)+, R1\t; POP R1 column");  // column -> R1
        }

        // R1 = column, R0 = row
        AddRuntimeCall(RuntimeWRAT, "LOCATE");
    }
    // Last case: no 1st, no 2nd argument
    else
    {
        assert(statement.args.size() == 3);
        assert(expr1.IsEmpty());
        assert(statement.args[1].IsEmpty());
        // skip WRAT call: no need
    }

    // 3rd argument - LOCATE cursor on/off
    if (statement.args.size() > 2 && !statement.args[2].IsEmpty())
    {
        const ExpressionModel& expr3 = statement.args[2];  // on/off, could be empty
        assert(expr3.GetExpressionValueType() != ValueTypeString);

        GenerateExpression(expr3);
        
        AddRuntimeCall(RuntimeCURSR, "show/hide cursor");
    }
}

// PSET [ @  ](<АРГ1>,< АРГ2>)[,< АРГ3>]
// PSET [STEP](<АРГ1>,< АРГ2>)[,< АРГ3>]
void Generator::GeneratePset(StatementModel& statement)
{
    assert(statement.args.size() >= 2);

    ExpressionModel& expr1 = statement.args[0];  // X
    assert(expr1.GetExpressionValueType() != ValueTypeString);
    ExpressionModel& expr2 = statement.args[1];  // Y
    assert(expr2.GetExpressionValueType() != ValueTypeString);

    GenerateExpression(expr1);

    GenerateExpression(expr2);

    if (statement.args.size() >= 3)
    {
        ExpressionModel& expr3 = statement.args[0];  // color
        assert(expr3.GetExpressionValueType() != ValueTypeString);

        GenerateExpression(expr3);
    }

    //TODO
    AddComment("TODO PSET");
    m_notimplemented.insert(KeywordPSET);
}

// PRESET [ @  ](<АРГ1>,< АРГ2>)[,< АРГ3>]
// PRESET [STEP](<АРГ1>,< АРГ2>)[,< АРГ3>]
void Generator::GeneratePreset(StatementModel& statement)
{
    assert(statement.args.size() >= 2);

    ExpressionModel& expr1 = statement.args[0];  // X
    assert(expr1.GetExpressionValueType() != ValueTypeString);
    ExpressionModel& expr2 = statement.args[1];  // Y
    assert(expr2.GetExpressionValueType() != ValueTypeString);

    GenerateExpression(expr1);

    GenerateExpression(expr2);

    if (statement.args.size() >= 3)
    {
        ExpressionModel& expr3 = statement.args[0];  // color
        assert(expr3.GetExpressionValueType() != ValueTypeString);

        GenerateExpression(expr3);
    }

    //TODO
    AddComment("TODO PRESET");
    m_notimplemented.insert(KeywordPRESET);
}

// POKE <АДРЕС>,<ВЫРАЖЕНИЕ>
void Generator::GeneratePoke(StatementModel& statement)
{
    assert(statement.args.size() == 2);

    ExpressionModel& expr1 = statement.args[0];  // address
    assert(expr1.GetExpressionValueType() != ValueTypeString);

    string stat1;
    if (expr1.IsConstExpression())
        stat1 = GET_CONSTEXPR_INT_VALUE_AS_CLRMOV(expr1);
    else if (expr1.IsVariableExpression() && expr1.GetExpressionValueType() == ValueTypeInteger)
        stat1 = "\tMOV\t" + expr1.GetVariableExpressionDecoratedName() + ", ";
    else
    {
        GenerateExpression(expr1);
        stat1 = "\tMOV\tR0, ";
    }

    ExpressionModel& expr2 = statement.args[1];  // value
    assert(expr2.GetExpressionValueType() != ValueTypeString);

    string stat2;
    if (expr2.IsConstExpression())
    {
        AddLine(stat1 + "R1");  // address -> R1
        stat2 = GET_CONSTEXPR_INT_VALUE_AS_CLRMOV(expr2);
    }
    else if (expr2.IsVariableExpression() && expr2.GetExpressionValueType() == ValueTypeInteger)
    {
        AddLine(stat1 + "R1");  // address -> R1
        stat2 = "\tMOV\t" + expr2.GetVariableExpressionDecoratedName() + ", ";
    }
    else
    {
        AddLine(stat1 + "-(SP)\t; PUSH address");
        GenerateExpression(expr2);  // result in R0
        stat2 = "\tMOV\tR0, ";
        AddLine("\tMOV\t(SP)+, R1\t; POP R1");  // address -> R1
    }

    AddLine(stat2 + "(R1)\t; POKE");
}

// OUT <АДРЕС>,<МАСКА>,<КОД>
void Generator::GenerateOut(StatementModel& statement)
{
    assert(statement.args.size() == 3);

    ExpressionModel& expr1 = statement.args[1];  // address
    assert(expr1.GetExpressionValueType() != ValueTypeString);

    if (expr1.IsConstExpression() && expr1.GetConstExpressionDValue() == 0)
    {
        Warning(statement.token, "OUT mask is 0, reduced to no operation; consider to remove the OUT statement");
        AddLine("\t; OUT mask is 0, reduced to no operation");
        return;
    }

    ExpressionModel& expr2 = statement.args[0];  // mask
    assert(expr2.GetExpressionValueType() != ValueTypeString);

    string stat2;
    if (expr2.IsConstExpression())
        stat2 = GET_CONSTEXPR_INT_VALUE_AS_CLRMOV(expr2);
    else if (expr2.IsVariableExpression() && expr2.GetExpressionValueType() == ValueTypeInteger)
        stat2 = "\tMOV\t" + expr2.GetVariableExpressionDecoratedName() + ", ";
    else
    {
        GenerateExpression(expr2);
        stat2 = "\tMOV\tR0, ";
    }

    ExpressionModel& expr3 = statement.args[2];  // code: 0 = BIC, else BIS
    assert(expr3.GetExpressionValueType() != ValueTypeString);

    if (expr3.IsConstExpression())
    {
        string operation = expr3.GetConstExpressionDValue() == 0 ? "BIC" : "BIS";

        if (expr1.IsConstExpression())
        {
            int ivalue1 = (int)std::floor(expr1.GetConstExpressionDValue());
            AddLine(stat2 + "R1");  // mask -> R1
            AddLine("\t" + operation + "\tR1, @#" + std::to_string(ivalue1) + "\t; OUT");
        }
        else if (expr1.IsVariableExpression() && expr1.GetExpressionValueType() == ValueTypeInteger)
        {
            AddLine(stat2 + "R1");  // mask -> R1
            AddLine("\t" + operation + "\tR1, @" + expr1.GetVariableExpressionDecoratedName() + "\t; OUT");
        }
        else
        {
            AddLine(stat2 + "-(SP)\t; PUSH mask");
            GenerateExpression(expr2);  // result in R0
            AddLine("\tMOV\t(SP)+, R1\t; POP R1 mask");  // mask -> R1
            AddLine("\t" + operation + "\tR1, (R0)\t; OUT");
        }
    }
    else
    {
        AddLine(stat2 + "-(SP)\t; PUSH mask");
        string stat1;
        if (expr1.IsConstExpression())
            stat1 = GET_CONSTEXPR_INT_VALUE_AS_CLRMOV(expr1);
        else if (expr1.IsVariableExpression() && expr1.GetExpressionValueType() == ValueTypeInteger)
            stat1 = "\tMOV\t" + expr1.GetVariableExpressionDecoratedName() + ", ";
        else
        {
            GenerateExpression(expr1);  // result in R0
            stat1 = "\tMOV\tR0, ";
        }

        if (expr3.IsVariableExpression())
        {
            AddLine(stat1 + "R2");  // address -> R2
            AddLine("\tTST\t" + expr3.GetVariableExpressionDecoratedName());
        }
        else
        {
            AddLine(stat1 + "-(SP)\t; PUSH address");
            GenerateExpression(expr3);  // result in R0
            AddLine("\tMOV\t(SP)+, R2\t; POP R2 address");  // address -> R2
            AddLine("\tTST\tR0");
        }
        AddLine("\tBEQ\t.+4");
        AddLine("\tBIS\t(SP)+, (R2)\t; OUT BIS");
        AddLine("\tBR\t.+2");
        AddLine("\tBIC\t(SP)+, (R2)\t; OUT BIC");
    }
}

void Generator::GenerateLine(StatementModel& statement)
{
    //TODO
    AddComment("TODO LINE");  //TODO
    m_notimplemented.insert(KeywordLINE);
}

void Generator::GenerateCircle(StatementModel& statement)
{
    //TODO
    AddComment("TODO CIRCLE");  //TODO
    m_notimplemented.insert(KeywordCIRCLE);
}

void Generator::GeneratePaint(StatementModel& statement)
{
    //TODO
    AddComment("TODO PAINT");  //TODO
    m_notimplemented.insert(KeywordPAINT);
}

void Generator::GeneratePrint(StatementModel& statement)
{
    for (auto it = std::begin(statement.args); it != std::end(statement.args); ++it)
    {
        const ExpressionModel& expr = *it;
        assert(!it->IsEmpty());
        const ExpressionNode& root = expr.nodes[expr.root];
        if (root.token.IsKeyword(KeywordAT))  // AT(col,row)
        {
            GeneratePrintAt(expr);
        }
        else if (root.token.IsKeyword(KeywordTAB))  // TAB(pos)
        {
            assert(root.args.size() == 1);
            const ExpressionModel& expr1 = root.args[0];
            GenerateExpression(expr1);
            AddRuntimeCall(RuntimeWRTAB, "PRINT tab");
        }
        else if (root.token.IsKeyword(KeywordSPC))  // SPC(num)
        {
            assert(root.args.size() == 1);
            const ExpressionModel& expr1 = root.args[0];
            if (expr1.IsConstExpression() && (int)expr1.GetConstExpressionDValue() <= 0)
                ;  // skip SPC(0) or SPC(-1)
            else
            {
                GenerateExpression(expr1);
                AddRuntimeCall(RuntimeWRSPC, "PRINT spaces");
            }
        }
        else if (root.vtype == ValueTypeString)
        {
            GeneratePrintString(expr);
        }
        else if (root.vtype == ValueTypeInteger)
        {
            GenerateExpression(expr);
            AddRuntimeCall(RuntimeWRINT, "PRINT Integer");
        }
        else if (root.vtype == ValueTypeSingle)
        {
            GenerateExpression(expr);
            AddRuntimeCall(RuntimeWRSNG, "PRINT Single");
        }
        else if (root.token.IsComma())  // special expression with Comma as root
        {
            AddRuntimeCall(RuntimeWRCOM, "PRINT comma");
        }
    }
 
    // CR/LF at end of PRINT
    if (!statement.nocrlf)
        AddRuntimeCall(RuntimeWREOL);
}

void Generator::GeneratePrintAt(const ExpressionModel& expr)
{
    const ExpressionNode& root = expr.nodes[expr.root];
    assert(root.args.size() == 2);

    const ExpressionModel& expr1 = root.args[0];  // column
    assert(expr1.GetExpressionValueType() != ValueTypeString);

    string stat1;
    if (expr1.IsConstExpression())
        stat1 = GET_CONSTEXPR_INT_VALUE_AS_CLRMOV(expr1);
    else if (expr1.IsVariableExpression() && expr1.GetExpressionValueType() == ValueTypeInteger)
        stat1 = "\tMOV\t" + expr1.GetVariableExpressionDecoratedName() + ", ";
    else
    {
        GenerateExpression(expr1);
        stat1 = "\tMOV\tR0, ";
    }

    const ExpressionModel& expr2 = root.args[1];  // row
    assert(expr2.GetExpressionValueType() != ValueTypeString);

    if (expr2.IsConstExpression())
    {
        AddLine(stat1 + "R1");  // column -> R1
        GET_CONSTEXPR_INT_VALUE_IN_R0(expr2)
    }
    else if (expr2.IsVariableExpression() && expr2.GetExpressionValueType() == ValueTypeInteger)
    {
        AddLine(stat1 + "R1");  // column -> R1
        string svalue = expr2.GetVariableExpressionDecoratedName();
        AddLine("\tMOV\t" + svalue + ", R0");
    }
    else
    {
        AddLine(stat1 + "-(SP)\t; PUSH column");
        GenerateExpression(expr2);  // R0
        AddLine("\tMOV\t(SP)+, R1\t; POP R1");  // column -> R1
    }

    // R1 = column, R0 = row
    AddRuntimeCall(RuntimeWRAT, "PRINT AT");
}

void Generator::GeneratePrintString(const ExpressionModel& expr)
{
    assert(expr.GetExpressionValueType() == ValueTypeString);
    assert(!expr.IsEmpty());
    const ExpressionNode& root = expr.nodes[expr.root];

    // Const string
    if (root.constval)
    {
        string svalue = root.token.svalue;

        if (svalue.empty())
            return;  // Empty string, nothing to print

        if (svalue.length() == 1)  // one-char string, no use of const string
        {
            //NOTE: char to int conversion depends on encoding
            char ch = svalue[0];
            string line = "\tMOV\t#" + std::to_string((unsigned char)ch) + "., R0";
            if (ch >= ' ' && ch <= 127) line += string("\t; '") + ch + "'";
            AddLine(line);
            AddRuntimeCall(RuntimeWRCH, "PRINT char");
            return;
        }

        int sindex = m_source->GetConstStringIndex(svalue);
        if (sindex < 0)
        {
            Error("Failed to find index for const string \"" + svalue + "\".");
            return;
        }

        AddLine("\tMOV\t#ST" + std::to_string(sindex) + ", R0");
        AddRuntimeCall(RuntimeWRST, "PRINT string");
        return;
    }

    // Variable
    if (root.token.type == TokenTypeIdentifier)
    {
        string deconame = DecorateVariableName(GetCanonicVariableName(root.token.text));
        AddLine("\tMOV\t#" + deconame + ", R0");
        AddRuntimeCall(RuntimeWRST, "PRINT string");
        return;
    }

    //TODO
    AddComment("TODO PRINT string expression");
}

void Generator::GenerateRead(StatementModel& statement)
{
    //TODO
    AddComment("TODO READ");
    m_notimplemented.insert(KeywordREAD);
}

void Generator::GenerateRem(StatementModel& statement)
{
    // Do nothing
}

void Generator::GenerateRestore(StatementModel& statement)
{
    //TODO
    AddComment("TODO RESTORE");
    m_notimplemented.insert(KeywordRESTORE);
}

void Generator::GenerateReturn(StatementModel& statement)
{
    AddLine("\tRETURN");
}

void Generator::GenerateScreen(StatementModel& statement)
{
    AddComment("SCREEN statement is ignored");
    Warning(statement.token, "SCREEN statement is ignored");
}

void Generator::GenerateStop(StatementModel& statement)
{
    AddLine(m_line->linenum == 0 ? string("\tCLR\tR0") : string("\tMOV\t#" + std::to_string(m_line->linenum) + "., R0"));
    AddLine("\tMOV\t#" + std::to_string(m_line->srclinenum) + "., R1");
    AddRuntimeCall(RuntimeSTOP, "STOP");
}

void Generator::GenerateWidth(StatementModel& statement)
{
    AddComment("WIDTH statement is ignored");
    Warning(statement.token, "WIDTH statement is ignored");
}


// Operation generation //////////////////////////////////////////////

void Generator::GenerateOperPlus(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    const string comment = "\t; Operation \'+\'";

    // String operands
    if (nodeleft.vtype == ValueTypeString && noderight.vtype == ValueTypeString)
    {
        //TODO
        AddComment("TODO String + String");
        return;
    }

    assert(nodeleft.vtype != ValueTypeString);
    assert(noderight.vtype != ValueTypeString);

    // Single operands
    if (nodeleft.vtype == ValueTypeSingle || noderight.vtype == ValueTypeSingle)
    {
        GenerateExpression(expr, nodeleft);
        if (nodeleft.vtype == ValueTypeInteger)
            AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack

        GenerateExpression(expr, noderight);
        if (noderight.vtype == ValueTypeInteger)
            AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack

        AddRuntimeCall(RuntimeFADD, "Operation \'+\'");  // result on stack
        return;
    }

    GenerateExpression(expr, nodeleft);  // result in R0

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

    AddLine("\tMOV\tR0, -(SP)\t; PUSH R0");
    GenerateExpression(expr, noderight);
    AddLine("\tADD\t(SP)+, R0" + comment);  // POP & ADD
}

void Generator::GenerateOperMinus(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    assert(nodeleft.vtype != ValueTypeString);
    assert(noderight.vtype != ValueTypeString);

    const string comment = "\t; Operation \'-\'";

    // Single operands
    if (nodeleft.vtype == ValueTypeSingle || noderight.vtype == ValueTypeSingle)
    {
        GenerateExpression(expr, nodeleft);
        if (nodeleft.vtype == ValueTypeInteger)
            AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack

        GenerateExpression(expr, noderight);
        if (noderight.vtype == ValueTypeInteger)
            AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack

        AddRuntimeCall(RuntimeFSUB, "Operation \'-\'");  // result on stack
        return;
    }

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

    AddLine("\tMOV\tR0, -(SP)\t; PUSH R0");
    GenerateExpression(expr, noderight);
    AddLine("\tMOV\tR0, R1");
    AddLine("\tMOV\t(SP)+, R0\t; POP R0");
    AddLine("\tSUB\tR1, R0" + comment);
}

void Generator::GenerateOperMul(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    assert(nodeleft.vtype != ValueTypeString);
    assert(noderight.vtype != ValueTypeString);

    const string comment = "\t; Operation \'*\'";

    // Single operands
    if (nodeleft.vtype == ValueTypeSingle || noderight.vtype == ValueTypeSingle)
    {
        GenerateExpression(expr, nodeleft);
        if (nodeleft.vtype == ValueTypeInteger)
            AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack

        GenerateExpression(expr, noderight);
        if (noderight.vtype == ValueTypeInteger)
            AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack

        AddRuntimeCall(RuntimeFMUL, "Operation \'*\'");  // result on stack
        return;
    }

    assert(nodeleft.vtype == ValueTypeInteger);
    assert(noderight.vtype == ValueTypeInteger);

    if (noderight.constval && noderight.vtype == ValueTypeInteger)
    {
        int ivalue = noderight.GetConstIntegerValue();

        // Special case for some const values
        switch (ivalue)
        {
        case -1:
            GenerateExpression(expr, nodeleft);  // result in R0
            AddLine("\tNEG\tR0\t; *-1");
            return;
        case 0:
            AddLine("\tCLR\tR0\t; *0");
            Warning(noderight.token, "Multiplication by 0 reduced to 0, consider to remove the multiplication.");
            return;
        case 1:
            GenerateExpression(expr, nodeleft);  // result in R0
            Warning(noderight.token, "Multiplication by 1 reduced to nothing, consider to remove the multiplication.");
            return;
        }

        GenerateExpression(expr, nodeleft);
        AddLine("\tMOV\t#" + std::to_string(ivalue) + "., R1");
        AddRuntimeCall(RuntimeIMUL, "Operation \'*\'");  // result in R0
        return;
    }

    GenerateExpression(expr, nodeleft);  // result in R0
    AddLine("\tMOV\tR0, -(SP)\t; PUSH R0");
    GenerateExpression(expr, noderight);
    AddLine("\tMOV\t(SP)+, R1");
    AddRuntimeCall(RuntimeIMUL, "Operation \'*\'");  // result in R0
}

// result is Single
void Generator::GenerateOperDiv(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    assert(nodeleft.vtype != ValueTypeString);
    assert(noderight.vtype != ValueTypeString);

    if (noderight.constval && noderight.token.dvalue == 0.0)
    {
        std::cerr << "ERROR in expression at " << node.token.line << ":" << node.token.pos << " - Division by 0." << std::endl;
        m_line->error = true;
        RegisterError();
        return;
    }

    GenerateExpression(expr, nodeleft);
    if (nodeleft.vtype == ValueTypeInteger)
        AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack

    GenerateExpression(expr, noderight);
    if (noderight.vtype == ValueTypeInteger)
        AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack

    AddRuntimeCall(RuntimeFDIV, "Operation \'/\'");  // result on stack
}

// resuilt is Integer
void Generator::GenerateOperDivInt(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    assert(nodeleft.vtype != ValueTypeString);
    assert(noderight.vtype != ValueTypeString);

    if (noderight.constval && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle))
    {
        int ivalue = (int)std::floor(noderight.token.dvalue);
        
        // Special case for some const values
        switch (ivalue)
        {
        case -4:
            GenerateExpression(expr, nodeleft);  // result in R0
            AddLine("\tNEG\tR0\t; * -1");
            AddLine("\tASR\tR0");
            AddLine("\tASR\tR0\t; / 4");
            return;
        case -2:
            GenerateExpression(expr, nodeleft);  // result in R0
            AddLine("\tNEG\tR0\t; * -1");
            AddLine("\tASR\tR0\t; / 2");
            return;
        case -1:
            GenerateExpression(expr, nodeleft);  // result in R0
            AddLine("\tNEG\tR0\t; / -1");
            return;
        case 0:
            std::cerr << "ERROR in expression at " << node.token.line << ":" << node.token.pos << " - Didiver is zero." << std::endl;
            m_line->error = true;
            RegisterError();
            return;
        case 1:
            GenerateExpression(expr, nodeleft);  // result in R0
            Warning(noderight.token, "Division by 1 reduced to nothing, consider to remove the Division.");
            return;
        case 2:
            GenerateExpression(expr, nodeleft);  // result in R0
            AddLine("\tASR\tR0\t; / 2");
            return;
        case 4:
            GenerateExpression(expr, nodeleft);  // result in R0
            AddLine("\tASR\tR0");
            AddLine("\tASR\tR0\t; / 4");
            return;
        case 8:
            GenerateExpression(expr, nodeleft);  // result in R0
            if (g_platform == PlatformUKNC)  // no EIS
                AddLine("\tASH\t#-3, R0\t; / 8.");
            else  // no EIS
            {
                AddLine("\tASR\tR0");
                AddLine("\tASR\tR0");
                AddLine("\tASR\tR0\t; / 8.");
            }
            return;
        //TODO: Special cases: divide by 16/32/64
        }

        // Const expression at right
        GenerateExpression(expr, nodeleft);  // result in R0
        AddLine("\tMOV\tR0, R1");
        AddLine("\tMOV\t#" + std::to_string(ivalue) + "., R0");
    }
    else if (noderight.token.type == TokenTypeIdentifier && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle))
    {
        // Special case for variable at right
        string deconame = DecorateVariableName(GetCanonicVariableName(noderight.token.text));
        GenerateExpression(expr, nodeleft);  // result in R0
        AddLine("\tMOV\tR0, R1");
        AddLine("\tMOV\t" + deconame + ", R1");
    }
    else
    {
        GenerateExpression(expr, nodeleft);  // result in R0
        AddLine("\tMOV\tR0, -(SP)\t; PUSH R0");
        GenerateExpression(expr, noderight);  // result in R0
        AddLine("\tMOV\t(SP)+, R1\t; POP R1");
    }

    //TODO: Special cases for const/variable expressions at left

    AddRuntimeCall(RuntimeIDIV, "Integer division");  // DIV result in R0, MOD in R1
}

void Generator::GenerateOperMod(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    assert(nodeleft.vtype != ValueTypeString);
    assert(noderight.vtype != ValueTypeString);

    if (noderight.constval && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle))
    {
        int ivalue = noderight.GetConstIntegerValue();
        switch (ivalue)
        {
        case 0:  // check if divider is zero
            std::cerr << "ERROR in expression at " << node.token.line << ":" << node.token.pos << " - MOD didiver is zero." << std::endl;
            m_line->error = true;
            RegisterError();
            return;
        case 1:
            Warning(node.token, "MOD 1 reduced to 0; consider to remove this MOD.");
            AddLine("\tCLR\tR0\t; MOD 1");
            return;
        case 2:
            GenerateExpression(expr, nodeleft);  // result in R0
            AddLine("\tBIC\t#177776, R0\t; MOD 2");
            return;
        case 4:
            GenerateExpression(expr, nodeleft);  // result in R0
            AddLine("\tBIC\t#177774, R0\t; MOD 4");
            return;
        case 8:
            GenerateExpression(expr, nodeleft);  // result in R0
            AddLine("\tBIC\t#177770, R0\t; MOD 8");
            return;
        case 16:
            GenerateExpression(expr, nodeleft);  // result in R0
            AddLine("\tBIC\t#177760, R0\t; MOD 16");
            return;
        //TODO: MOD by 32/64/128/256
        }

        // Const expression at right
        GenerateExpression(expr, nodeleft);  // result in R0
        AddLine("\tMOV\tR0, R1");
        AddLine("\tMOV\t#" + std::to_string(ivalue) + "., R0");
    }
    else if (noderight.token.type == TokenTypeIdentifier && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle))
    {
        // Variable at right
        string deconame = DecorateVariableName(GetCanonicVariableName(noderight.token.text));
        GenerateExpression(expr, nodeleft);  // result in R0
        AddLine("\tMOV\t" + deconame + ", R1");
    }
    else
    {
        GenerateExpression(expr, nodeleft);  // result in R0
        AddLine("\tMOV\tR0, -(SP)\t; PUSH R0");
        GenerateExpression(expr, noderight);  // result in R0
        if (noderight.vtype == ValueTypeSingle)
            AddRuntimeCall(RuntimeFTOI, "to Integer");  // result in R0
        AddLine("\tMOV\t(SP)+, R1\t; POP R1");
    }

    //TODO: Special cases for const/variable expressions at left

    AddRuntimeCall(RuntimeIDIV, "Integer division");  // DIV result in R0, MOD in R1
    AddLine("\tMOV\tR1, R0\t; MOD result");
}

void Generator::GenerateOperPower(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    assert(nodeleft.vtype != ValueTypeString);
    assert(noderight.vtype != ValueTypeString);

    const string comment = "Operation \'^\'";

    // Single ^ Integer or Integer ^ Integer => call FPWI
    if (noderight.vtype == ValueTypeInteger)
    {
        GenerateExpression(expr, nodeleft);
        if (nodeleft.vtype == ValueTypeInteger)
            AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack
        GenerateExpression(expr, noderight);  // result in R0
        AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack
        AddRuntimeCall(RuntimeFPWI, comment);  // result on stack
    }
    // Single ^ Single or Integer ^ Single => call FPWF
    else if (noderight.vtype == ValueTypeSingle)
    {
        GenerateExpression(expr, nodeleft);
        if (nodeleft.vtype == ValueTypeInteger)
            AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack
        GenerateExpression(expr, noderight);
        AddRuntimeCall(RuntimeFPWF, comment);  // result on stack
    }
    else
        assert(false);
}

void Generator::GenerateLogicOperArguments(const ExpressionModel& expr, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    // Code to calculate left sub-expression
    GenerateExpression(expr, nodeleft);

    if (nodeleft.vtype == ValueTypeInteger)  // left result in R0
    {
        if (noderight.vtype == ValueTypeInteger)  // Integer <=> Integer
        {
            if (noderight.constval)
            {
                int ivalue = (int)std::floor(noderight.token.dvalue);
                AddLine("\tCMP\tR0, #" + std::to_string(ivalue) + ".\t; compare integer to const");
            }
            else if (noderight.token.type == TokenTypeIdentifier)
            {
                string deconame = DecorateVariableName(GetCanonicVariableName(noderight.token.text));
                AddLine("\tCMP\tR0, " + deconame + "\t; compare integer to var");
            }
            else
            {
                AddLine("\tMOV\tR0, -(SP)\t; PUSH R0");
                GenerateExpression(expr, noderight);
                AddLine("\tCMP\t(SP)+, R0\t; compare integers");
            }
        }
        else if (noderight.vtype == ValueTypeSingle)  // Integer <=> Single
        {
            AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack
            GenerateExpression(expr, noderight);
            AddRuntimeCall(RuntimeFCMP, "compare floats");  // result in flags
        }
        else  // Integer <=> String
        {
            assert(noderight.vtype == ValueTypeString);
            assert(false);  // Compare Integer <=> String should be covered in validation
        }
    }
    else if (nodeleft.vtype == ValueTypeSingle)  // left result on stack
    {
        if (noderight.vtype == ValueTypeInteger)  // Single <=> Integer
        {
            GenerateExpression(expr, noderight);
            AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack
            AddRuntimeCall(RuntimeFCMP, "compare floats");  // result in flags
        }
        else if (noderight.vtype == ValueTypeSingle)  // Single <=> Single
        {
            GenerateExpression(expr, noderight);
            AddRuntimeCall(RuntimeFCMP, "compare floats");  // result in flags
        }
        else  // Single <=> String
        {
            assert(noderight.vtype == ValueTypeString);
            assert(false);  // Compare Single <=> String should be covered in validation
        }
    }
    else if (nodeleft.vtype == ValueTypeString)
    {
        if (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle)
        {
            assert(false);  // Compare String <=> Integer/Single should be covered in validation
        }
        else
        {
            assert(noderight.vtype == ValueTypeString);
            AddComment("TODO compare String to String");
            //TODO
        }
    }
}

void Generator::GenerateOperEqual(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    const string comment = "\t; Operation \'=\'";

    GenerateLogicOperArguments(expr, nodeleft, noderight);

    AddLine("\tBEQ\t.+6");
    AddLine("\tCLR\tR0\t; false");
    AddLine("\tBR\t.+6");
    AddLine("\tMOV\t#-1, R0\t; true");
}

void Generator::GenerateOperNotEqual(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    GenerateLogicOperArguments(expr, nodeleft, noderight);

    AddLine("\tBNE\t.+6\t; Operation \'<>\'");
    AddLine("\tCLR\tR0\t; false");
    AddLine("\tBR\t.+6");
    AddLine("\tMOV\t#-1, R0\t; true");
}

void Generator::GenerateOperLess(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    GenerateLogicOperArguments(expr, nodeleft, noderight);

    AddLine("\tBLT\t.+6\t; Operation \'<\'");
    AddLine("\tCLR\tR0\t; false");
    AddLine("\tBR\t.+6");
    AddLine("\tMOV\t#-1, R0\t; true");
}

void Generator::GenerateOperGreater(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    GenerateLogicOperArguments(expr, nodeleft, noderight);

    AddLine("\tBGT\t.+6\t; Operation \'>\'");
    AddLine("\tCLR\tR0\t; false");
    AddLine("\tBR\t.+6");
    AddLine("\tMOV\t#-1, R0\t; true");
}

void Generator::GenerateOperLessOrEqual(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    GenerateLogicOperArguments(expr, nodeleft, noderight);

    AddLine("\tBLE\t.+6\t; Operation \'<=\'");
    AddLine("\tCLR\tR0\t; false");
    AddLine("\tBR\t.+6");
    AddLine("\tMOV\t#-1, R0\t; true");
}

void Generator::GenerateOperGreaterOrEqual(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    GenerateLogicOperArguments(expr, nodeleft, noderight);

    AddLine("\tBGE\t.+6\t; Operation \'>=\'");
    AddLine("\tCLR\tR0\t; false");
    AddLine("\tBR\t.+6");
    AddLine("\tMOV\t#-1, R0\t; true");
}

void Generator::GenerateOperAnd(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    assert(nodeleft.vtype != ValueTypeString);
    assert(noderight.vtype != ValueTypeString);

    const string comment = "\t; Operation \'AND\'";

    // Special case: 0 AND xxx, result is 0
    if (noderight.vtype != ValueTypeString &&
        nodeleft.constval && (nodeleft.vtype == ValueTypeInteger || nodeleft.vtype == ValueTypeSingle) &&
        (int)std::floor(nodeleft.token.dvalue) == 0)
    {
        Warning(node.token, "AND operation with 0 reduced to 0; consider to remove the useless AND");
        AddLine("\tCLR\tR0\t; 0 AND xxx");
        return;
    }
    // Special case: xxx AND 0, result is 0
    if (nodeleft.vtype != ValueTypeString &&
        noderight.constval && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle) &&
        (int)std::floor(noderight.token.dvalue) == 0)
    {
        Warning(node.token, "AND operation with 0 reduced to 0; consider to remove the useless AND");
        AddLine("\tCLR\tR0\t; xxx AND 0");
        return;
    }

    // Special case: -1 AND xxx, result is xxx
    if (noderight.vtype != ValueTypeString &&
        nodeleft.constval && (nodeleft.vtype == ValueTypeInteger || nodeleft.vtype == ValueTypeSingle) &&
        (int)std::floor(nodeleft.token.dvalue) == -1)
    {
        Warning(node.token, "AND operation with -1 reduced to no operation; consider to remove the useless AND");
        GenerateExpression(expr, noderight);
        return;
    }
    // Special case: xxx AND -1, result is xxx
    if (nodeleft.vtype != ValueTypeString &&
        noderight.constval && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle) &&
        (int)std::floor(noderight.token.dvalue) == -1)
    {
        Warning(node.token, "AND operation with -1 reduced to no operation; consider to remove the useless AND");
        GenerateExpression(expr, nodeleft);
        return;
    }

    // Left part is constant, let's calculate right part first
    if (nodeleft.constval &&
        (nodeleft.vtype == ValueTypeInteger || nodeleft.vtype == ValueTypeSingle))
    {
        GenerateExpression(expr, noderight);
        int ivalue = ~(int)std::floor(nodeleft.token.dvalue);  // inverted to use with BIC
        AddLine("\tBIC\t#" + std::to_string(ivalue) + "., R0" + comment);
        return;
    }
    // Right part is constant
    if (noderight.constval &&
        (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle))
    {
        GenerateExpression(expr, nodeleft);
        int ivalue = ~(int)std::floor(noderight.token.dvalue);  // inverted to use with BIC
        AddLine("\tBIC\t#" + std::to_string(ivalue) + "., R0" + comment);
        return;
    }

    // Both right and left parts are not constant
    GenerateExpression(expr, nodeleft);  // result in R0
    AddLine("\tCOM\tR0");  // invert for BIC
    AddLine("\tMOV\tR0, -(SP)\t; PUSH");
    GenerateExpression(expr, noderight);  // result in R0
    AddLine("\tBIC\t(SP)+, R0" + comment);
}

void Generator::GenerateOperOr(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    assert(nodeleft.vtype != ValueTypeString);
    assert(noderight.vtype != ValueTypeString);

    const string comment = "\t; Operation \'OR\'";

    // Special case: -1 OR xxx, result is -1
    if (noderight.vtype != ValueTypeString &&
        nodeleft.constval && (nodeleft.vtype == ValueTypeInteger || nodeleft.vtype == ValueTypeSingle) &&
        (int)std::floor(nodeleft.token.dvalue) == -1)
    {
        Warning(node.token, "OR operation with -1 reduced to -1; consider to remove the useless OR");
        AddLine("\tMOV\t#-1, R0\t; -1 OR xxx");
        return;
    }
    // Special case: xxx OR -1, result is -1
    if (nodeleft.vtype != ValueTypeString &&
        noderight.constval && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle) &&
        (int)std::floor(noderight.token.dvalue) == -1)
    {
        Warning(node.token, "OR operation with -1 reduced to -1; consider to remove the useless OR");
        AddLine("\tMOV\t#-1, R0\t; xxx OR -1");
        return;
    }

    // Special case: 0 OR xxx, result is xxx
    if (noderight.vtype != ValueTypeString &&
        nodeleft.constval && (nodeleft.vtype == ValueTypeInteger || nodeleft.vtype == ValueTypeSingle) &&
        (int)std::floor(nodeleft.token.dvalue) == 0)
    {
        Warning(node.token, "OR operation with 0 reduced to no operation; consider to remove the useless OR");
        GenerateExpression(expr, noderight);
        return;
    }
    // Special case: xxx OR 0, result is xxx
    if (nodeleft.vtype != ValueTypeString &&
        noderight.constval && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle) &&
        (int)std::floor(noderight.token.dvalue) == 0)
    {
        Warning(node.token, "OR operation with 0 reduced to no operation; consider to remove the useless OR");
        GenerateExpression(expr, nodeleft);
        return;
    }

    // Left part is constant, let's calculate right part first
    if (nodeleft.constval &&
        (nodeleft.vtype == ValueTypeInteger || nodeleft.vtype == ValueTypeSingle))
    {
        GenerateExpression(expr, noderight);
        int ivalue = (int)std::floor(nodeleft.token.dvalue);
        AddLine("\tBIS\t#" + std::to_string(ivalue) + "., R0" + comment);
        return;
    }
    // Right part is constant
    if (noderight.constval &&
        (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle))
    {
        GenerateExpression(expr, nodeleft);
        int ivalue = (int)std::floor(noderight.token.dvalue);
        AddLine("\tBIS\t#" + std::to_string(ivalue) + "., R0" + comment);
        return;
    }

    // Both right and left parts are not constant
    GenerateExpression(expr, nodeleft);  // result in R0
    AddLine("\tMOV\tR0, -(SP)\t; PUSH");
    GenerateExpression(expr, noderight);  // result in R0
    AddLine("\tBIS\t(SP)+, R0" + comment);
}

void Generator::GenerateOperXor(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    assert(nodeleft.vtype != ValueTypeString);
    assert(noderight.vtype != ValueTypeString);

    const string comment = "\t; Operation \'XOR\'";

    // Special case: 0 XOR xxx, result is xxx
    if (noderight.vtype != ValueTypeString &&
        nodeleft.constval && (nodeleft.vtype == ValueTypeInteger || nodeleft.vtype == ValueTypeSingle) &&
        (int)std::floor(nodeleft.token.dvalue) == 0)
    {
        Warning(node.token, "XOR operation with 0 reduced to no operation; consider to remove the useless XOR");
        GenerateExpression(expr, noderight);
        return;
    }
    // Special case: xxx XOR 0, result is xxx
    if (nodeleft.vtype != ValueTypeString &&
        noderight.constval && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle) &&
        (int)std::floor(noderight.token.dvalue) == 0)
    {
        Warning(node.token, "XOR operation with 0 reduced to no operation; consider to remove the useless XOR");
        GenerateExpression(expr, nodeleft);
        return;
    }

    // Special case: -1 XOR xxx, result same as NOT xxx
    if (noderight.vtype != ValueTypeString &&
        nodeleft.constval && (nodeleft.vtype == ValueTypeInteger || nodeleft.vtype == ValueTypeSingle) &&
        (int)std::floor(nodeleft.token.dvalue) == -1)
    {
        Warning(node.token, "XOR operation with -1 reduced to inversion; consider to replace XOR with NOT");
        GenerateExpression(expr, noderight);
        AddLine("\tCOM\tR0\t; xxx XOR -1");
        return;
    }
    // Special case: xxx XOR -1, result same as NOT xxx
    if (nodeleft.vtype != ValueTypeString &&
        noderight.constval && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle) &&
        (int)std::floor(noderight.token.dvalue) == -1)
    {
        Warning(node.token, "XOR operation with -1 reduced to inversion; consider to replace XOR with NOT");
        GenerateExpression(expr, nodeleft);
        AddLine("\tCOM\tR0\t; xxx XOR -1");
        return;
    }

    // Left part is constant, let's calculate right part first
    if (nodeleft.constval &&
        (nodeleft.vtype == ValueTypeInteger || nodeleft.vtype == ValueTypeSingle))
    {
        GenerateExpression(expr, noderight);
        int ivalue = (int)std::floor(nodeleft.token.dvalue);
        AddLine("\tMOV\t#" + std::to_string(ivalue) + "., R1");
        AddLine("\tXOR\tR1, R0" + comment);  // XOR works only from register
        return;
    }
    // Right part is constant
    if (noderight.constval &&
        (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle))
    {
        GenerateExpression(expr, nodeleft);
        int ivalue = (int)std::floor(noderight.token.dvalue);
        AddLine("\tMOV\t#" + std::to_string(ivalue) + "., R1");
        AddLine("\tXOR\tR1, R0" + comment);  // XOR works only from register
        return;
    }

    // Both right and left parts are not constant
    GenerateExpression(expr, nodeleft);  // result in R0
    AddLine("\tMOV\tR0, -(SP)\t; PUSH");
    GenerateExpression(expr, noderight);  // result in R0
    AddLine("\tMOV\t(SP)+, R1\t; POP");
    AddLine("\tXOR\tR1, R0" + comment);  // XOR works only from register
}

// X EQV Y == NOT(X XOR Y)
void Generator::GenerateOperEqv(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    assert(nodeleft.vtype != ValueTypeString);
    assert(noderight.vtype != ValueTypeString);

    const string comment = "\t; Operation \'EQV\'";

    // Special case: 0 EQV xxx, result is NOT xxx
    if (noderight.vtype != ValueTypeString &&
        nodeleft.constval && (nodeleft.vtype == ValueTypeInteger || nodeleft.vtype == ValueTypeSingle) &&
        (int)std::floor(nodeleft.token.dvalue) == 0)
    {
        Warning(node.token, "EQV operation with 0 reduced to inversion; consider to replace EQV with NOT");
        GenerateExpression(expr, noderight);
        AddLine("\tCOM\tR0\t; 0 EQV xxx");
        return;
    }
    // Special case: xxx EQV 0, result is NOT xxx
    if (nodeleft.vtype != ValueTypeString &&
        noderight.constval && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle) &&
        (int)std::floor(noderight.token.dvalue) == 0)
    {
        Warning(node.token, "EQV operation with 0 reduced to inversion; consider to replace EQV with NOT");
        GenerateExpression(expr, nodeleft);
        AddLine("\tCOM\tR0\t; xxx EQV 0");
        return;
    }

    // Special case: -1 EQV xxx, result is xxx
    if (noderight.vtype != ValueTypeString &&
        nodeleft.constval && (nodeleft.vtype == ValueTypeInteger || nodeleft.vtype == ValueTypeSingle) &&
        (int)std::floor(nodeleft.token.dvalue) == -1)
    {
        Warning(node.token, "EQV operation with -1 reduced to no operation; consider to remove the useless EQV");
        GenerateExpression(expr, noderight);
        return;
    }
    // Special case: xxx EQV -1, result is xxx
    if (nodeleft.vtype != ValueTypeString &&
        noderight.constval && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle) &&
        (int)std::floor(noderight.token.dvalue) == -1)
    {
        Warning(node.token, "EQV operation with -1 reduced to no operation; consider to remove the useless EQV");
        GenerateExpression(expr, nodeleft);
        return;
    }

    // Left part is constant, let's calculate right part first
    if (nodeleft.constval &&
        (nodeleft.vtype == ValueTypeInteger || nodeleft.vtype == ValueTypeSingle))
    {
        GenerateExpression(expr, noderight);
        int ivalue = (int)std::floor(nodeleft.token.dvalue);
        AddLine("\tMOV\t#" + std::to_string(ivalue) + "., R1");
        AddLine("\tXOR\tR1, R0");  // XOR works only from register
        AddLine("\tCOM\tR0" + comment);
        return;
    }
    // Right part is constant
    if (noderight.constval &&
        (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle))
    {
        GenerateExpression(expr, nodeleft);
        int ivalue = (int)std::floor(noderight.token.dvalue);
        AddLine("\tMOV\t#" + std::to_string(ivalue) + "., R1");
        AddLine("\tXOR\tR1, R0");  // XOR works only from register
        AddLine("\tCOM\tR0" + comment);
        return;
    }

    // Both right and left parts are not constant
    GenerateExpression(expr, nodeleft);  // result in R0
    AddLine("\tMOV\tR0, -(SP)\t; PUSH");
    GenerateExpression(expr, noderight);  // result in R0
    AddLine("\tMOV\t(SP)+, R1\t; POP");
    AddLine("\tXOR\tR1, R0");  // XOR works only from register
    AddLine("\tCOM\tR0" + comment);
}


// Function generation ///////////////////////////////////////////////

// X=CINT(<АРГУМЕНТ>)
// result is Integer
void Generator::GenerateFuncCint(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(expr.GetExpressionValueType() != ValueTypeString);
    assert(node.args.size() == 1);

    const ExpressionModel& expr1 = node.args[0];
    assert(expr1.GetExpressionValueType() != ValueTypeString);

    if (expr1.GetExpressionValueType() == ValueTypeInteger)
        return;  // already Integer

    GenerateExpression(expr1);

    AddRuntimeCall(RuntimeFTOI, "to Integer");  // result in R0
}

// X=FIX(<АРИФМЕТИЧЕСКОЕ ВЫРАЖЕНИЕ>)
// result is Single
void Generator::GenerateFuncFix(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(expr.GetExpressionValueType() != ValueTypeString);
    assert(node.args.size() == 1);

    const ExpressionModel& expr1 = node.args[0];
    assert(expr1.GetExpressionValueType() != ValueTypeString);

    GenerateExpression(expr1);  // result on stack

    if (expr1.GetExpressionValueType() == ValueTypeInteger)
        AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack

    AddRuntimeCall(RuntimeFFIX, "FIX");
}

// X=INT(<АРИФМЕТИЧЕСКОЕ ВЫРАЖЕНИЕ>)
// result is Single
void Generator::GenerateFuncInt(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(expr.GetExpressionValueType() != ValueTypeString);
    assert(node.args.size() == 1);

    const ExpressionModel& expr1 = node.args[0];
    assert(expr1.GetExpressionValueType() != ValueTypeString);

    GenerateExpression(expr1);  // result on stack

    if (expr1.GetExpressionValueType() == ValueTypeInteger)
        AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack

    AddRuntimeCall(RuntimeFINT, "INT");
}

// X=ABS(<АРИФМЕТИЧЕСКОЕ ВЫРАЖЕНИЕ>)
// result is Integer if arguments is Integer
// result is Single if arguments is Single
void Generator::GenerateFuncAbs(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(node.args.size() == 1);

    const ExpressionModel& expr1 = node.args[0];
    assert(expr1.GetExpressionValueType() != ValueTypeString);

    switch (expr1.GetExpressionValueType())
    {
    case ValueTypeInteger:
        GenerateExpression(expr1);  // result in R0
        AddLine("\tBPL\t.+4");
        AddLine("\tNEG\tR0");
        return;
    case ValueTypeSingle:
        GenerateExpression(expr1);  // result on stack
        AddLine("\tBIC\t#100000, (SP)\t; ABS");  // clear sign
        return;
    default:
        assert(false);  // unexpected value type
    }
}

// X=RND(<АРИФМЕТИЧЕСКОЕ ВЫРАЖЕНИЕ>)
// result is Single
void Generator::GenerateFuncRnd(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(node.args.size() == 1);

    const ExpressionModel& expr1 = node.args[0];
    assert(expr1.GetExpressionValueType() != ValueTypeString);

    // Special case for RND(0): return RNDSAV value
    if (expr1.IsConstExpression() && std::floor(expr1.GetConstExpressionDValue()) == 0.0)
    {
        AddLine("\tMOV\tRNDSAV+2, -(SP)\t; RND(0)");
        AddLine("\tMOV\tRNDSAV, -(SP)");
        m_runtimeneeds.insert(RuntimeFRND);
        return;
    }

    GenerateExpression(expr1);
    if (expr1.GetExpressionValueType() == ValueTypeInteger)
        AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack

    AddRuntimeCall(RuntimeFRND, "random number");  // result on stack
}

// X=PEEK(<АРГУМЕНТ>)
// result is Integer
void Generator::GenerateFuncPeek(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(node.args.size() == 1);

    const string comment = "\t; PEEK";

    const ExpressionModel& expr1 = node.args[0];
    assert(expr1.GetExpressionValueType() != ValueTypeString);

    if (expr1.IsConstExpression())
    {
        int ivalue = (int)std::floor(expr1.GetConstExpressionDValue());
        AddLine("\tMOV\t@#" + std::to_string(ivalue) + "., R0" + comment);
        return;
    }
    else if (expr1.IsVariableExpression())
    {
        AddLine("\tMOV\t@" + expr1.GetVariableExpressionDecoratedName() + ", R0" + comment);
        return;
    }

    GenerateExpression(expr1);
    //TODO: For Single expression, convert to Integer
    AddLine("\tMOV\t(R0), R0" + comment);
}

// X=INP(<АДРЕС>,<МАСКА>)
// result is Integer
void Generator::GenerateFuncInp(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(node.args.size() == 2);

    const ExpressionModel& expr1 = node.args[0];
    assert(expr1.GetExpressionValueType() != ValueTypeString);
    const ExpressionModel& expr2 = node.args[1];
    assert(expr2.GetExpressionValueType() != ValueTypeString);

    //TODO: If mask is 0 then return 0 and WARN
    //TODO: If mask is 0xFFFF then same as PEEK and WARN

    //TODO: Special case for const expression and variable expression
    GenerateExpression(expr1);  // R0 = address
    //TODO: For Single expression, convert to Integer

    AddLine("\tMOV\t(R0), R1\t; INP value");  // R1 = value

    GenerateExpression(expr2);
    //TODO: For Single expression, convert to Integer
    AddLine("\tCOM\tR0");  // invert the mask

    AddLine("\tBIC\tR0, R1\t; INP mask");  // apply the mask
    AddLine("\tMOV\tR1, R0\t; INP"); // result in R0
}

// X=CSRLIN[(<АРИФМЕТИЧЕСКОЕ ВЫРАЖЕНИЕ>)]
// result is Integer
void Generator::GenerateFuncCsrlin(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(node.args.size() <= 1);

    // If we have non-const expression then calculate it
    if (node.args.size() > 0)
    {
        const ExpressionModel& expr1 = node.args[0];
        assert(expr1.GetExpressionValueType() != ValueTypeString);
        if (!expr1.IsConstExpression() && !expr1.IsVariableExpression())
            GenerateExpression(expr1);
        Warning(node.token, "CSRLIN argument calculated but value not used; consider to remove the argument");
    }

    AddRuntimeCall(RuntimeGETCR, "get cursor pos for CSRLIN");  // R1 = column, R2 = row
    AddLine("\tMOV\tR2, R0\t; row");
}

// X=POS[(<АРИФМЕТИЧЕСКОЕ ВЫРАЖЕНИЕ>)]
// result is Integer
void Generator::GenerateFuncPos(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(node.args.size() <= 1);

    // If we have non-const expression then calculate it
    if (node.args.size() > 0)
    {
        const ExpressionModel& expr1 = node.args[0];
        assert(expr1.GetExpressionValueType() != ValueTypeString);
        if (!expr1.IsConstExpression() && !expr1.IsVariableExpression())
            GenerateExpression(expr1);
        Warning(node.token, "POS argument calculated but value not used; consider to remove the argument");
    }

    AddRuntimeCall(RuntimeGETCR, "get cursor pos for POS");  // R1 = column, R2 = row
    AddLine("\tMOV\tR1, R0\t; column");
}

// X=LEN(<СИМВОЛЬНОЕ ВЫРАЖЕНИЕ>)
// result is Integer
void Generator::GenerateFuncLen(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(node.args.size() == 1);

    //TODO: Special case for const expression and variable expression
    const ExpressionModel& expr1 = node.args[0];
    assert(expr1.GetExpressionValueType() == ValueTypeString);

    GenerateExpression(expr1);  // R0 = string address

    AddLine("\tMOV\tR0, R1\t");
    AddLine("\tCLR\tR0\t");
    AddLine("\tBISB\t(R1), R0\t; LEN");  // get byte of the string length
}

// X=SQR(<АРГУМЕНТ>)
// result is Single
void Generator::GenerateFuncSqr(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(expr.GetExpressionValueType() != ValueTypeString);
    assert(node.args.size() == 1);

    const ExpressionModel& expr1 = node.args[0];
    assert(expr1.GetExpressionValueType() != ValueTypeString);

    GenerateExpression(expr1);

    if (expr1.GetExpressionValueType() == ValueTypeInteger)
        AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack

    AddRuntimeCall(RuntimeFSQR, "square root");  // result on stack
}

// X=SGN(<АРИФМЕТИЧЕСКОЕ ВЫРАЖЕНИЕ>)
// result is Single
void Generator::GenerateFuncSgn(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(expr.GetExpressionValueType() != ValueTypeString);
    assert(node.args.size() == 1);

    const ExpressionModel& expr1 = node.args[0];
    assert(expr1.GetExpressionValueType() != ValueTypeString);

    GenerateExpression(expr1);
    if (expr1.GetExpressionValueType() == ValueTypeInteger)
        AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack

    AddRuntimeCall(RuntimeFSGN, "SGN");
}

// X=CSNG(<АРИФМЕТИЧЕСКОЕ ВЫРАЖЕНИЕ>)
// result is Single
void Generator::GenerateFuncCsng(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(expr.GetExpressionValueType() != ValueTypeString);
    assert(node.args.size() == 1);

    const ExpressionModel& expr1 = node.args[0];
    assert(expr1.GetExpressionValueType() != ValueTypeString);

    GenerateExpression(expr1);

    if (expr1.GetExpressionValueType() == ValueTypeSingle)
    {
        Warning(node.token, "CSNG function call has no effect, value already has Single type.");
        return;
    }

    AddRuntimeCall(RuntimeITOF, "CSNG");  // result on stack
}

// X=SIN(<АРИФМЕТИЧЕСКОЕ ВЫРАЖЕНИЕ>)
// result is Single
void Generator::GenerateFuncSin(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(expr.GetExpressionValueType() != ValueTypeString);
    assert(node.args.size() == 1);

    const ExpressionModel& expr1 = node.args[0];
    assert(expr1.GetExpressionValueType() != ValueTypeString);

    GenerateExpression(expr1);
    if (expr1.GetExpressionValueType() == ValueTypeInteger)
        AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack

    AddRuntimeCall(RuntimeFSIN, "sin(X)");  // result on stack
}

// X=COS(<АРИФМЕТИЧЕСКОЕ ВЫРАЖЕНИЕ>)
// result is Single
void Generator::GenerateFuncCos(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(expr.GetExpressionValueType() != ValueTypeString);
    assert(node.args.size() == 1);

    const ExpressionModel& expr1 = node.args[0];
    assert(expr1.GetExpressionValueType() != ValueTypeString);

    GenerateExpression(expr1);
    if (expr1.GetExpressionValueType() == ValueTypeInteger)
        AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack

    AddRuntimeCall(RuntimeFCOS, "cos(X)");  // result on stack
}

// X=TAN(<АРИФМЕТИЧЕСКОЕ ВЫРАЖЕНИЕ>)
// result is Single
void Generator::GenerateFuncTan(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(expr.GetExpressionValueType() != ValueTypeString);
    assert(node.args.size() == 1);

    const ExpressionModel& expr1 = node.args[0];
    assert(expr1.GetExpressionValueType() != ValueTypeString);

    GenerateExpression(expr1);
    if (expr1.GetExpressionValueType() == ValueTypeInteger)
        AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack

    AddRuntimeCall(RuntimeFTAN, "tan(X)");  // result on stack
}

// X=ATN(<АРИФМЕТИЧЕСКОЕ ВЫРАЖЕНИЕ>)
// result is Single
void Generator::GenerateFuncAtn(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(expr.GetExpressionValueType() != ValueTypeString);
    assert(node.args.size() == 1);

    const ExpressionModel& expr1 = node.args[0];
    assert(expr1.GetExpressionValueType() != ValueTypeString);

    GenerateExpression(expr1);
    if (expr1.GetExpressionValueType() == ValueTypeInteger)
        AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack

    AddRuntimeCall(RuntimeFATN, "arctan(X)");  // result on stack
}

// X=EXP(<АРГУМЕНТ>)
// result is Single
void Generator::GenerateFuncExp(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(expr.GetExpressionValueType() != ValueTypeString);
    assert(node.args.size() == 1);

    const ExpressionModel& expr1 = node.args[0];
    assert(expr1.GetExpressionValueType() != ValueTypeString);

    GenerateExpression(expr1);
    if (expr1.GetExpressionValueType() == ValueTypeInteger)
        AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack

    AddRuntimeCall(RuntimeFEXP, "exp(X)");  // result on stack
}

// X=LOG(<АРГУМЕНТ>)
// result is Single
void Generator::GenerateFuncLog(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(expr.GetExpressionValueType() != ValueTypeString);
    assert(node.args.size() == 1);

    const ExpressionModel& expr1 = node.args[0];
    assert(expr1.GetExpressionValueType() != ValueTypeString);

    GenerateExpression(expr1);
    if (expr1.GetExpressionValueType() == ValueTypeInteger)
        AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack

    AddRuntimeCall(RuntimeFLOG, "log(X)");  // result on stack
}

// X¤=INKEY¤
// result is String
void Generator::GenerateFuncInkey(const ExpressionModel& expr, const ExpressionNode& node)
{
    AddRuntimeCall(RuntimeINKEY, "get input key");  // R0 = symbol or 0
    AddComment("TODO INKEY$");
    //TODO: form a string
    m_notimplemented.insert(KeywordINKEY);
}

// X=ASC(<АРГУМЕНТ>)
// result is Integer
void Generator::GenerateFuncAsc(const ExpressionModel& expr, const ExpressionNode& node)
{
    //TODO: Special case for const expression and variable expression
    const ExpressionModel& expr1 = node.args[0];
    assert(expr1.GetExpressionValueType() == ValueTypeString);

    GenerateExpression(expr1);  // R0 = string address

    AddLine("\tMOV\tR0, R1\t");
    AddLine("\tCLR\tR0\t");
    AddLine("\tBISB\t1(R1), R0\t; ASC");  // get first byte of the string
}

// X=IIF(<ЛОГИЧЕСКОЕ ВЫРАЖЕНИЕ>,<АРИФМЕТИЧЕСКОЕ ВЫРАЖЕНИЕ>,<АРИФМЕТИЧЕСКОЕ ВЫРАЖЕНИЕ>)
// result is Single or Integer
void Generator::GenerateFuncIif(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(expr.GetExpressionValueType() != ValueTypeString);
    assert(node.args.size() == 3);

    string labelfalse = GetNextLocalLabel();  // local label for false expression
    string labelend = GetNextLocalLabel();  // local label for end of IIF

    const ExpressionModel& expr1 = node.args[0];
    assert(expr1.GetExpressionValueType() != ValueTypeString);

    GenerateExpression(expr1);
    AddLine("\tBEQ\t" + labelfalse + "\t; false =>");
    AddComment("IIF true expression");

    const ExpressionModel& expr2 = node.args[1];
    assert(expr2.GetExpressionValueType() != ValueTypeString);
    GenerateExpression(expr2);
    if (expr.GetExpressionValueType() == ValueTypeSingle && expr2.GetExpressionValueType() == ValueTypeInteger)
        AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack

    AddLine("\tBR\t" + labelend);
    AddLine(labelfalse + ":\t; IIF false expression");

    const ExpressionModel& expr3 = node.args[2];
    assert(expr3.GetExpressionValueType() != ValueTypeString);
    GenerateExpression(expr3);
    if (expr.GetExpressionValueType() == ValueTypeSingle && expr3.GetExpressionValueType() == ValueTypeInteger)
        AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack

    AddLine(labelend + ":\t; end of IIF");
}


//////////////////////////////////////////////////////////////////////
