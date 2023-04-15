
#include <cassert>
#include <iomanip>

#include "main.h"

// The table has the same keywords in the same order as KeywordIndex enum
const char* Keywords[] = {
    "ABS", "AND", "ASC", "AT", "ATN", "AUTO",
    "BEEP", "BLOAD", "BSAVE", "BIN$",
    "CDBL", "CHR$", "CINT", "CIRCLE", "CLEAR", "CLOAD", "CLS",
    "COLOR", "CONT", "COS", "CSAVE", "CSNG", "CSRLIN", "CLOSE", "SCREEN",
    "DELETE", "DIM", "DRAW", "DATA", "DEF",
    "ELSE", "END", "EOF", "EXP",
    "FILES", "FIX", "FN", "FOR", "FRE",
    "GOSUB",
    "GOTO",
    "HEX$",
    "IF", "IMP", "INKEY$", "INP", "INPUT", "INT",
    "KEY",
    "LEN", "LET", "LIST", "LLIST", "LOAD", "LOCATE", "LOG", "LPOS", "LPRINT", "LINE",
    "MID$", "MOD", "MERGE",
    "NEW", "NEXT", "NOT",
    "ON", "OR", "OUT", "OPEN", "OCT$",
    "PAINT", "PEEK", "PI", "POINT", "POKE", "POS", "PRESET", "PRINT", "PSET",
    "REM", "RENUM", "RETURN", "RND", "READ", "RESTORE",
    "SAVE", "SGN", "SIN", "SQR", "STEP", "STOP", "STR$", "SYSTEM", "STRING$", "SPC",
    "TAB", "TAN", "THEN", "TO", "TROFF", "TRON",
    "USR",
    "VAL",
    "XOR"
};

bool IsFunctionKeyword(KeywordIndex keyword)
{
    return keyword == KeywordSQR ||
        keyword == KeywordSIN || keyword == KeywordCOS || keyword == KeywordTAN || keyword == KeywordATN ||
        keyword == KeywordPI || keyword == KeywordEXP || keyword == KeywordLOG || keyword == KeywordABS ||
        keyword == KeywordFIX || keyword == KeywordINT || keyword == KeywordSGN || keyword == KeywordRND || keyword == KeywordFRE ||
        keyword == KeywordCINT || keyword == KeywordCSNG || keyword == KeywordCDBL ||
        keyword == KeywordPEEK || keyword == KeywordINP ||
        keyword == KeywordASC || keyword == KeywordCHR || keyword == KeywordLEN || keyword == KeywordMID ||
        keyword == KeywordSTRING || keyword == KeywordVAL || keyword == KeywordINKEY || keyword == KeywordSTR ||
        keyword == KeywordBIN || keyword == KeywordOCT || keyword == KeywordHEX ||
        keyword == KeywordCSRLIN || keyword == KeywordPOS || keyword == KeywordLPOS || keyword == KeywordEOF ||
        keyword == KeywordAT || keyword == KeywordTAB || keyword == KeywordSPC ||
        keyword == KeywordFN || keyword == KeywordUSR;
}

KeywordIndex GetKeywordIndex(string& str)
{
    const char* cstr = str.c_str();
    for (int i = 0; i < sizeof(Keywords) / sizeof(Keywords[0]); i++)
    {
        if (_stricmp(cstr, Keywords[i]) == 0)
            return (KeywordIndex)(i + 1);
    }

    return KeywordNone;
}


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

void Token::Dump(std::ostream& out) const
{
    out << "{Token ";
    out << "line:" << std::right << std::setw(3) << line << " pos:" << std::setw(3) << pos;
    out << " type: " << std::left << std::setw(7);
    if (type == TokenTypeNumber)
        out << GetTokenVTypeStr();
    else
        out << GetTokenTypeStr();
    if (!text.empty())
        out << " text:\"" << text << "\"";  //TODO: Escape special chars
    if (type == TokenTypeSymbol || symbol != 0)
        out << " symb:\'" << symbol << "\'";  //TODO: Escape special chars
    out << " }";
}


Tokenizer::Tokenizer(std::istream * pInput)
{
    assert(pInput != nullptr);
    m_pInput = pInput;
    m_line = m_pos = 1;
}

char Tokenizer::GetNextChar()
{
    if (m_pInput->eof())
        return 0;

    char ch = m_pInput->get();
    if (ch == -1)
        return 0;
    //TODO: if (ch == 0)

    if (ch == '\n')
    {
        m_line++;  m_pos = 1;
        m_text.clear();
        return ch;
    }
    if (ch == '\r')
        return ch;

    m_text.append(1, ch);
    m_pos++;
    return ch;
}

char Tokenizer::PeekNextChar()
{
    if (m_pInput->eof())
        return 0;

    return m_pInput->peek();
}

