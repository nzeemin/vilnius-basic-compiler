
#include <cassert>
#include <iomanip>
#include <cmath>

#include "main.h"


//////////////////////////////////////////////////////////////////////


string GetCanonicVariableName(const string& name)
{
    const char* str = name.c_str();
    char ch = *str;
    if (ch == 0)
        return str;
    string result;
    if (ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z')
    {
        result.append(1, toupper(ch));
        str++;
        ch = *str;
        if (ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z' || ch >= '0' && ch <= '9')
            result.append(1, toupper(ch));
        char chtype = '!';
        while (true)
        {
            if (ch == '$' || ch == '!' || ch == '%')
            {
                chtype = ch;
                break;
            }
            if (ch == 0)
                break;
            str++;
            ch = *str;
        }
        result.append(1, toupper(chtype));
        return result;
    }
    return name;
}

string DecorateVariableName(const string& name)
{
    char chtype = name[name.length() - 1];
    switch (chtype)
    {
    case '$': chtype = 'S'; break;
    case '%': chtype = 'I'; break;
    default:
        chtype = 'N'; break;
    }
    string stdname = name.substr(0, name.length() - 1);
    if (stdname.length() < 2)
        stdname += '.';

    return "VAR" + stdname + chtype;
}


//////////////////////////////////////////////////////////////////////
// Token

string Token::GetTokenTypeStr() const
{
    switch (type)
    {
    case TokenTypeNone:     return "None";
    case TokenTypeNumber:   return "Number";
    case TokenTypeString:   return "String";
    case TokenTypeDivider:  return "Divider";
    case TokenTypeKeyword:  return "Keyword";
    case TokenTypeIdentifier: return "Ident";
    case TokenTypeSymbol:   return "Symbol";
    case TokenTypeOperation: return "Operation";
    case TokenTypeEOL:      return "EOL";
    case TokenTypeEOT:      return "EOT";
    default:
        return "Unknown";
    }
}

string Token::GetTokenVTypeStr() const
{
    switch (vtype)
    {
    case ValueTypeNone:     return "None";
    case ValueTypeInteger:  return "Integer";
    case ValueTypeSingle:   return "Single";
        //case ValueTypeDouble:   return "Double";
    case ValueTypeString:   return "String";
    default:
        return "unk";
    }
}

void Token::ParseDValue()
{
    const char* str = text.c_str();
    if (vtype == ValueTypeInteger)
    {
        if (*str == '&')
        {
            char* pend;
            switch (str[1])
            {
            case 'H':
                dvalue = (double)strtol(str + 2, &pend, 16);
                break;
            case 'O':
                dvalue = (double)strtol(str + 2, &pend, 8);
                break;
            case 'B':
                dvalue = (double)strtol(str + 2, &pend, 2);
                break;
            }
        }
        else
            dvalue = (double)atoi(str);
    }
    if (vtype == ValueTypeSingle)
    {
        int strlen = text.length();
        int dotpos = text.find('.');
        int epos = text.find('E');

        int epart = 0;
        if (epos > 0)
            epart = atoi(str + epos + 1);
        double ipart = (double)atoi(str);  // integer part including sign
        double fpart = 0;

        if (dotpos >= 0 && epos < 0)  // 123.45 or .45 or 123.
        {
            int fpartlen = strlen - dotpos - 1;
            fpart = fpartlen > 0 ? (double)atoi(str + dotpos + 1) : 0;
            for (int i = 0; i < fpartlen; i++)
                fpart /= 10.0;
        }
        else if (dotpos >= 0 && epos >= 0)  // 123.45E12
        {
            int fpartlen = epos - dotpos - 1;
            fpart = fpartlen > 0 ? (double)atoi(str + dotpos + 1) : 0;
            for (int i = 0; i < fpartlen; i++)
                fpart /= 10.0;
        }

        dvalue = ipart + fpart;

        if (epart < 0)
            for (int i = 0; i < -epart; i++)
                dvalue /= 10.0;
        else if (epart > 0)
            for (int i = 0; i < epart; i++)
                dvalue *= 10.0;
    }
}

void Token::Dump(std::ostream& out) const
{
    out << "{Token " << line << ":" << std::left << std::setw(3) << pos;
    out << " type: " << std::left << std::setw(7);
    if (type == TokenTypeNumber)
        out << GetTokenVTypeStr();
    else
        out << GetTokenTypeStr();
    if (!text.empty())
        out << " text:\"" << text << "\"";  //TODO: Escape special chars
    if (type == TokenTypeSymbol || symbol != 0)
        out << " symb:\'" << symbol << "\'";  //TODO: Escape special chars
    //if (type == TokenTypeNumber)
    {
        std::cout.unsetf(std::ios::floatfield);
        if (IsDValueInteger())
            out << " d:" << dvalue;
        else
            out << " d:" << std::scientific << dvalue;
    }
    out << " }";
}


//////////////////////////////////////////////////////////////////////
// VariableModel

ValueType VariableModel::GetValueType()
{
    char ch = name[name.length() - 1];
    switch (ch)
    {
    case '$': return ValueTypeString;
    case '%': return ValueTypeInteger;
    default:
        return ValueTypeSingle;
    }
}


//////////////////////////////////////////////////////////////////////
// ExpressionNode

int ExpressionNode::GetOperationPriority() const
{
    if (brackets)
        return 1;

    if (node.type == TokenTypeSymbol)
    {
        switch (node.symbol)
        {
        case '^':
            return 2;
        case '*': case '/':
            return 3;
        case '\\':
            return 4;
        case '+': case '-':
            return 6;
        default:
            return 0;
        }
    }

    if (node.type == TokenTypeKeyword && node.keyword == KeywordMOD)
        return 5;

    return 0;
}

void ExpressionNode::Dump(std::ostream& out) const
{
    out << "{Node ";

    if (left == -1)
        out << "  ";
    else
        out << std::right << std::setw(2) << left;
    std::cout << ":";
    if (right == -1)
        out << "  ";
    else
        out << std::left << std::setw(2) << right;
    int pri = GetOperationPriority();
    if (pri > 0)
        out << " !" << pri;
    else
        out << "   ";
    out << " ";

    node.Dump(out);

    if (vtype != ValueTypeNone)
        std::cout << " " << GetNodeVTypeStr();
    if (constval)
        out << " const";

    if (brackets)
        std::cout << " brackets";

    out << " }";
}

string ExpressionNode::GetNodeVTypeStr() const
{
    switch (vtype)
    {
    case ValueTypeNone:     return "None";
    case ValueTypeInteger:  return "Integer";
    case ValueTypeSingle:   return "Single";
        //case ValueTypeDouble:   return "Double";
    case ValueTypeString:   return "String";
    default:
        return "unk";
    }
}


//////////////////////////////////////////////////////////////////////
// ExpressionModel

int ExpressionModel::GetParentIndex(int index) const
{
    for (int i = 0; i < (int)nodes.size(); i++)
    {
        const ExpressionNode& node = nodes[i];
        if (node.left == index || node.right == index)
            return i;
    }

    return -1;  // Not found
}

int ExpressionModel::AddOperationNode(ExpressionNode& node, int prev)
{
    int index = (int)nodes.size();
    int pred = prev < 0 ? root : prev;

    {
        ExpressionNode& nodepred = nodes[pred];
        if (!nodepred.node.IsBinaryOperation() || nodepred.brackets)
        {
            node.left = pred;
            root = index;
            nodes.push_back(node);
            return index;
        }
    }

    int pri = node.GetOperationPriority();
    while (true)
    {
        ExpressionNode& nodepred = nodes[pred];
        int pripred = nodepred.GetOperationPriority();

        if (nodepred.brackets || pripred > pri)
        {
            node.left = nodepred.right;
            nodepred.right = index;
            break;
        }

        int parent = GetParentIndex(pred);
        if (parent < 0)
        {
            node.left = pred;
            root = index;
            break;
        }

        pred = parent;
    }

    nodes.push_back(node);
    return index;
}

//TODO: Move this logic to validator
void ExpressionModel::CalculateVTypes()
{
    if (root < 0)
        return;

    CalculateVTypeForNode(root);
}
void ExpressionModel::CalculateVTypeForNode(int index)
{
    ExpressionNode& node = nodes[index];
    if (node.vtype != ValueTypeNone)
        return;

    if (node.left >= 0)
        CalculateVTypeForNode(node.left);
    if (node.right >= 0)
        CalculateVTypeForNode(node.right);

    //TODO: Unary plus/minus with one operand only

}


//////////////////////////////////////////////////////////////////////
// SourceModel

bool SourceModel::IsVariableRegistered(string varname) const
{
    for (size_t i = 0; i < vars.size(); i++)
    {
        if (vars[i].name == varname)
            return true;
    }
    return false;
}

bool SourceModel::RegisterVariable(VariableModel& var)
{
    for (size_t i = 0; i < vars.size(); i++)
    {
        if (vars[i].name == var.name)
            return false;  // Variable redefinition
    }

    vars.push_back(var);

    return true;
}

bool SourceModel::IsLineNumberExists(int linenumber) const
{
    if (linenumber <= 0 || linenumber > MAX_LINE_NUMBER)
        return false;
    for (size_t i = 0; i < lines.size(); i++)
    {
        if (lines[i].number == linenumber)
            return true;
    }
    return false;
}

int SourceModel::GetNextLineNumber(int linenumber) const
{
    if (linenumber > MAX_LINE_NUMBER)
        return MAX_LINE_NUMBER + 1;
    for (size_t i = 0; i < lines.size(); i++)
    {
        if (lines[i].number > linenumber)
            return lines[i].number;
    }
    return MAX_LINE_NUMBER + 1;
}

SourceLineModel& SourceModel::GetSourceLine(int linenumber)
{
    assert(linenumber < MAX_LINE_NUMBER);

    for (size_t i = 0; i < lines.size(); i++)
    {
        if (lines[i].number == linenumber)
            return lines[i];
    }

    assert(false);  // Line number not found
    exit(EXIT_FAILURE);
}


//////////////////////////////////////////////////////////////////////
