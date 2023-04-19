
#include <cassert>
#include <iomanip>
#include <cmath>

#include "main.h"


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
    case TokenTypeEOL:      return "EOL";
    case TokenTypeEOF:      return "EOF";
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
    if (type == TokenTypeNumber)
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

int ExpressionModel::GetParentIndex(int index)
{
    for (int i = 0; i < (int)nodes.size(); i++)
    {
        ExpressionNode& node = nodes[i];
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

    if (node.left >= 0 && node.right >= 0)
    {
        const ExpressionNode& nodeleft = nodes[node.left];
        const ExpressionNode& noderight = nodes[node.right];

        node.constval = (nodeleft.constval && noderight.constval);

        if (nodeleft.vtype == ValueTypeNone || noderight.vtype == ValueTypeNone)
        {
            std::cerr << "ERROR at " << node.node.line << ":" << node.node.pos << " - Cannot calculate value type for the node.";
            exit(EXIT_FAILURE);
        }

        //TODO: Use knowledge about the operation to make this decision

        if (nodeleft.vtype == noderight.vtype)
            node.vtype = nodeleft.vtype;

        if (nodeleft.vtype == ValueTypeSingle && noderight.vtype == ValueTypeInteger ||
            nodeleft.vtype == ValueTypeInteger && noderight.vtype == ValueTypeSingle)
            node.vtype = ValueTypeSingle;

        if ((nodeleft.vtype == ValueTypeSingle || nodeleft.vtype == ValueTypeInteger) && noderight.vtype == ValueTypeString ||
            nodeleft.vtype == ValueTypeString && (noderight.vtype == ValueTypeSingle || noderight.vtype == ValueTypeInteger))
        {
            std::cerr << "ERROR at " << node.node.line << ":" << node.node.pos << " - Value types are incompatible.";
            exit(EXIT_FAILURE);
        }
    }
}


//////////////////////////////////////////////////////////////////////
