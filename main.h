#pragma once

#include <cstring>
#include <iostream>
#include <limits.h>
#include <vector>
#include <algorithm>
#include <iterator>

#ifndef PATH_MAX
#define PATH_MAX    _MAX_PATH
#endif

const int MAX_LINE_NUMBER = 65535;

typedef std::string string;
typedef std::vector<string> stringvec;

enum KeywordIndex
{
    KeywordNone = 0,
    KeywordABS, KeywordAND, KeywordASC, KeywordAT, KeywordATN, KeywordAUTO,
    KeywordBEEP, KeywordBLOAD, KeywordBSAVE, KeywordBIN,
    KeywordCDBL, KeywordCHR, KeywordCINT, KeywordCIRCLE, KeywordCLEAR, KeywordCLOAD, KeywordCLS,
    KeywordCOLOR, KeywordCONT, KeywordCOS, KeywordCSAVE, KeywordCSNG, KeywordCSRLIN, KeywordCLOSE, KeywordSCREEN,
    KeywordDELETE, KeywordDIM, KeywordDRAW, KeywordDATA, KeywordDEF,
    KeywordELSE, KeywordEND, KeywordEOF, KeywordEXP,
    KeywordFILES, KeywordFIX, KeywordFN, KeywordFOR, KeywordFRE,
    KeywordGOSUB,
    KeywordGOTO,
    KeywordHEX,
    KeywordIF, KeywordIMP, KeywordINKEY, KeywordINP, KeywordINPUT, KeywordINT,
    KeywordKEY,
    KeywordLEN, KeywordLET, KeywordLIST, KeywordLLIST, KeywordLOAD, KeywordLOCATE, KeywordLOG, KeywordLPOS, KeywordLPRINT, KeywordLINE,
    KeywordMID, KeywordMOD, KeywordMERGE,
    KeywordNEW, KeywordNEXT, KeywordNOT,
    KeywordON, KeywordOR, KeywordOUT, KeywordOPEN, KeywordOCT,
    KeywordPAINT, KeywordPEEK, KeywordPI, KeywordPOINT, KeywordPOKE, KeywordPOS, KeywordPRESET, KeywordPRINT, KeywordPSET,
    KeywordREM, KeywordRENUM, KeywordRETURN, KeywordRND, KeywordREAD, KeywordRESTORE,
    KeywordSAVE, KeywordSGN, KeywordSIN, KeywordSQR, KeywordSTEP, KeywordSTOP, KeywordSTR, KeywordSYSTEM, KeywordSTRING, KeywordSPC,
    KeywordTAB, KeywordTAN, KeywordTHEN, KeywordTO, KeywordTROFF, KeywordTRON,
    KeywordUSR,
    KeywordVAL,
    KeywordXOR
};

bool IsFunctionKeyword(KeywordIndex keyword);

enum TokenType
{
    TokenTypeNone       = 0,
    TokenTypeNumber     = 1,
    TokenTypeString     = 2,
    TokenTypeDivider    = 3,
    TokenTypeKeyword    = 4,
    TokenTypeIdentifier = 5,
    TokenTypeSymbol     = 6,
    TokenTypeEOL        = 100,
    TokenTypeEOF        = 101,
};

enum ValueType
{
    ValueTypeNone       = 0,
    ValueTypeInteger    = 1,    // Integer value in range -32768..32767
    ValueTypeSingle     = 2,    // Float value single precision, 4 bytes
    //ValueTypeDouble     = 3,    // Float value double precision, 8 bytes (maybe in the future)
    ValueTypeString     = 10,   // String of length 0..255
};

struct Token
{
    int		    line, pos;
    TokenType   type;
    string	    text;
    char	    symbol;
    KeywordIndex keyword;
    ValueType   vtype;
    double      dvalue;
public:
    Token() :
        line(0), pos(0), type(TokenTypeNone), symbol(0), keyword(KeywordNone), vtype(ValueTypeNone),
        dvalue(0) {}
public:
    bool IsEolOrEof() const { return type == TokenTypeEOL || type == TokenTypeEOF; }
    bool IsOpenBracket() const { return type == TokenTypeSymbol && symbol == '('; }
    bool IsCloseBracket() const { return type == TokenTypeSymbol && symbol == ')'; }
    bool IsComma() const { return type == TokenTypeSymbol && symbol == ','; }
    bool IsEndOfExpression() const
    {
        return type == TokenTypeEOL || type == TokenTypeEOF ||
            type == TokenTypeSymbol && (symbol == ',' || symbol == ';' || symbol == ')') ||
            //TODO: Keyword which is not MOD of function
            type == TokenTypeKeyword && !IsFunctionKeyword(keyword);
    }
    bool IsBinaryOperation() const
    {
        return
            type == TokenTypeSymbol && (
                symbol == '+' || symbol == '-' || symbol == '*' || symbol == '/' || symbol == '\\' || symbol == '^') ||
            type == TokenTypeKeyword && keyword == KeywordMOD;
    }
    string GetTokenTypeStr() const;
    string GetTokenVTypeStr() const;
    void ParseDValue();
    bool IsDValueInteger() const { return floor(dvalue) == ceil(dvalue); }
    void Dump(std::ostream& out) const;
};

struct ExpressionModel;

struct ExpressionNode
{
    Token	    node;
    int         left;
    int         right;
    std::vector<ExpressionModel> args;  // Function argument list
    bool        brackets;       // Flag indicating that this node and all the sub-tree was in brackets
    ValueType   vtype;
    bool        constval;       // Flag for constant value
public:
    ExpressionNode() : left(-1), right(-1), brackets(false), vtype(ValueTypeNone), constval(false) {}
public:
    int GetOperationPriority() const;
    void Dump(std::ostream& out) const;
    string GetNodeVTypeStr() const;
};

