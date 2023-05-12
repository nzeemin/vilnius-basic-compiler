#pragma once

#include <cstring>
#include <iostream>
#include <limits.h>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cmath>

#ifndef PATH_MAX
#define PATH_MAX    _MAX_PATH
#endif

#ifdef __GNUC__
#define _stricmp    strcasecmp
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
    KeywordON, KeywordOR, KeywordOPEN, KeywordOCT, KeywordOUT, KeywordOUTPUT,
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

enum TokenizerMode
{
    TokenizerModeUsual  = 0,
    TokenizerModeData   = 1,
};

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

enum FileMode
{
    FileModeAny         = 0,
    FileModeInput       = 1,
    FileModeOutput      = 2,
};

extern void RegisterError();

string GetCanonicVariableName(const string& name);
string DecorateVariableName(const string& name);

struct Token
{
    int		    line, pos;
    TokenType   type;
    string      text;
    char	    symbol;
    KeywordIndex keyword;
    ValueType   vtype;
    double      dvalue;
    string      svalue;
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
    bool IsKeyword(KeywordIndex index) const { return type == TokenTypeKeyword && keyword == index; }
    bool IsEndOfExpression() const
    {
        return type == TokenTypeEOL || type == TokenTypeEndComment || type == TokenTypeEOT ||
            (type == TokenTypeSymbol && (symbol == ',' || symbol == ';' || symbol == ')')) ||
            (type == TokenTypeKeyword && !IsFunctionKeyword(keyword));
    }
    bool IsBinaryOperation() const
    {
        return
            type == TokenTypeOperation ||
            (type == TokenTypeKeyword && keyword == KeywordMOD);
    }
    string GetTokenTypeStr() const;
    string GetTokenVTypeStr() const;
    void ParseDValue();
    bool IsDValueInteger() const { return std::floor(dvalue) == std::ceil(dvalue); }
    void Dump(std::ostream& out) const;
};

struct VariableBaseModel
{
    string name;  // Variable name in canonic form
public:
    ValueType GetValueType() const;
    string GetVariableDecoratedName() const { return DecorateVariableName(GetCanonicVariableName(name)); }
};

struct VariableModel : VariableBaseModel
{
    std::vector<int> indices;  // List of variable indices if any
};

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
    bool IsEmpty() const { return nodes.size() == 0; }
    int GetParentIndex(int index) const;
    bool IsConstExpression() const;
    double GetConstExpressionDValue() const;
    string GetConstExpressionSValue() const;
    bool IsVariableExpression() const;
    string GetVariableExpressionDecoratedName() const;
    ValueType GetExpressionValueType() const;
    int AddOperationNode(ExpressionNode& node, int prev);  // Add binary operation node into the tree
};

struct VariableExpressionModel : VariableBaseModel
{
    std::vector<ExpressionModel> args;  // List of variable indices as expressions
};

struct StatementModel
{
    Token	token;
    Token	ident;	    // LET identifier at left, FOR variable
    int		paramline;	// Line number parameter for GOTO, GOSUB, RESTORE
    bool    relative;   // PSET, PRESET, LINE, CIRCLE, PAINT with '@' sign
    bool    fileoper;   // File operation, for INPUT
    bool    gotogosub;  // true for ON GOTO, false for ON GOSUB
    bool    deffnorusr; // true for DEF FN, false for DEF USR
    bool    nocrlf;     // PRINT flag indicating we don't need CR/LF at the end
    FileMode filemode;  // File mode for OPEN
    std::vector<ExpressionModel> args;  // Statement arguments
    std::vector<Token> params;  // Statement params like list of variables
    std::vector<VariableModel> variables;  // Variables with indices
    std::vector<VariableExpressionModel> varexprs;  // Variables with expressions for indices
public:
    StatementModel() :
        paramline(0), relative(false), gotogosub(false), deffnorusr(false), fileoper(false), nocrlf(false),
        filemode(FileModeAny) { }
};

