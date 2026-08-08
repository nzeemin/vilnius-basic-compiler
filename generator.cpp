
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
    { KeywordCALL,      &Generator::GenerateCall },
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
    { "IMP",            &Generator::GenerateOperImp },
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
    { KeywordCHR,       &Generator::GenerateFuncChr },
    { KeywordSTRING,    &Generator::GenerateFuncString },
    { KeywordVAL,       &Generator::GenerateFuncVal },
    { KeywordIIF,       &Generator::GenerateFuncIif },
};


// Comparison function to sort variables by decorated names
static bool CompareVariables(const VariableModel& a, const VariableModel& b)
{
    string deconamea = a.GetVariableDecoratedName();
    string deconameb = b.GetVariableDecoratedName();
    return deconamea < deconameb;
}

string to_string_octal(uint16_t value)
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
    int ivalue = ConstToInteger(expr.GetConstExpressionDValue()); \
    if (ivalue == 0) \
        AddLine("\tCLR\tR0"); \
    else \
        AddLine("\tMOV\t#" + std::to_string(ivalue) + "., R0"); \
}
// Get expression value as integer, put in register R1.
// Use only when we know expr.IsConstExpression() == true, and it can't be ValueTypeString.
#define GET_CONSTEXPR_INT_VALUE_IN_R1(expr) { \
    int ivalue = ConstToInteger(expr.GetConstExpressionDValue()); \
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
    int ivalue = ConstToInteger(expr.GetConstExpressionDValue());
    if (ivalue == 0)
        return "\tCLR\t";
    else \
        return "\tMOV\t#" + std::to_string(ivalue) + "., ";
}


//////////////////////////////////////////////////////////////////////


Generator::Generator(SourceModel* source, FinalModel* final,
        const std::vector<string>* initlines, const std::vector<string>* termlines)
    : m_source(source), m_final(final), m_initlines(initlines), m_termlines(termlines),
    m_lineindex(-1), m_line(nullptr), m_local(0), m_runtimeneeds(), m_notimplemented()
{
    assert(source != nullptr);
    assert(final != nullptr);
    assert(initlines != nullptr);
    assert(termlines != nullptr);
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

    // Copy initialization code from the runtime template
    for (const string& line : *m_initlines)
        m_final->AddLine(line);
}

