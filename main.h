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
    KeywordELSE, KeywordEND, KeywordEOF, KeywordEQV, KeywordEXP,
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
    bool IsEndOfStatement() const
    {
        return type == TokenTypeEOL || type == TokenTypeEndComment || type == TokenTypeEOT ||
            (type == TokenTypeKeyword && keyword == KeywordELSE);
    }
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
    Token	token;      // Token for the statement keyword
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
    StatementModel* stthen;
    StatementModel* stelse;
public:
    StatementModel() :
        paramline(0), relative(false), fileoper(false), gotogosub(false), deffnorusr(false), nocrlf(false),
        filemode(FileModeAny), stthen(nullptr), stelse(nullptr) { }
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
    void AddLine(const string& str);
    void AddComment(const string& str);
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
typedef void (Parser::* ParseMethodRef)(StatementModel&);
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
    SourceLineModel* m_line;  // Curent line being parsed
public:
    Parser(Tokenizer* tokenizer);
public:
    SourceLineModel ParseNextLine();
private:
    static const ParserKeywordSpec m_keywordspecs[];
    static const ParserFunctionSpec m_funcspecs[];
    static const ParserFunctionSpec* FindFunctionSpec(KeywordIndex keyword);
private:
    void ParseStatement(StatementModel& statement);
    Token GetNextToken();
    Token GetNextTokenSkipDivider();
    Token PeekNextToken();
    Token PeekNextTokenSkipDivider();
    void SkipTilEnd();
    void SkipTilStatementEnd();
    void SkipComma();
    void Error(const Token& token, const string& message);
    ExpressionModel ParseExpression();
    VariableModel ParseVariable();
    VariableExpressionModel ParseVariableExpression();
    void ParseLetShort(Token& tokenIdentOrMid, StatementModel& statement);
private:
    void ParseIgnoredStatement(StatementModel& statement);
    void ParseStatementNoParams(StatementModel& statement);
    void ParseClear(StatementModel& statement);
    void ParseColor(StatementModel& statement);
    void ParseData(StatementModel& statement);
    void ParseDef(StatementModel& statement);
    void ParseDefFn(StatementModel& statement);
    void ParseDefUsr(StatementModel& statement);
    void ParseDim(StatementModel& statement);
    void ParseDraw(StatementModel& statement);
    void ParseFor(StatementModel& statement);
    void ParseGotoGosub(StatementModel& statement);
    void ParseIf(StatementModel& statement);
    void ParseInput(StatementModel& statement);
    void ParseKey(StatementModel& statement);
    void ParseLet(StatementModel& statement);
    void ParseLocate(StatementModel& statement);
    void ParseNext(StatementModel& statement);
    void ParseOn(StatementModel& statement);
    void ParseOpen(StatementModel& statement);
    void ParseOut(StatementModel& statement);
    void ParsePrint(StatementModel& statement);
    void ParsePoke(StatementModel& statement);
    void ParsePsetPreset(StatementModel& statement);
    void ParseLine(StatementModel& statement);
    void ParseCircle(StatementModel& statement);
    void ParsePaint(StatementModel& statement);
    void ParseRead(StatementModel& statement);
    void ParseRem(StatementModel& statement);
    void ParseRestore(StatementModel& statement);
    void ParseScreen(StatementModel& statement);
    void ParseWidth(StatementModel& statement);
};

class Validator;
typedef void (Validator::* ValidatorMethodRef)(StatementModel&);
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
    SourceLineModel* m_line;  // Curent line being validated
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
    void ValidateStatement(StatementModel& statement);
    void Error(const string& message);
    void Error(ExpressionModel& expr, const string& message);
    void Error(ExpressionModel& expr, const ExpressionNode& node, const string& message);
    bool CheckIntegerOrSingleExpression(ExpressionModel& expr);
    bool CheckStringExpression(ExpressionModel& expr);
    void ValidateExpression(ExpressionModel& expr);
    void ValidateExpression(ExpressionModel& expr, int index);
private:
    void ValidateNothing(StatementModel& statement);
    void ValidateCircle(StatementModel& statement);
    void ValidateClear(StatementModel& statement);
    void ValidateColor(StatementModel& statement);
    void ValidateData(StatementModel& statement);
    void ValidateDef(StatementModel& statement);
    void ValidateDim(StatementModel& statement);
    void ValidateKey(StatementModel& statement);
    void ValidateDraw(StatementModel& statement);
    void ValidateFor(StatementModel& statement);
    void ValidateGotoGosub(StatementModel& statement);
    void ValidateIf(StatementModel& statement);
    void ValidateInput(StatementModel& statement);
    void ValidateLet(StatementModel& statement);
    void ValidateLine(StatementModel& statement);
    void ValidateLocate(StatementModel& statement);
    void ValidateNext(StatementModel& statement);
    void ValidateOn(StatementModel& statement);
    void ValidateOpen(StatementModel& statement);
    void ValidateOut(StatementModel& statement);
    void ValidatePaint(StatementModel& statement);
    void ValidatePoke(StatementModel& statement);
    void ValidatePrint(StatementModel& statement);
    void ValidatePsetPreset(StatementModel& statement);
    void ValidateRead(StatementModel& statement);
    void ValidateRestore(StatementModel& statement);
    void ValidateScreen(StatementModel& statement);
    void ValidateWidth(StatementModel& statement);