struct ExpressionModel
{
    std::vector<ExpressionNode> nodes;
    int root;           // Index of the root node or -1 if the expression is empty
public:
    bool IsEmpty() { return nodes.size() == 0; }
    int GetParentIndex(int index);
    int AddOperationNode(ExpressionNode& node, int prev);  // Add binary operation node into the tree
    void CalculateVTypes();  // Calculate vtype/const for all nodes
    void CalculateVTypeForNode(int index);  // Calculate vtype/const for one node
};

struct SourceLineModel
{
    int		number;		// Line number
    //TODO: text;         // Full line text
    //TODO: StatementModel
    Token	statement;
    int		paramline;	// Line number parameter for GOTO, GOSUB, RESTORE
    Token	ident;	    // LET identifier at left, FOR variable
    bool    relative;   // PSET, PRESET, LINE, CIRCLE, PAINT with '@' sign
    bool    gotogosub;  // true for ON GOTO, false for ON GOSUB
    std::vector<ExpressionModel> args;  // Statement arguments
    std::vector<Token> params;  // Statement params like list of variables
};

struct SourceModel
{
    std::vector<SourceLineModel> lines;
};

struct IntermedModel
{
    std::vector<string> intermeds;
};

class Tokenizer
{
    std::istream* m_pInput;
    int m_line, m_pos;
    string m_text;      // Line text up to current position
    bool m_atend;       // Flag indicating that we should clear m_text on next char
public:
    Tokenizer(std::istream* pInput);
public:
    Token GetNextToken();
    string GetLineText() { return m_text; }
private:
    char GetNextChar();
    char PeekNextChar();
};

class Parser;
typedef void (Parser::* ParseMethodRef)(SourceLineModel&);
struct ParserKeywordSpec
{
    KeywordIndex keyword;
    ParseMethodRef methodref;
};

struct ParserFunctionSpec
{
    KeywordIndex keyword;
    int minparams, maxparams;
    ValueType resulttype;
};

class Parser
{
    Tokenizer* m_tokenizer;
    Token	m_nexttoken;
    bool	m_havenexttoken;
public:
    Parser(Tokenizer* tokenizer);
public:
    SourceLineModel ParseNextLine();
private:
    static const ParserKeywordSpec m_keywordspecs[];
    static const ParserFunctionSpec m_funcspecs[];
    static const int FindFunctionSpec(KeywordIndex keyword);
private:
    Token GetNextToken();
    Token GetNextTokenSkipDivider();
    Token PeekNextToken();
    Token PeekNextTokenSkipDivider();
    void SkipTilEnd();
    void Error(SourceLineModel& model, Token& token, const char* message);
    ExpressionModel ParseExpression(SourceLineModel& model);
    void ParseLetShort(Token& tokenIdent, SourceLineModel& model);
private:
    void ParseBeep(SourceLineModel& model);
    void ParseCls(SourceLineModel& model);
    void ParseColor(SourceLineModel& model);
    void ParseData(SourceLineModel& model);
    void ParseEnd(SourceLineModel& model);
    void ParseFor(SourceLineModel& model);
    void ParseGosub(SourceLineModel& model);
    void ParseGoto(SourceLineModel& model);
    void ParseIf(SourceLineModel& model);
    void ParseLet(SourceLineModel& model);
    void ParseLocate(SourceLineModel& model);
    void ParseNext(SourceLineModel& model);
    void ParseOn(SourceLineModel& model);
    void ParseOut(SourceLineModel& model);
    void ParsePrint(SourceLineModel& model);
    void ParsePoke(SourceLineModel& model);
    void ParseRem(SourceLineModel& model);
    void ParseRestore(SourceLineModel& model);
    void ParseReturn(SourceLineModel& model);
    void ParseScreen(SourceLineModel& model);
    void ParseStop(SourceLineModel& model);
    void ParseTron(SourceLineModel& model);
    void ParseTroff(SourceLineModel& model);
};

class Generator;
typedef void (Generator::* GeneratorMethodRef)(SourceLineModel&);
struct GeneratorKeywordSpec
{
    KeywordIndex keyword;
    GeneratorMethodRef methodref;
};

class Generator
{
    SourceModel*    m_source;
    IntermedModel*  m_intermed;
    int             m_lineindex;
public:
    Generator(SourceModel* source, IntermedModel* intermed);
public:
    void ProcessBegin();
    bool ProcessLine();
    void ProcessEnd();
private:
    static const GeneratorKeywordSpec m_keywordspecs[];
private:
    void GenerateBeep(SourceLineModel& line);
    void GenerateCls(SourceLineModel& line);
    void GenerateEnd(SourceLineModel& line);
    void GenerateFor(SourceLineModel& line);
    void GenerateGosub(SourceLineModel& line);
    void GenerateGoto(SourceLineModel& line);
    void GenerateIf(SourceLineModel& line);
    void GenerateLet(SourceLineModel& line);
    void GenerateNext(SourceLineModel& line);
    void GenerateOn(SourceLineModel& line);
    void GeneratePrint(SourceLineModel& line);
    void GenerateRem(SourceLineModel& line);
    void GenerateReturn(SourceLineModel& line);
    void GenerateStop(SourceLineModel& line);
    void GenerateTron(SourceLineModel& line);
    void GenerateTroff(SourceLineModel& line);
};
