// generator-func.cpp - split from generator.cpp

#include <cassert>
#include <sstream>

#include "main.h"

// Function generation ///////////////////////////////////////////////

// X=CINT(<АРГУМЕНТ>)
// result is Integer
void Generator::GenerateFuncCint(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(expr.GetExpressionValueType() != ValueTypeString);
    assert(node.args.size() == 1);

    const ExpressionModel& expr1 = node.args[0];
    assert(expr1.GetExpressionValueType() != ValueTypeString);

    GenerateExpression(expr1);

    if (expr1.GetExpressionValueType() == ValueTypeInteger)
    {
        Warning(node.token, "CINT function call has no effect, value already has Integer type.");
        return;  // already Integer, the value is in R0 already
    }

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

    GenerateOperandAsSingle(expr1);

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

    GenerateOperandAsSingle(expr1);

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
    //NOTE: The sign of the argument decides what RND does, see 5.1.13, so only the
    //      exact zero goes here: RND(0.5) has a positive argument, not a zero one.
    if (expr1.IsConstExpression() && expr1.GetConstExpressionDValue() == 0.0)
    {
        AddLine("\tMOV\tRNDSAV+2, -(SP)\t; RND(0)");
        AddLine("\tMOV\tRNDSAV, -(SP)");
        m_runtimeneeds.insert(RuntimeFRND);
        return;
    }

    GenerateOperandAsSingle(expr1);

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
        int ivalue = ConstToInteger(expr1.GetConstExpressionDValue());
        AddLine("\tMOV\t@#" + std::to_string(ivalue) + "., R0" + comment);
        return;
    }
    else if (expr1.IsVariableExpression() && expr1.GetExpressionValueType() == ValueTypeInteger)
    {
        // Integer variable holds the address
        AddLine("\tMOV\t@" + expr1.GetVariableExpressionDecoratedName() + ", R0" + comment);
        return;
    }

    // The address parameter, Integer; Single converted to Integer
    GenerateOperandAsInteger(expr1);  // result in R0
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

    // Special cases for the const mask
    if (expr2.IsConstExpression())
    {
        //NOTE: The mask is a 16-bit value, so &HFFFF gives -1 here, see ParseDValue.
        int imask = ConstToInteger(expr2.GetConstExpressionDValue()) & 0xFFFF;

        if (imask == 0)  // Nothing left of the value, the result is always 0
        {
            Warning(node.token, "INP with mask 0 reduced to 0, consider to remove this INP.");
            AddLine("\tCLR\tR0\t; INP mask 0");
            return;
        }

        if (imask == 0xFFFF)  // The mask keeps all the bits, same as PEEK
        {
            Warning(node.token, "INP with mask 177777 does the same as PEEK, consider to use PEEK.");
            GenerateOperandAsInteger(expr1);  // R0 = address
            AddLine("\tMOV\t(R0), R0\t; INP");
            return;
        }
    }

    //TODO: Special case for const expression and variable expression
    GenerateOperandAsInteger(expr1);  // R0 = address

    AddLine("\tMOV\t(R0), R1\t; INP value");  // R1 = value

    GenerateOperandAsInteger(expr2);  // R0 = mask
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

    GenerateOperandAsSingle(expr1);

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

    GenerateOperandAsSingle(expr1);

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

    GenerateOperandAsSingle(expr1);

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

    GenerateOperandAsSingle(expr1);

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

    GenerateOperandAsSingle(expr1);

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

    GenerateOperandAsSingle(expr1);

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

    GenerateOperandAsSingle(expr1);

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

    GenerateOperandAsSingle(expr1);

    AddRuntimeCall(RuntimeFLOG, "log(X)");  // result on stack
}

// X¤=INKEY¤
// result is String
void Generator::GenerateFuncInkey(const ExpressionModel& expr, const ExpressionNode& node)
{
    //TODO: Rework to return dynamic String
    AddRuntimeCall(RuntimeINKEY);  // R0 = string address
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
    AddLine("\tTSTB\t(R1)+\t");  // check string length
    AddLine("\tBEQ\t.+4");
    AddLine("\tBISB\t(R1), R0\t; ASC");  // get first byte of the string
}

// X¤=CHR¤(<АРГУМЕНТ>)
// result is String
void Generator::GenerateFuncChr(const ExpressionModel& expr, const ExpressionNode& node)
{
    //TODO: Special case for const expression and variable expression
    const ExpressionModel& expr1 = node.args[0];
    assert(expr1.GetExpressionValueType() != ValueTypeString);

    GenerateExpression(expr1);
    if (expr1.GetExpressionValueType() == ValueTypeSingle)
        AddRuntimeCall(RuntimeFTOI, "to Integer");  // result in R0

    //TODO: Allocate dynamic 1-char string
    //TODO: String length = 1, MOVB R0 to first char of the string
    AddComment("TODO CHR$");
}

// X¤=STRING¤(<АРГУМЕНТ1>,<АРГУМЕНТ2>)
// result is String
void Generator::GenerateFuncString(const ExpressionModel& expr, const ExpressionNode& node)
{
    assert(node.args.size() == 2);

    const ExpressionModel& expr1 = node.args[0];
    assert(expr1.GetExpressionValueType() != ValueTypeString);

    const ExpressionModel& expr2 = node.args[1];

    if (expr1.IsConstExpression())
    {
        int ivalue = ConstToInteger(expr1.GetConstExpressionDValue());
        assert(ivalue >= 0 && ivalue <= 255);
        if (ivalue == 0)
        {
            Warning(node.token, "STRING$(0, ...) reduced to empty string; consider to replace this expression with \"\".");
            AddLine("\tMOV\tST0, R0");
            return;
        }

        GenerateExpression(expr2);
        if (expr2.GetExpressionValueType() == ValueTypeSingle)
            AddRuntimeCall(RuntimeFTOI, "to Integer");  // result in R0
        //TODO: If the string is empty, result is empty string (already in R0)

        //TODO: Allocate string with the given length
        //TODO: Fill the string
        AddComment("TODO STRING$");
        return;
    }

    if (expr2.IsConstExpression())
    {
        ValueType expr2vtype = expr2.GetExpressionValueType();
        if (expr2vtype == ValueTypeString && expr2.GetConstExpressionSValue() == "")
        {
            Warning(node.token, "STRING$(..., \"\") reduced to empty string; consider to replace this expression with \"\".");
            AddLine("\tMOV\tST0, R0");
            return;
        }

        GenerateExpression(expr1);
        if (expr1.GetExpressionValueType() == ValueTypeSingle)
            AddRuntimeCall(RuntimeFTOI, "to Integer");  // result in R0

        //TODO: Allocate string with the given length
        //TODO: Fill the string
        AddComment("TODO STRING$");
        return;
    }

    GenerateExpression(expr1);
    if (expr1.GetExpressionValueType() == ValueTypeSingle)
        AddRuntimeCall(RuntimeFTOI, "to Integer");  // result in R0

    GenerateExpression(expr2);
    if (expr2.GetExpressionValueType() == ValueTypeSingle)
        AddRuntimeCall(RuntimeFTOI, "to Integer");  // result in R0
    //TODO: If the string is empty, result is empty string (already in R0)

    //TODO: Allocate string with the given length
    //TODO: Fill the string
    AddComment("TODO STRING$");
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