private:
    void ValidateUnaryPlus(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& noderight);
    void ValidateUnaryMinus(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& noderight);
    void ValidateUnaryNot(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& noderight);
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
    void ValidateOperXor(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void ValidateOperEqv(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void ValidateOperImp(ExpressionModel& expr, ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
private:
    void ValidateFuncSin(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncCos(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncTan(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncAtn(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncPi(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncExp(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncLog(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncAbs(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncInt(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncSgn(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncRnd(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncFre(ExpressionModel& expr, ExpressionNode& node);
    void ValidateFuncCintFix(ExpressionModel& expr, ExpressionNode& node);
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
typedef void (Generator::* GeneratorMethodRef)(StatementModel&);
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
    SourceLineModel* m_line;  // Curent line being generated
public:
    Generator(SourceModel* source, FinalModel* intermed);
public:
    void ProcessBegin();
    bool ProcessLine();
    void ProcessEnd();
    void GenerateStrings();
    void GenerateVariables();
private:
    static const GeneratorKeywordSpec m_keywordspecs[];
    static const GeneratorOperSpec m_operspecs[];
    static const GeneratorFuncSpec m_funcspecs[];
private:
    void Error(const string& message);
    void GenerateExpression(const ExpressionModel& expr);
    void GenerateExpression(const ExpressionModel& expr, const ExpressionNode& node);
    void GenerateExprFunction(const ExpressionModel& expr, const ExpressionNode& node);
    void GenerateExprBinaryOperation(const ExpressionModel& expr, const ExpressionNode& node);
    void GenerateAssignment(VariableExpressionModel& var, ExpressionModel& expr);
private:
    void GenerateIgnoredStatement(StatementModel& statement);
    void GenerateBeep(StatementModel& statement);
    void GenerateCircle(StatementModel& statement);
    void GenerateClear(StatementModel& statement);
    void GenerateClose(StatementModel& statement);
    void GenerateCls(StatementModel& statement);
    void GenerateColor(StatementModel& statement);
    void GenerateData(StatementModel& statement);
    void GenerateDim(StatementModel& statement);
    void GenerateDraw(StatementModel& statement);
    void GenerateEnd(StatementModel& statement);
    void GenerateFor(StatementModel& statement);
    void GenerateGosub(StatementModel& statement);
    void GenerateGoto(StatementModel& statement);
    void GenerateIf(StatementModel& statement);
    void GenerateInput(StatementModel& statement);
    void GenerateLet(StatementModel& statement);
    void GenerateLine(StatementModel& statement);
    void GenerateLocate(StatementModel& statement);
    void GenerateNext(StatementModel& statement);
    void GenerateOn(StatementModel& statement);
    void GenerateOpen(StatementModel& statement);
    void GenerateOut(StatementModel& statement);
    void GeneratePaint(StatementModel& statement);
    void GeneratePoke(StatementModel& statement);
    void GeneratePrint(StatementModel& statement);
    void GeneratePrintAt(const ExpressionModel& expr);
    void GeneratePrintString(const ExpressionModel& expr);
    void GeneratePreset(StatementModel& statement);
    void GeneratePset(StatementModel& statement);
    void GenerateRead(StatementModel& statement);
    void GenerateRem(StatementModel& statement);
    void GenerateRestore(StatementModel& statement);
    void GenerateReturn(StatementModel& statement);
    void GenerateScreen(StatementModel& statement);
    void GenerateStop(StatementModel& statement);
    void GenerateWidth(StatementModel& statement);
private:
    void GenerateOperPlus(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void GenerateOperMinus(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void GenerateOperMul(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void GenerateOperDiv(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void GenerateOperDivInt(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void GenerateOperMod(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void GenerateOperPower(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void GenerateLogicOperIntegerArguments(const ExpressionModel& expr, const ExpressionNode& nodeleft, const ExpressionNode& noderight, const string& comment);
    void GenerateOperEqual(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void GenerateOperNotEqual(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void GenerateOperLess(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void GenerateOperGreater(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void GenerateOperLessOrEqual(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void GenerateOperGreaterOrEqual(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void GenerateOperAnd(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void GenerateOperOr(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
    void GenerateOperXor(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight);
private:
    void GenerateFuncAbs(const ExpressionModel& expr, const ExpressionNode& node);
    void GenerateFuncRnd(const ExpressionModel& expr, const ExpressionNode& node);
    void GenerateFuncPeek(const ExpressionModel& expr, const ExpressionNode& node);
    void GenerateFuncInp(const ExpressionModel& expr, const ExpressionNode& node);
    void GenerateFuncLen(const ExpressionModel& expr, const ExpressionNode& node);
    void GenerateFuncInkey(const ExpressionModel& expr, const ExpressionNode& node);
    void GenerateFuncPos(const ExpressionModel& expr, const ExpressionNode& node);
};