Token Tokenizer::GetNextToken()
{
    Token token;
    token.line = m_line;
    token.pos = m_pos;

    char ch = GetNextChar();
    if (ch == 0)
    {
        token.type = TokenTypeEOF;
        return token;
    }

    if (ch == '\n')
    {
        token.type = TokenTypeEOL;
        return token;
    }

    if (ch == '\r')
    {
        char next = PeekNextChar();
        if (next == '\n')
        {
            GetNextChar();
            token.type = TokenTypeEOL;
            return token;
        }

        token.symbol = ch;
        token.type = TokenTypeSymbol;
        return token;
    }

    if (ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z')  // Identifier or Keyword
    {
        token.text = ch;

        while (true)
        {
            ch = PeekNextChar();
            if (ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z')
                token.text.append(1, GetNextChar());
            else if (ch == '$')
            {
                token.text.append(1, GetNextChar());
                break;
            }
            else
                break;
        }

        token.type = TokenTypeIdentifier;
        token.keyword = GetKeywordIndex(token.text);
        if (token.keyword != KeywordNone)
            token.type = TokenTypeKeyword;

        return token;
    }

    char ch2 = PeekNextChar();
    if (ch >= '0' && ch <= '9' || ch == '.' ||
        ch == '-' && (ch2 >= '0' && ch2 <= '9' || ch2 == '.'))  // Number
    {
        token.text = ch;
        token.vtype = ValueTypeSingle;  // by default
        bool hasdot = (ch == '.');
        bool hasDorE = false;
        while (true)
        {
            ch = PeekNextChar();
            if (ch >= '0' && ch <= '9')
                token.text.append(1, GetNextChar());
            else if (ch == '.')
            {
                if (hasdot)
                    break;
                token.text.append(1, GetNextChar());
                hasdot = true;
            }
            else if (/*ch == 'D' ||*/ ch == 'E')
            {
                if (hasDorE)
                    break;
                token.text.append(1, GetNextChar());
                hasDorE = true;
                //if (ch == 'E')
                token.vtype = ValueTypeSingle;
                //else if (ch == 'D')
                //    token.vtype = ValueTypeDouble;
                ch = PeekNextChar();
                if (ch == '-')
                    token.text.append(1, GetNextChar());
            }
            else if (ch == '%' || ch == '!' || ch == '#')
            {
                token.text.append(1, GetNextChar());
                if (ch == '%')
                    token.vtype = ValueTypeInteger;
                else if (ch == '!')
                    token.vtype = ValueTypeSingle;
                //else if (ch == '#')
                //    token.vtype = ValueTypeDouble;
                break;
            }
            else
                break;
        }

        token.type = TokenTypeNumber;
        return token;
    }

    // String
    if (ch == '\"')
    {
        token.text = ch;

        while (true)
        {
            ch = PeekNextChar();
            if (ch == '\r' || ch == '\n' || ch == 0)
                break;  // Incomplete string
            else
            {
                token.text.append(1, GetNextChar());
                if (ch == '\"')
                    break;  // Completed string
            }
        }

        token.type = TokenTypeString;
        return token;
    }

    if (ch == ' ' || ch == '\t')
    {
        token.text = ch;

        while (true)
        {
            ch = PeekNextChar();
            if (ch == ' ' || ch == '\t')
                token.text.append(1, GetNextChar());
            else
                break;
        }

        token.type = TokenTypeDivider;
        return token;
    }

    if (ch == '&')	// Hex, Octal, Binary
    {
        char next = PeekNextChar();
        if (next == 'H')  // Hex
        {
            token.text = "&H";
            GetNextChar();
            while (true)
            {
                ch = PeekNextChar();
                if (ch >= '0' && ch <= '9' || ch >= 'A' && ch <= 'F')
                    token.text.append(1, GetNextChar());
                else
                    break;
            }
            token.type = TokenTypeNumber;
            return token;
        }
        else if (next == 'O')  // Octal
        {
            token.text = "&O";
            GetNextChar();
            while (true)
            {
                ch = PeekNextChar();
                if (ch >= '0' && ch <= '7')
                    token.text.append(1, GetNextChar());
                else
                    break;
            }
            token.type = TokenTypeNumber;
            return token;
        }
        else if (next == 'B')  // Binary
        {
            token.text = "&B";
            GetNextChar();
            while (true)
            {
                ch = PeekNextChar();
                if (ch >= '0' && ch <= '1')
                    token.text.append(1, GetNextChar());
                else
                    break;
            }
            token.type = TokenTypeNumber;
            return token;
        }
        // else it is Symbol
    }

    token.symbol = ch;
    token.type = TokenTypeSymbol;
    return token;
}
