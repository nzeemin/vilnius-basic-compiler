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
    KeywordWIDTH,
    KeywordXOR
};

bool IsFunctionKeyword(KeywordIndex keyword);
string GetKeywordString(KeywordIndex keyword);

enum TokenType
{
    TokenTypeNone       = 0,
    TokenTypeNumber     = 1,
    TokenTypeString     = 2,
    TokenTypeDivider    = 3,
    TokenTypeKeyword    = 4,
    TokenTypeIdentifier = 5,
    TokenTypeSymbol     = 6,
    TokenTypeOperation  = 7,
    TokenTypeEOL        = 100,
    TokenTypeEndComment = 101,  // Apostroph and text after that, including EOL
    TokenTypeEOT        = 200,  // End of text
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
    bool IsEolOrEof() const { return type == TokenTypeEOL || type == TokenTypeEndComment || type == TokenTypeEOT; }
    bool IsOpenBracket() const { return type == TokenTypeSymbol && symbol == '('; }
    bool IsCloseBracket() const { return type == TokenTypeSymbol && symbol == ')'; }
    bool IsComma() const { return type == TokenTypeSymbol && symbol == ','; }
    bool IsSemicolon() const { return type == TokenTypeSymbol && symbol == ';'; }
    bool IsEqualSign() const { return type == TokenTypeOperation && text == "="; }
    bool IsEndOfExpression() const
    {
        return type == TokenTypeEOL || type == TokenTypeEndComment || type == TokenTypeEOT ||
            type == TokenTypeSymbol && (symbol == ',' || symbol == ';' || symbol == ')') ||
            type == TokenTypeKeyword && !IsFunctionKeyword(keyword);
    }
    bool IsBinaryOperation() const
    {
        return
            type == TokenTypeOperation ||
            type == TokenTypeKeyword && keyword == KeywordMOD;
    }
    string GetTokenTypeStr() const;
    string GetTokenVTypeStr() const;
    void ParseDValue();
    bool IsDValueInteger() const { return floor(dvalue) == ceil(dvalue); }
    void Dump(std::ostream& out) const;
};

struct VariableModel
{
    string name;  // Variable name in canonic form
    std::vector<int> indices;  // List of variable indices if any
public:
    ValueType GetValueType();
};

extern void RegisterError();

string GetCanonicVariableName(const string& name);
string DecorateVariableName(const string& name);

struct ExpressionModel;

struct ExpressionNode
{
    Token	    node;//TODO: rename to token
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
    string  text;       // Full line text
    bool    error;      // Flag indicating that this line has an error
    //TODO: StatementModel
    Token	statement;
    int		paramline;	// Line number parameter for GOTO, GOSUB, RESTORE
    Token	ident;	    // LET identifier at left, FOR variable
    bool    relative;   // PSET, PRESET, LINE, CIRCLE, PAINT with '@' sign
    bool    gotogosub;  // true for ON GOTO, false for ON GOSUB
    std::vector<ExpressionModel> args;  // Statement arguments
    std::vector<Token> params;  // Statement params like list of variables
    std::vector<VariableModel> variables;
public:
    SourceLineModel() : number(0), error(false), paramline(0), relative(false), gotogosub(false) {}
};

struct SourceModel
{
    std::vector<SourceLineModel> lines;
    std::vector<VariableModel> vars;//TODO: change to set
public:
    bool IsVariableRegistered(string varname);
    bool RegisterVariable(VariableModel& var);  // Add variable to the list
    bool IsLineNumberExists(int linenumber);
    int GetNextLineNumber(int linenumber);
};

struct IntermedModel
{
    std::vector<string> intermeds;
};