void Generator::ProcessEnd()
{
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

    AddLine("LEND:");

    // Copy termination code from the runtime template
    for (const string& line : *m_termlines)
        m_final->AddLine(line);

    GenerateStrings();

    GenerateVariables();

    GenerateDataBlock();

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

void Generator::GenerateConstString(string label, string str)
{
    string strlen = std::to_string(str.length());
    if (str.length() > 7) strlen += '.';
    if (str.length() % 2 == 0) str += '\0';  // to align strings to word boundary
    // Mask special symbols, mask '/'
    std::ostringstream oss;
    if (!label.empty())
        oss << label << ":";
    oss << "\t.ASCII\t<" << strlen << ">";
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

void Generator::GenerateStrings()
{
    AddComment("STRINGS");
    AddLine("\t.EVEN");
    AddLine("ST0:\t.WORD\t0\t; empty string");

    if (m_source->conststrings.empty())
        return;

    for (size_t stno = 0; stno < m_source->conststrings.size(); ++stno)
    {
        string strdeco = "ST" + std::to_string(stno + 1);
        string& str = m_source->conststrings[stno];
        GenerateConstString(strdeco, str);
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

void Generator::GenerateDataBlock()
{
    if (m_source->data.empty())
        return;

    AddComment("DATA BLOCK");
    AddLine("\t.EVEN");

    size_t firstdatacount = 0;
    ValueType firstdatatype = ValueTypeNone;
    size_t datacount = 0;
    for (size_t i = 0; i < m_source->data.size(); i++)
    {
        const DataElementModel& dataelem = m_source->data[i];
        string label;

        if (datacount == 0)
            AddLine("D" + std::to_string(i) + ":");

        datacount++;
        if (datacount >= 8000 ||
            i == m_source->data.size() - 1 ||
            (m_source->data[i + 1].vtype != dataelem.vtype || m_source->data[i + 1].fixed))
        {
            if (firstdatacount == 0)
            {
                firstdatacount = datacount;  // for DATACN initialization
                firstdatatype = dataelem.vtype;  // for DATATY
            }

            // write data descriptor
            uint16_t descriptor = (uint16_t)((dataelem.vtype << 13) | datacount);
            AddLine("\t.WORD\t" + to_string_octal(descriptor) + "\t\t; " + GetValueTypeStr(dataelem.vtype) + " * " + std::to_string(datacount));

            string line;
            for (size_t k = 0; k < datacount; k++)
            {
                const DataElementModel& elem = m_source->data[i + 1 - datacount + k];
                switch (elem.vtype)
                {
                case ValueTypeInteger:
                    if (line.empty()) line = "\t.WORD\t"; else line += ", ";
                    line += std::to_string(ConstToInteger(elem.dvalue)) + ".";
                    if (k % 8 == 7)
                    {
                        AddLine(line);
                        line.clear();
                    }
                    break;
                case ValueTypeSingle:
                {
                    if (line.empty()) line = "\t.WORD\t"; else line += ", ";
                    uint32_t wvalue = float_to_dec_float((float)elem.dvalue);
                    line += to_string_octal(wvalue & 0xFFFF) + "," + to_string_octal(wvalue >> 16);
                    if (k % 4 == 3)
                    {
                        AddLine(line);
                        line.clear();
                    }
                    break;
                }
                case ValueTypeString:
                    GenerateConstString("", elem.svalue);
                    break;
                default:
                    assert(false);  // unexpected value type
                    break;
                }

                if (!line.empty() && k == datacount - 1)
                {
                    AddLine(line);
                    line.clear();
                }
            }

            datacount = 0;
        }
    }
    AddLine("\t.WORD\t0\t\t; End of DATA");

    if (!g_turbo8)
        AddLine("\t.GLOBL\tDATAPT, DATATY, DATACN");
    AddLine("DATAPT:\t.WORD\tD0+2\t\t; Data pointer");
    AddLine("DATATY:\t.WORD\t" + to_string_octal(firstdatatype << 13) + "\t\t; Data type");
    AddLine("DATACN:\t.WORD\t" + std::to_string(firstdatacount) + ".\t\t; Data counter");
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

    // Skip DATA lines completely, will process them in GenerateDataBlock
    if (m_line->statement.token.keyword == KeywordDATA)
        return true;

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
    std::cerr << "ERROR ";
    if (m_line->linenum == 0)
        std::cerr << "at " << m_line->srclinenum;
    else
        std::cerr << "in line " << m_line->linenum;
    std::cerr << " - " << message << std::endl;
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
            int ivalue = ConstToInteger(node.token.dvalue);
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
        {
            int sindex = m_source->GetConstStringIndex(node.token.svalue);
            AddLine("\tMOV\t#ST" + std::to_string(sindex) + ", R0\t; const String");
            return;
        }
        default:
            assert(false);  // unexpected value type
            return;
        }
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
        switch (node.vtype)
        {
        case ValueTypeSingle:
            AddLine("\tMOV\t" + deconame + ",   -(SP)\t; var " + canoname);  // lower word
            AddLine("\tMOV\t" + deconame + "+2, -(SP)");  // higher word
            break;
        case ValueTypeString:
            AddLine("\tMOV\t#" + deconame + ", R0\t; var " + canoname);
            break;
        case ValueTypeInteger:
            AddLine("\tMOV\t" + deconame + ", R0\t; var " + canoname);
            break;
        default:
            assert(false);  // unexpected value type
        }
        return;
    }

    if (node.vtype == ValueTypeString)
    {
        // The only operation with strings is '+'
        if (node.token.type == TokenTypeOperation && node.token.text == "+" && node.left >= 0 && node.right >= 0)
        {
            GenerateOperPlus(expr, node, expr.nodes[node.left], expr.nodes[node.right]);
            return;
        }

        assert(false);  // should not fall in here
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
        else if (node.token.text == "+")  // unary '+' does nothing with the operand
            GenerateExpression(expr, expr.nodes[node.right]);
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

// Generate an operand for an operation working with Integers only.
// Integer operations take the value in R0, while a Single value is left on the stack,
// so a Single operand has to be converted first, see 2.3.6 in the language description.
void Generator::GenerateOperandAsInteger(const ExpressionModel& expr, const ExpressionNode& node)
{
    GenerateExpression(expr, node);

    if (node.vtype == ValueTypeSingle)
        AddRuntimeCall(RuntimeFTOI, "to Integer");  // result in R0
}
// Same as above but for a whole sub-expression, not a single node. GetExpressionValueType
// is used and not the root node vtype: for some nodes the type is kept in token.vtype,
// see GetExpressionValueType.
void Generator::GenerateOperandAsInteger(const ExpressionModel& expr)
{
    GenerateExpression(expr);
    if (expr.GetExpressionValueType() == ValueTypeSingle)
        AddRuntimeCall(RuntimeFTOI, "to Integer");  // result in R0
}

void Generator::GenerateOperandAsSingle(const ExpressionModel& expr)
{
    GenerateExpression(expr);
    if (expr.GetExpressionValueType() == ValueTypeInteger)
        AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack
}

void Generator::GenerateOperandAsSingle(const ExpressionModel& expr, const ExpressionNode& node)
{
    GenerateExpression(expr, node);
    if (node.vtype == ValueTypeInteger)
        AddRuntimeCall(RuntimeITOF, "to Single");  // result on stack
}

void Generator::GenerateExprUnaryNot(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(node.left == -1);
    assert(node.right >= 0);

    const ExpressionNode& noderight = expr.nodes[node.right];
    assert(noderight.vtype != ValueTypeString);

    //NOTE: NOT is a logic operation, so a Single operand is converted to Integer first
    GenerateOperandAsInteger(expr, noderight);

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
    ValueType exprvtype = expr.GetExpressionValueType();

    const string comment = "\t; var " + canoname + " assignment";

    if (expr.IsConstExpression())
    {
        if (vtype == ValueTypeInteger && (exprvtype == ValueTypeInteger || exprvtype == ValueTypeSingle))
        {
            int ivalue = ConstToInteger(expr.GetConstExpressionDValue());
            if (ivalue == 0)
                AddLine("\tCLR\t" + deconame + comment);
            else {
                string svalue = "#" + std::to_string(ivalue) + ".";
                AddLine("\tMOV\t" + svalue + ", " + deconame + comment);
            }
            return;
        }
        if (vtype == ValueTypeSingle && (exprvtype == ValueTypeInteger || exprvtype == ValueTypeSingle))
        {
            float fvalue = static_cast<float>(expr.GetConstExpressionDValue());
            string comment = "\t; var " + canoname + " = const " + to_string_float(static_cast<float>(expr.GetConstExpressionDValue()));
            uint32_t bits = float_to_dec_float(fvalue);
            uint16_t wordlo = bits & 0xFFFF;
            uint16_t wordhi = bits >> 16;
            AddLine((wordlo == 0 ? "\tCLR\t" : "\tMOV\t#" + to_string_octal(wordlo) + ", ") + deconame + comment);
            AddLine((wordhi == 0 ? "\tCLR\t" : "\tMOV\t#" + to_string_octal(wordhi) + ", ") + deconame + "+2");
            return;
        }
        if (vtype == ValueTypeString)  // const String
        {
            string svalue = expr.GetConstExpressionSValue();
            if (svalue == "")  // Special case for empty string
            {
                AddLine("\tCLR\t" + deconame + "\t; var " + canoname + " assignment");
                return;
            }
            if (svalue.size() == 1)  // Special case for one-char string
            {
                //NOTE: Character conversion depends on encoding
                uint16_t value = (1 << 8) | svalue[0];
                AddLine("\tMOV\t#" + to_string_octal(value) + ", " + deconame + "\t; var " + canoname + " assignment");
                return;
            }
            int sindex = m_source->GetConstStringIndex(svalue);
            AddLine("\tMOV\t#ST" + std::to_string(sindex) + ", R0");
            AddLine("\tMOV\t#" + deconame + ", R1");
            AddRuntimeCall(RuntimeSTCP, "var " + canoname + " assignment");
            return;
        }
    }

    if (expr.IsVariableExpression() && exprvtype == ValueTypeInteger && vtype == ValueTypeInteger)
    {
        string svalue = expr.GetVariableExpressionDecoratedName();
        AddLine("\tMOV\t" + svalue + ", " + deconame + comment);
        return;
    }
    if (expr.IsVariableExpression() && exprvtype == ValueTypeSingle && vtype == ValueTypeSingle)
    {
        string svalue = expr.GetVariableExpressionDecoratedName();
        AddLine("\tMOV\t" + svalue + ", " + deconame + comment);
        AddLine("\tMOV\t" + svalue + "+2, " + deconame + "+2");
        return;
    }
    
    // non-const, non-variable
    ExpressionNode& root = expr.nodes[expr.root];

    // Convert "A% = A% + N" and "A% = A% - N" assignments into INC/DEC/ADD/SUB
    //NOTE: IsBinaryOperation() is true for unary '+'/'-' too, so check for both operands here
    if (vtype == ValueTypeInteger && root.token.IsBinaryOperation() &&
        root.left >= 0 && root.right >= 0 &&
        (root.token.text == "-" || root.token.text == "+") &&
        expr.nodes[root.left].token.type == TokenTypeIdentifier &&
        GetCanonicVariableName(expr.nodes[root.left].token.text) == var.name &&
        expr.nodes[root.right].constval &&
        (expr.nodes[root.right].vtype == ValueTypeInteger || expr.nodes[root.right].vtype == ValueTypeSingle))
    {
        bool plusminus = (root.token.text == "+");
        int ivalue = ConstToInteger(expr.nodes[root.right].token.dvalue);
        if (plusminus && ivalue == 1)
            AddLine("\tINC\t" + deconame + comment);
        else if (!plusminus && ivalue == 1)
            AddLine("\tDEC\t" + deconame + comment);
        else if (plusminus && ivalue != 1)
            AddLine("\tADD\t#" + std::to_string(ivalue) + "., " + deconame + comment);
        else //if (!plusminus && ivalue != 1)
            AddLine("\tSUB\t#" + std::to_string(ivalue) + "., " + deconame + comment);
    }
    else if (vtype == ValueTypeSingle)  // non-const Single
    {
        GenerateOperandAsSingle(expr);
        AddLine("\tMOV\t(SP)+, " + deconame + "+2" + comment);
        AddLine("\tMOV\t(SP)+, " + deconame);
    }
    else if (vtype == ValueTypeInteger)  // non-const non-variable Integer
    {
        GenerateExpression(expr);
        if (exprvtype == ValueTypeSingle)
            AddRuntimeCall(RuntimeFTOI, "to Integer");  // result in R0
        AddLine("\tMOV\tR0, " + deconame + comment);
    }
    else  // non-const non-variable String
    {
        GenerateExpression(expr);  // R0 = source string address
        AddLine("\tMOV\t#" + deconame + ", R1");  // R1 = destination buffer address
        AddRuntimeCall(RuntimeSTCP, "var " + canoname + " assignment");
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
            // Integer parameter; Single converted to Integer
            GenerateOperandAsInteger(expr1);  // result in R0
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
            GenerateOperandAsInteger(expr2);  // value as Integer in R0, stack balanced
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
            GenerateOperandAsInteger(expr3);  // value as Integer in R0, stack balanced
            AddLine("\tMOV\tR0, R2");
            AddLine("\tMOV\t(SP)+, R1");  // POP R1
            AddLine(stat1 + "R0");
        }
    }

    AddRuntimeCall(RuntimeCOLR, "COLOR");
}

void Generator::GenerateData(StatementModel& statement)
{
    //NOTE: Data elements generated in DATA BLOCK
    assert(false);  // should never fall down here
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

    assert(statement.forindex != 0);

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
        tovalue = "#" + std::to_string(ConstToInteger(expr2.GetConstExpressionDValue())) + ".";
    }
    else if (expr2.IsVariableExpression())
    {
        //TODO: register variable
        string svalue = expr2.GetVariableExpressionDecoratedName();
        AddLine("\tMOV\t" + svalue + ", @#<F" + std::to_string(statement.forindex) + "+2>");
    }
    else
    {
        GenerateExpression(expr2);
        if (expr2.GetExpressionValueType() == ValueTypeSingle)
            AddRuntimeCall(RuntimeFTOI, "to Integer");  // result in R0
        AddLine("\tMOV\tR0, @#<F" + std::to_string(statement.forindex) + "+2>");  //  Save "to" value
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
            AddLine("\tMOV\tR0, @#<S" + std::to_string(statement.forindex) + "+2>");
        }
    }

    AddLine("F" + std::to_string(statement.forindex) + ":\tCMP\t" + tovalue + ", " + deconame);
    AddLine("\tBGE\t.+6\t; to loop body");
    AddLine("\tJMP\tX" + std::to_string(statement.forindex));  // label after NEXT
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
        assert(forstatement.forindex != 0);

        string canoname = variable.GetVariableCanonicName();
        string deconame = DecorateVariableName(canoname);
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
                int ivalue = ConstToInteger(forexpr3.GetConstExpressionDValue());
                AddLine("\tADD\t#" + std::to_string(ivalue) + "., " + deconame + "\t; " + comment);
            }
            else
            {
                //NOTE: "#1" here will be replaced at run-time with calculated STEP value
                AddLine("S" + std::to_string(forstatement.forindex) + ":\tADD\t#1, " + deconame + "\t; " + comment);
            }
        }

        // JMP to continue loop
        AddLine("\tJMP\tF" + std::to_string(forstatement.forindex) + "\t; continue loop");
        // Label after NEXT
        AddLine("X" + std::to_string(forstatement.forindex) + ":\t; FOR exit addr");
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
    {
        //NOTE: The Single result is two words on the stack; both have to be removed,
        //      otherwise every IF with a Single condition leaks two words and a program
        //      with many of them overflows the stack. A Single is zero only when its
        //      high word (sign and exponent) is zero, so testing the high word is enough.
        AddLine("\tMOV\t(SP)+, R0\t; IF Single: high word");
        AddLine("\tTST\t(SP)+\t; drop low word");
        AddLine("\tTST\tR0\t; check float high word for 0");
    }
    // set flags: Z=0 for TRUE, Z=1 for FALSE
    AddLine("\tBEQ\t" + (haveelse ? labelelse : labelend));
    AddComment("THEN");

    if (statement.stthen == nullptr)  // no THEN statement, so it's THEN linenum
    {
        assert(statement.params.size() >= 1);
        int linenum1 = ConstToInteger(statement.params[0].dvalue);  // THEN line number
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
            int linenum2 = ConstToInteger(statement.params[1].dvalue);
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

    // Expression is Integer or Single, converting to Integer
    GenerateOperandAsInteger(expr);  // result in R0

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
            GenerateOperandAsInteger(expr1);  // Single column -> Integer in R0
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
            GenerateOperandAsInteger(expr2);  // Single row -> Integer in R0
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
            GenerateOperandAsInteger(expr1);  // Single column -> Integer in R0
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
            GenerateOperandAsInteger(expr2);  // Single row -> Integer in R0
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

        GenerateOperandAsInteger(expr3);  // Single on/off -> Integer in R0

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
        // The address must be an Integer; Single converted to Integer
        GenerateOperandAsInteger(expr1);  // result in R0
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
        GenerateOperandAsInteger(expr2);  // value as Integer in R0, stack balanced
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
        GenerateOperandAsInteger(expr2);  // Single mask -> Integer in R0
        stat2 = "\tMOV\tR0, ";
    }

    ExpressionModel& expr3 = statement.args[2];  // code: 0 = BIC, else BIS
    assert(expr3.GetExpressionValueType() != ValueTypeString);

    if (expr3.IsConstExpression())
    {
        string operation = expr3.GetConstExpressionDValue() == 0 ? "BIC" : "BIS";

        if (expr1.IsConstExpression())
        {
            int ivalue1 = ConstToInteger(expr1.GetConstExpressionDValue());
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
            GenerateOperandAsInteger(expr2);  // Single mask -> Integer in R0
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
            GenerateOperandAsInteger(expr1);  // Single address -> Integer in R0
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
            GenerateOperandAsInteger(expr3);  // Single code -> Integer in R0
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
            GenerateOperandAsInteger(expr1);  // Single tab position -> Integer in R0
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
                GenerateOperandAsInteger(expr1);  // Single space count -> Integer in R0
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
        GenerateOperandAsInteger(expr1);  // Single column -> Integer in R0
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
        GenerateOperandAsInteger(expr2);  // Single row -> Integer in R0
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
    assert(!statement.varexprs.empty());

    if (m_source->data.empty())
    {
        Error("READ statement without any DATA to read.");
        return;
    }

    for (const VariableExpressionModel& varexpr : statement.varexprs)
    {
        string deconame = varexpr.GetVariableDecoratedName();
        ValueType vtype = varexpr.GetValueType();
        switch (vtype)
        {
        case ValueTypeInteger:
            AddLine("\tMOV\t#" + deconame + ", R0");
            AddRuntimeCall(RuntimeREAI, "READ Integer");  // result in R0
            break;
        case ValueTypeSingle:
            AddLine("\tMOV\t#" + deconame + ", R0");
            AddRuntimeCall(RuntimeREAF, "READ Single");  // result on 
            break;
        case ValueTypeString:
            AddLine("\tMOV\t#" + deconame + ", R0");
            AddRuntimeCall(RuntimeREAS, "READ String");
            break;
        default:
            assert(false);  // unexpected value type
            break;
        }
    }
}

void Generator::GenerateRem(StatementModel& statement)
{
    // Do nothing
}

void Generator::GenerateRestore(StatementModel& statement)
{
    if (m_source->data.empty())
    {
        Error("RESTORE statement without any DATA to read.");
        return;
    }

    int datanum = 0;
    if (statement.paramline > 0)
    {
        // Find the source line and the DATA statement
        bool linefound = false;
        bool datafound = false;
        int srclinenum = 0;
        for (SourceLineModel& line : m_source->lines)
        {
            if (!linefound)
            {
                if (line.linenum == statement.paramline)
                    linefound = true;
            }
            if (linefound)
            {
                if (line.statement.token.keyword == KeywordDATA)
                {
                    datafound = true;
                    srclinenum = line.srclinenum;
                    break;
                }
            }
        }
        assert(datafound);  // should be covered in validation
        assert(srclinenum > 0);

        // Find that DATA element and its number
        for (DataElementModel& dataelem : m_source->data)
        {
            if (dataelem.srclinenum == srclinenum)
                break;
            datanum++;
        }
    }

    AddLine("\tMOV\t#D" + std::to_string(datanum) + ", R0");
    AddRuntimeCall(RuntimeREST, "RESTORE");
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

// Extension: calls assembler procedure
// CALL <LABEL>
void Generator::GenerateCall(StatementModel& statement)
{
    if (!g_turbo8)
        AddLine("\t.GLOBL\t" + statement.ident.text);
    AddLine("\tCALL\t" + statement.ident.text + "\t; CALL label");
}