struct SourceLineModel
{
    int		number;		// Line number
    string  text;       // Full line text
    bool    error;      // Flag indicating that this line has an error
    StatementModel statement;
public:
    SourceLineModel() :
        number(0), error(false) {}
};

struct SourceModel
{
    std::vector<SourceLineModel> lines;
    std::vector<VariableModel> vars;//TODO: change to set
    std::vector<string> conststrings;//TODO: change to set
public:
    bool RegisterVariable(const VariableModel& var);  // Add variable to the list
    bool RegisterVariable(const VariableExpressionModel& var);
    bool IsVariableRegistered(const string& varname) const;
    bool IsLineNumberExists(int linenumber) const;
    int GetNextLineNumber(int linenumber) const;
    SourceLineModel& GetSourceLine(int linenumber);
    void RegisterConstString(const string& str);
    int GetConstStringIndex(const string& str);
};

struct FinalModel
{
    std::vector<string> lines;
public:
    void AddLine(string str) { lines.push_back(str); }
};


//////////////////////////////////////////////////////////////////////


class Tokenizer
{
    std::istream* m_pInput;
    string m_text;      // Line text
    int m_line, m_pos;  // Line number (1-based) and position (1-based)
    bool m_eof;
    bool m_atend;       // Flag indicating that we should clear m_text on next char
    TokenizerMode m_mode;
public:
    Tokenizer(std::istream* pInput);
public:
    Token GetNextToken();
    string GetLineText() { return m_text; }
    void SetMode(TokenizerMode mode) { m_mode = mode; }
private:
    void PrepareLine();
    char GetNextChar();
    char PeekNextChar();
    void TokenizeEndComment(char ch, Token& token);
    void TokenizeIdentifierOrKeyword(char ch, Token& token);
    void TokenizeNumber(char ch, char ch2, Token& token);
    void TokenizeString(char ch, Token& token);
    void TokenizeDataString(char ch, Token& token);
    void TokenizeHexOctalBinary(char ch, char next, Token& token);
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
    static const ParserFunctionSpec* FindFunctionSpec(KeywordIndex keyword);
private:
    Token GetNextToken();
    Token GetNextTokenSkipDivider();
    Token PeekNextToken();
    Token PeekNextTokenSkipDivider();
    void SkipTilEnd();
    void SkipComma(SourceLineModel& model);
    void Error(SourceLineModel& model, const Token& token, const string& message);
    ExpressionModel ParseExpression(SourceLineModel& model);
    VariableModel ParseVariable(SourceLineModel& model);
    VariableExpressionModel ParseVariableExpression(SourceLineModel& model);
    void ParseLetShort(Token& tokenIdentOrMid, SourceLineModel& model);
private:
    void ParseIgnoredStatement(SourceLineModel& model);
    void ParseStatementNoParams(SourceLineModel& model);
    void ParseClear(SourceLineModel& model);
    void ParseColor(SourceLineModel& model);
    void ParseData(SourceLineModel& model);
    void ParseDef(SourceLineModel& model);
    void ParseDefFn(SourceLineModel& model);
    void ParseDefUsr(SourceLineModel& model);
    void ParseDim(SourceLineModel& model);
    void ParseDraw(SourceLineModel& model);
    void ParseFor(SourceLineModel& model);
    void ParseGotoGosub(SourceLineModel& model);
    void ParseIf(SourceLineModel& model);
    void ParseInput(SourceLineModel& model);
    void ParseKey(SourceLineModel& model);
    void ParseLet(SourceLineModel& model);
    void ParseLocate(SourceLineModel& model);
    void ParseNext(SourceLineModel& model);
    void ParseOn(SourceLineModel& model);
    void ParseOpen(SourceLineModel& model);
    void ParseOut(SourceLineModel& model);
    void ParsePrint(SourceLineModel& model);
    void ParsePoke(SourceLineModel& model);
    void ParsePsetPreset(SourceLineModel& model);
    void ParseLine(SourceLineModel& model);
    void ParseCircle(SourceLineModel& model);
    void ParsePaint(SourceLineModel& model);
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
typedef void (Validator::* ValidatorOperMethodRef)(ExpressionModel&, ExpressionNode&, const ExpressionNode&, const ExpressionNode&);
struct ValidatorOperSpec
{
    string text;
    ValidatorOperMethodRef methodref;
};
typedef void (Validator::* ValidatorFuncMethodRef)(ExpressionModel&, ExpressionNode&);
struct ValidatorFuncSpec
{
    KeywordIndex keyword;
    ValidatorFuncMethodRef methodref;
};

struct ValidatorForSpec
{
    int     linenum;  // FOR statement line number
    string  varname;
};

class Validator
{
    SourceModel*    m_source;
    int             m_lineindex;
    int             m_linenumber;  // Line number for the line under analysis, for error reporting
    std::vector<ValidatorForSpec> m_fornextstack;
private:
    static const ValidatorKeywordSpec m_keywordspecs[];
    static const ValidatorOperSpec m_operspecs[];
    static const ValidatorFuncSpec m_funcspecs[];
public:
    Validator(SourceModel* source);
public:
    bool ProcessLine();
private:
    void Error(SourceLineModel& line, const string& message);
    void Error(ExpressionModel& expr, const string& message);
    void Error(ExpressionModel& expr, const ExpressionNode& node, const string& message);
    bool CheckIntegerOrSingleExpression(ExpressionModel& expr);
    bool CheckStringExpression(ExpressionModel& expr);
    void ValidateExpression(ExpressionModel& expr);
    void ValidateExpression(ExpressionModel& expr, int index);
private:
    void ValidateNothing(SourceLineModel& model);
    void ValidateCircle(SourceLineModel& model);
    void ValidateClear(SourceLineModel& model);
    void ValidateColor(SourceLineModel& model);
    void ValidateData(SourceLineModel& model);
    void ValidateDef(SourceLineModel& model);
    void ValidateDim(SourceLineModel& model);
    void ValidateKey(SourceLineModel& model);
    void ValidateDraw(SourceLineModel& model);
    void ValidateFor(SourceLineModel& model);
    void ValidateGotoGosub(SourceLineModel& model);
    void ValidateIf(SourceLineModel& model);
    void ValidateInput(SourceLineModel& model);
    void ValidateLet(SourceLineModel& model);
    void ValidateLine(SourceLineModel& model);
    void ValidateLocate(SourceLineModel& model);
    void ValidateNext(SourceLineModel& model);
    void ValidateOn(SourceLineModel& model);
    void ValidateOpen(SourceLineModel& model);
    void ValidateOut(SourceLineModel& model);
    void ValidatePaint(SourceLineModel& model);
    void ValidatePoke(SourceLineModel& model);
    void ValidatePrint(SourceLineModel& model);
    void ValidatePreset(SourceLineModel& model);
    void ValidatePset(SourceLineModel& model);
    void ValidateRead(SourceLineModel& model);
    void ValidateRestore(SourceLineModel& model);
    void ValidateScreen(SourceLineModel& model);
    void ValidateWidth(SourceLineModel& model);
private:
    void ValidateUnaryPlus(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& noderight);
    void ValidateUnaryMinus(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& noderight);
private:
    void ValidateOperPlus(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void ValidateOperMinus(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void ValidateOperMul(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void ValidateOperDiv(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void ValidateOperDivInt(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void ValidateOperMod(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void ValidateOperPower(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void ValidateOperEqual(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void ValidateOperNotEqual(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void ValidateOperLess(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void ValidateOperGreater(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void ValidateOperLessOrEqual(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void ValidateOperGreaterOrEqual(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void ValidateOperAnd(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void ValidateOperOr(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
private:
    void ValidateFuncSin(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncCos(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncTan(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncAtn(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncPi(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncExp(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncLog(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncAbs(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncFix(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncInt(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncSgn(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncRnd(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncFre(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncCint(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncCsng(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncPeek(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncInp(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncAsc(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncChr(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncLen(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncMid(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncString(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncVal(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncInkey(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncStr(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncBin(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncOct(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncHex(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncCsrlinPosLpos(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncEof(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncPoint(ExpressionModel& expr, ExpressionNode& node);
};

class Generator;
typedef void (Generator::* GeneratorMethodRef)(SourceLineModel&);
struct GeneratorKeywordSpec
{
    KeywordIndex keyword;
    GeneratorMethodRef methodref;
};
typedef void (Generator::* GeneratorOperMethodRef)(const ExpressionModel&, const ExpressionNode&, const ExpressionNode&, const ExpressionNode&);
struct GeneratorOperSpec
{
    string text;
    GeneratorOperMethodRef methodref;
};
typedef void (Generator::* GeneratorFuncMethodRef)(const ExpressionModel&, const ExpressionNode&);
struct GeneratorFuncSpec
{
    KeywordIndex keyword;
    GeneratorFuncMethodRef methodref;
};

class Generator
{
    SourceModel*    m_source;
    FinalModel*     m_final;
    int             m_lineindex;
public:
    Generator(SourceModel* source, FinalModel* intermed);
public:
    void ProcessBegin();
    bool ProcessLine();
    void ProcessEnd();
private:
    static const GeneratorKeywordSpec m_keywordspecs[];
    static const GeneratorOperSpec m_operspecs[];
    static const GeneratorFuncSpec m_funcspecs[];
private:
    void Error(SourceLineModel& line, const string& message);
    void GenerateExpression(const ExpressionModel& expr);
    void GenerateExpression(const ExpressionModel& expr, const ExpressionNode& node);
    void GenerateExprFunction(const ExpressionModel& expr, const ExpressionNode& node);
    void GenerateExprBinaryOperation(const ExpressionModel& expr, const ExpressionNode& node);
    void GenerateAssignment(SourceLineModel& line, VariableExpressionModel& var, ExpressionModel& expr);
private:
    void GenerateIgnoredStatement(SourceLineModel& line);
    void GenerateBeep(SourceLineModel& line);
    void GenerateCircle(SourceLineModel& line);
    void GenerateClear(SourceLineModel& line);
    void GenerateClose(SourceLineModel& line);
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
    void GenerateInput(SourceLineModel& line);
    void GenerateLet(SourceLineModel& line);
    void GenerateLine(SourceLineModel& line);
    void GenerateLocate(SourceLineModel& line);
    void GenerateNext(SourceLineModel& line);
    void GenerateOn(SourceLineModel& line);
    void GenerateOpen(SourceLineModel& line);
    void GeneratePaint(SourceLineModel& line);
    void GeneratePoke(SourceLineModel& line);
    void GeneratePrint(SourceLineModel& line);
    void GeneratePreset(SourceLineModel& line);
    void GeneratePset(SourceLineModel& line);
    void GenerateRead(SourceLineModel& line);
    void GenerateRem(SourceLineModel& line);
    void GenerateRestore(SourceLineModel& line);
    void GenerateReturn(SourceLineModel& line);
    void GenerateScreen(SourceLineModel& line);
    void GenerateStop(SourceLineModel& line);
    void GenerateWidth(SourceLineModel& line);
private:
    void GenerateOperPlus(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void GenerateOperMinus(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void GenerateOperMul(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void GenerateOperDiv(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void GenerateOperDivInt(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void GenerateOperPower(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void GenerateLogicOperIntegerArguments(const ExpressionModel& expr, const ExpressionNode& nodeleft, const ExpressionNode& noderight, const string& comment);
    void GenerateOperEqual(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void GenerateOperNotEqual(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void GenerateOperLess(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void GenerateOperGreater(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
private:
    void GenerateFuncAbs(const ExpressionModel& expr, const ExpressionNode& node);
    void GenerateFuncRnd(const ExpressionModel& expr, const ExpressionNode& node);
    void GenerateFuncPeek(const ExpressionModel& expr, const ExpressionNode& node);
    void GenerateFuncInp(const ExpressionModel& expr, const ExpressionNode& node);
    void GenerateFuncLen(const ExpressionModel& expr, const ExpressionNode& node);
};