class Tokenizer
{
    std::istream* m_pInput;
    string m_text;      // Line text
    int m_line, m_pos;  // Line number (1-based) and position (1-based)
    bool m_eof;
    bool m_atend;       // Flag indicating that we should clear m_text on next char
public:
    Tokenizer(std::istream* pInput);
public:
    Token GetNextToken();
    string GetLineText() { return m_text; }
private:
    void PrepareLine();
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
    int     m_prevlinenum;
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
    void SkipComma(SourceLineModel& model);
    void Error(SourceLineModel& model, Token& token, string message);
    ExpressionModel ParseExpression(SourceLineModel& model);
    void ParseLetShort(Token& tokenIdentOrMid, SourceLineModel& model);
private:
    void ParseStatementNoParams(SourceLineModel& model);
    void ParseClear(SourceLineModel& model);
    void ParseColor(SourceLineModel& model);
    void ParseData(SourceLineModel& model);
    void ParseDim(SourceLineModel& model);
    void ParseDraw(SourceLineModel& model);
    void ParseFor(SourceLineModel& model);
    void ParseGotoGosub(SourceLineModel& model);
    void ParseIf(SourceLineModel& model);
    void ParseInput(SourceLineModel& model);
    void ParseLet(SourceLineModel& model);
    void ParseLocate(SourceLineModel& model);
    void ParseNext(SourceLineModel& model);
    void ParseOn(SourceLineModel& model);
    void ParseOut(SourceLineModel& model);
    void ParsePrint(SourceLineModel& model);
    void ParsePoke(SourceLineModel& model);
    void ParsePsetPreset(SourceLineModel& model);
    void ParseRead(SourceLineModel& model);
    void ParseRem(SourceLineModel& model);
    void ParseRestore(SourceLineModel& model);
    void ParseScreen(SourceLineModel& model);
    void ParseWidth(SourceLineModel& model);
};

class Validator;
typedef void (Validator::* ValidatorMethodRef)(SourceLineModel&);
struct ValidatorKeywordSpec
{
    KeywordIndex keyword;
    ValidatorMethodRef methodref;
};

class Validator
{
    SourceModel*    m_source;
    int             m_lineindex;
private:
    static const ValidatorKeywordSpec m_keywordspecs[];
public:
    Validator(SourceModel* source);
public:
    bool ProcessLine();
private:
    void Error(SourceLineModel& line, string message);
    bool CheckIntegerExpression(SourceLineModel& model, ExpressionModel& expr);
    void ValidateExpression(ExpressionModel& expr, int index);
    void ValidateNothing(SourceLineModel& model);
    void ValidateClear(SourceLineModel& model);
    void ValidateColor(SourceLineModel& model);
    void ValidateDim(SourceLineModel& model);
    void ValidateDraw(SourceLineModel& model);
    void ValidateFor(SourceLineModel& model);
    void ValidateGotoGosub(SourceLineModel& model);
    void ValidateIf(SourceLineModel& model);
    void ValidateInput(SourceLineModel& model);
    void ValidateLet(SourceLineModel& model);
    void ValidateLocate(SourceLineModel& model);
    void ValidateNext(SourceLineModel& model);
    void ValidateOn(SourceLineModel& model);
    void ValidateOut(SourceLineModel& model);
    void ValidatePoke(SourceLineModel& model);
    void ValidatePrint(SourceLineModel& model);
    void ValidateRestore(SourceLineModel& model);
    void ValidateScreen(SourceLineModel& model);
    void ValidateWidth(SourceLineModel& model);
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
    void Error(SourceLineModel& line, string message);
    void GenerateExpression(ExpressionModel& expr);
    void GenerateBeep(SourceLineModel& line);
    void GenerateClear(SourceLineModel& line);
    void GenerateCls(SourceLineModel& line);
    void GenerateColor(SourceLineModel& line);
    void GenerateData(SourceLineModel& line);
    void GenerateDim(SourceLineModel& line);
    void GenerateDraw(SourceLineModel& line);
    void GenerateEnd(SourceLineModel& line);
    void GenerateFor(SourceLineModel& line);
    void GenerateGosub(SourceLineModel& line);
    void GenerateGoto(SourceLineModel& line);
    void GenerateIf(SourceLineModel& line);
    void GenerateLet(SourceLineModel& line);
    void GenerateLocate(SourceLineModel& line);
    void GenerateNext(SourceLineModel& line);
    void GenerateOn(SourceLineModel& line);
    void GeneratePrint(SourceLineModel& line);
    void GenerateRead(SourceLineModel& line);
    void GenerateRem(SourceLineModel& line);
    void GenerateRestore(SourceLineModel& line);
    void GenerateReturn(SourceLineModel& line);
    void GenerateScreen(SourceLineModel& line);
    void GenerateStop(SourceLineModel& line);
    void GenerateTron(SourceLineModel& line);
    void GenerateTroff(SourceLineModel& line);
    void GenerateWidth(SourceLineModel& line);
};
