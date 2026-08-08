// generator-oper.cpp - split from generator.cpp

#include <cassert>
#include <sstream>

#include "main.h"

// Operation generation //////////////////////////////////////////////

void Generator::GenerateOperPlus(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    const string comment = "\t; Operation \'+\'";

    // String + String
    if (nodeleft.vtype == ValueTypeString && noderight.vtype == ValueTypeString)
    {
        if (nodeleft.constval && nodeleft.GetConstStringValue() == "")
        {
            Warning(node.token, "Concatenation with empty string at left; consider to remove the useless concatenation.");
            GenerateExpression(expr, noderight);
            return;
        }

        if (noderight.constval)
        {
            if (noderight.GetConstStringValue() == "")
            {
                Warning(node.token, "Concatenation with empty string at right; consider to remove the useless concatenation.");
                GenerateExpression(expr, nodeleft);
                return;
            }

            //TODO
        }

        //TODO
        AddRuntimeCall(RuntimeSSAL, "allocate string");
        //TODO
        AddComment("TODO String + String");
        return;
    }

    assert(nodeleft.vtype != ValueTypeString);
    assert(noderight.vtype != ValueTypeString);

    // Single operands, result is Single
    if (nodeleft.vtype == ValueTypeSingle || noderight.vtype == ValueTypeSingle)
    {
        GenerateOperandAsSingle(expr, nodeleft);

        GenerateOperandAsSingle(expr, noderight);

        AddRuntimeCall(RuntimeFADD, "Operation \'+\'");  // result on stack
        return;
    }

    assert(nodeleft.vtype == ValueTypeInteger);
    assert(noderight.vtype == ValueTypeInteger);

    GenerateExpression(expr, nodeleft);  // result in R0

    // Convert "XXX + N" into INC/ADD
    if (nodeleft.vtype == ValueTypeInteger &&
        noderight.constval && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle))
    {
        int ivalue = ConstToInteger(noderight.token.dvalue);
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
        GenerateOperandAsSingle(expr, nodeleft);

        GenerateOperandAsSingle(expr, noderight);

        AddRuntimeCall(RuntimeFSUB, "Operation \'-\'");  // result on stack
        return;
    }

    // Code to calculate left sub-expression, with result in R0
    GenerateExpression(expr, nodeleft);

    // Convert "XXX - N" into DEC/SUB
    if (nodeleft.vtype == ValueTypeInteger &&
        noderight.constval && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle))
    {
        int ivalue = ConstToInteger(noderight.token.dvalue);
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

    const string comment = "Operation \'*\'";

    // Single operands
    if (nodeleft.vtype == ValueTypeSingle || noderight.vtype == ValueTypeSingle)
    {
        GenerateOperandAsSingle(expr, nodeleft);

        GenerateOperandAsSingle(expr, noderight);

        AddRuntimeCall(RuntimeFMUL, comment);  // result on stack
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
        AddRuntimeCall(RuntimeIMUL, comment);  // result in R0
        return;
    }

    GenerateExpression(expr, nodeleft);  // result in R0
    AddLine("\tMOV\tR0, -(SP)\t; PUSH R0");
    GenerateExpression(expr, noderight);
    AddLine("\tMOV\t(SP)+, R1");
    AddRuntimeCall(RuntimeIMUL, comment);  // result in R0
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

    GenerateOperandAsSingle(expr, nodeleft);

    GenerateOperandAsSingle(expr, noderight);

    AddRuntimeCall(RuntimeFDIV, "Operation \'/\'");  // result on stack
}

// resuilt is Integer
void Generator::GenerateOperDivInt(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    assert(nodeleft.vtype != ValueTypeString);
    assert(noderight.vtype != ValueTypeString);

    if (noderight.constval && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle))
    {
        int ivalue = ConstToInteger(noderight.token.dvalue);
        
        // Special case for some const values
        switch (ivalue)
        {
        case -1:
            GenerateOperandAsInteger(expr, nodeleft);  // result in R0
            AddLine("\tNEG\tR0\t; / -1");
            return;
        case 0:
            std::cerr << "ERROR in expression at " << node.token.line << ":" << node.token.pos << " - Didiver is zero." << std::endl;
            m_line->error = true;
            RegisterError();
            return;
        case 1:
            GenerateOperandAsInteger(expr, nodeleft);  // result in R0
            Warning(noderight.token, "Division by 1 reduced to nothing, consider to remove the division.");
            return;
        //NOTE: There is no special case for a power of two here. ASR and ASH shift the
        //      sign bit in, so they round down, while the integer division truncates
        //      towards zero: -5 \ 2 gives -2 and not -3. IDIV does that already.
        }

        // Const expression at right
        GenerateOperandAsInteger(expr, nodeleft);  // result in R0
        AddLine("\tMOV\tR0, R1");
        AddLine("\tMOV\t#" + std::to_string(ivalue) + "., R0");
    }
    //NOTE: Single variable takes two words, it cannot be moved to a register by one MOV,
    //      so only Integer goes here, Single is handled by the common code below.
    else if (noderight.token.type == TokenTypeIdentifier && noderight.vtype == ValueTypeInteger)
    {
        // Special case for variable at right
        //NOTE: IDIV needs the divider in R0 and the divided value in R1
        string deconame = DecorateVariableName(GetCanonicVariableName(noderight.token.text));
        GenerateOperandAsInteger(expr, nodeleft);  // result in R0
        AddLine("\tMOV\tR0, R1");
        AddLine("\tMOV\t" + deconame + ", R0");
    }
    else
    {
        GenerateOperandAsInteger(expr, nodeleft);  // result in R0
        AddLine("\tMOV\tR0, -(SP)\t; PUSH R0");
        GenerateOperandAsInteger(expr, noderight);  // result in R0
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
            std::cerr << "ERROR in expression at " << node.token.line << ":" << node.token.pos << " - MOD divider is zero." << std::endl;
            m_line->error = true;
            RegisterError();
            return;
        case 1:
            Warning(node.token, "MOD 1 reduced to 0; consider to remove this MOD.");
            AddLine("\tCLR\tR0\t; MOD 1");
            return;
        //NOTE: There is no special case for a power of two here. Masking the lower bits
        //      gives the positive remainder, while the remainder has to take the sign of
        //      the dividend: -5 MOD 4 gives -1 and not 3. IDIV does that already.
        }

        // Const expression at right
        GenerateOperandAsInteger(expr, nodeleft);  // result in R0
        AddLine("\tMOV\tR0, R1");
        AddLine("\tMOV\t#" + std::to_string(ivalue) + "., R0");
    }
    //NOTE: Single variable takes two words, it cannot be moved to a register by one MOV,
    //      so only Integer goes here, Single is handled by the common code below.
    else if (noderight.token.type == TokenTypeIdentifier && noderight.vtype == ValueTypeInteger)
    {
        // Variable at right
        //NOTE: IDIV needs the divider in R0 and the divided value in R1
        string deconame = DecorateVariableName(GetCanonicVariableName(noderight.token.text));
        GenerateOperandAsInteger(expr, nodeleft);  // result in R0
        AddLine("\tMOV\tR0, R1");
        AddLine("\tMOV\t" + deconame + ", R0");
    }
    else
    {
        GenerateOperandAsInteger(expr, nodeleft);  // result in R0
        AddLine("\tMOV\tR0, -(SP)\t; PUSH R0");
        GenerateOperandAsInteger(expr, noderight);  // result in R0
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

    // Special cases for a constant exponent of 0 or 1. Handles both Integer and
    // Single constants; the Single case also avoids the FPWF runtime call.
    if (noderight.constval)
    {
        double dexp = noderight.token.dvalue;
        if (dexp == 0)  // X ^ 0 => 1
        {
            Warning(noderight.token, "Power 0 reduced to 1; consider to remove the power.");
            AddLine("\tCLR\t-(SP)\t; ^ 0 reduced to 1.0");  // Single 1.0 on the stack
            AddLine("\tMOV\t#040200, -(SP)");
            return;
        }
        if (dexp == 1)  // X ^ 1 => X
        {
            Warning(noderight.token, "Power 1 reduced to nothing; consider to remove the power.");
            GenerateOperandAsSingle(expr, nodeleft);  // base as a Single on the stack
            return;
        }
    }

    // Single ^ Integer or Integer ^ Integer => call FPWI
    if (noderight.vtype == ValueTypeInteger)
    {
        // FPWI wants the base as a Single on the stack and the Integer exponent in R0
        GenerateOperandAsSingle(expr, nodeleft);  // base -> Single on the stack
        GenerateExpression(expr, noderight);  // exponent -> Integer in R0
        AddRuntimeCall(RuntimeFPWI, comment);  // result on stack
    }
    // Single ^ Single or Integer ^ Single => call FPWF
    else if (noderight.vtype == ValueTypeSingle)
    {
        // FPWF wants the exponent on top of the stack and the base below it:
        // on entry (SP+2)(SP+4) = exponent, (SP+6)(SP+10) = base.
        GenerateOperandAsSingle(expr, nodeleft);  // base -> deeper on the stack
        GenerateOperandAsSingle(expr, noderight);  // exponent -> on top of the stack
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
                int ivalue = ConstToInteger(noderight.token.dvalue);
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
        assert(noderight.vtype == ValueTypeString);  // Compare String <=> Integer/Single should be covered in validation

        AddRuntimeCall(RuntimeSTCM);
        AddComment("TODO compare String to String");
        //TODO
    }
}

void Generator::GenerateOperEqual(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    // Special case: String equals empty String
    if (nodeleft.vtype == ValueTypeString && noderight.vtype == ValueTypeString &&
        noderight.constval && noderight.GetConstStringValue() == "")
    {
        GenerateExpression(expr, nodeleft);  // result in R0
        AddLine("\tTSTB\t(R0)");
    }
    // Special case: empty String equals String
    else if (nodeleft.vtype == ValueTypeString && noderight.vtype == ValueTypeString &&
        nodeleft.constval && nodeleft.GetConstStringValue() == "")
    {
        GenerateExpression(expr, noderight);  // result in R0
        AddLine("\tTSTB\t(R0)");
    }
    // Special case: String equals one-char String
    else if (nodeleft.vtype == ValueTypeString && noderight.vtype == ValueTypeString &&
        noderight.constval && noderight.GetConstStringValue().size() == 1)
    {
        GenerateExpression(expr, nodeleft);  // result in R0
        string svalue = noderight.GetConstStringValue();
        //NOTE: Character conversion depends on encoding
        uint16_t value = (1 << 8) | svalue[0];
        AddLine("\tCMP\t(R0), #" + to_string_octal(value));
    }
    else  // all other cases
    {
        GenerateLogicOperArguments(expr, nodeleft, noderight);
    }

    AddLine("\tBEQ\t.+6\t; Operation \'=\'");
    AddLine("\tCLR\tR0\t; false");
    AddLine("\tBR\t.+6");
    AddLine("\tMOV\t#-1, R0\t; true");
}

void Generator::GenerateOperNotEqual(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    // Special case: String <> empty String
    if (nodeleft.vtype == ValueTypeString && noderight.vtype == ValueTypeString &&
        noderight.constval && noderight.GetConstStringValue() == "")
    {
        GenerateExpression(expr, nodeleft);  // result in R0
        AddLine("\tTSTB\t(R0)");
    }
    // Special case: empty String <> String
    else if (nodeleft.vtype == ValueTypeString && noderight.vtype == ValueTypeString &&
        nodeleft.constval && nodeleft.GetConstStringValue() == "")
    {
        GenerateExpression(expr, noderight);  // result in R0
        AddLine("\tTSTB\t(R0)");
    }
    // Special case: String <> 1-char String
    else if (nodeleft.vtype == ValueTypeString && noderight.vtype == ValueTypeString &&
        noderight.constval && noderight.GetConstStringValue().size() == 1)
    {
        GenerateExpression(expr, nodeleft);  // result in R0
        string svalue = noderight.GetConstStringValue();
        //NOTE: Character conversion depends on encoding
        uint16_t value = (1 << 8) | svalue[0];
        AddLine("\tCMP\t(R0), #" + to_string_octal(value));
    }
    // all other cases
    else
    {
        GenerateLogicOperArguments(expr, nodeleft, noderight);
    }

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
        ConstToInteger(nodeleft.token.dvalue) == 0)
    {
        Warning(node.token, "AND operation with 0 reduced to 0; consider to remove the useless AND");
        AddLine("\tCLR\tR0\t; 0 AND xxx");
        return;
    }
    // Special case: xxx AND 0, result is 0
    if (nodeleft.vtype != ValueTypeString &&
        noderight.constval && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle) &&
        ConstToInteger(noderight.token.dvalue) == 0)
    {
        Warning(node.token, "AND operation with 0 reduced to 0; consider to remove the useless AND");
        AddLine("\tCLR\tR0\t; xxx AND 0");
        return;
    }

    // Special case: -1 AND xxx, result is xxx
    if (noderight.vtype != ValueTypeString &&
        nodeleft.constval && (nodeleft.vtype == ValueTypeInteger || nodeleft.vtype == ValueTypeSingle) &&
        ConstToInteger(nodeleft.token.dvalue) == -1)
    {
        Warning(node.token, "AND operation with -1 reduced to no operation; consider to remove the useless AND");
        GenerateOperandAsInteger(expr, noderight);
        return;
    }
    // Special case: xxx AND -1, result is xxx
    if (nodeleft.vtype != ValueTypeString &&
        noderight.constval && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle) &&
        ConstToInteger(noderight.token.dvalue) == -1)
    {
        Warning(node.token, "AND operation with -1 reduced to no operation; consider to remove the useless AND");
        GenerateOperandAsInteger(expr, nodeleft);
        return;
    }

    // Left part is constant, let's calculate right part first
    if (nodeleft.constval &&
        (nodeleft.vtype == ValueTypeInteger || nodeleft.vtype == ValueTypeSingle))
    {
        GenerateOperandAsInteger(expr, noderight);
        int ivalue = ~ConstToInteger(nodeleft.token.dvalue);  // inverted to use with BIC
        AddLine("\tBIC\t#" + std::to_string(ivalue) + "., R0" + comment);
        return;
    }
    // Right part is constant
    if (noderight.constval &&
        (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle))
    {
        GenerateOperandAsInteger(expr, nodeleft);
        int ivalue = ~ConstToInteger(noderight.token.dvalue);  // inverted to use with BIC
        AddLine("\tBIC\t#" + std::to_string(ivalue) + "., R0" + comment);
        return;
    }

    // Both right and left parts are not constant
    GenerateOperandAsInteger(expr, nodeleft);  // result in R0
    AddLine("\tCOM\tR0");  // invert for BIC
    AddLine("\tMOV\tR0, -(SP)\t; PUSH");
    GenerateOperandAsInteger(expr, noderight);  // result in R0
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
        ConstToInteger(nodeleft.token.dvalue) == -1)
    {
        Warning(node.token, "OR operation with -1 reduced to -1; consider to remove the useless OR");
        AddLine("\tMOV\t#-1, R0\t; -1 OR xxx");
        return;
    }
    // Special case: xxx OR -1, result is -1
    if (nodeleft.vtype != ValueTypeString &&
        noderight.constval && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle) &&
        ConstToInteger(noderight.token.dvalue) == -1)
    {
        Warning(node.token, "OR operation with -1 reduced to -1; consider to remove the useless OR");
        AddLine("\tMOV\t#-1, R0\t; xxx OR -1");
        return;
    }

    // Special case: 0 OR xxx, result is xxx
    if (noderight.vtype != ValueTypeString &&
        nodeleft.constval && (nodeleft.vtype == ValueTypeInteger || nodeleft.vtype == ValueTypeSingle) &&
        ConstToInteger(nodeleft.token.dvalue) == 0)
    {
        Warning(node.token, "OR operation with 0 reduced to no operation; consider to remove the useless OR");
        GenerateOperandAsInteger(expr, noderight);
        return;
    }
    // Special case: xxx OR 0, result is xxx
    if (nodeleft.vtype != ValueTypeString &&
        noderight.constval && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle) &&
        ConstToInteger(noderight.token.dvalue) == 0)
    {
        Warning(node.token, "OR operation with 0 reduced to no operation; consider to remove the useless OR");
        GenerateOperandAsInteger(expr, nodeleft);
        return;
    }

    // Left part is constant, let's calculate right part first
    if (nodeleft.constval &&
        (nodeleft.vtype == ValueTypeInteger || nodeleft.vtype == ValueTypeSingle))
    {
        GenerateOperandAsInteger(expr, noderight);
        int ivalue = ConstToInteger(nodeleft.token.dvalue);
        AddLine("\tBIS\t#" + std::to_string(ivalue) + "., R0" + comment);
        return;
    }
    // Right part is constant
    if (noderight.constval &&
        (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle))
    {
        GenerateOperandAsInteger(expr, nodeleft);
        int ivalue = ConstToInteger(noderight.token.dvalue);
        AddLine("\tBIS\t#" + std::to_string(ivalue) + "., R0" + comment);
        return;
    }

    // Both right and left parts are not constant
    GenerateOperandAsInteger(expr, nodeleft);  // result in R0
    AddLine("\tMOV\tR0, -(SP)\t; PUSH");
    GenerateOperandAsInteger(expr, noderight);  // result in R0
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
        ConstToInteger(nodeleft.token.dvalue) == 0)
    {
        Warning(node.token, "XOR operation with 0 reduced to no operation; consider to remove the useless XOR");
        GenerateOperandAsInteger(expr, noderight);
        return;
    }
    // Special case: xxx XOR 0, result is xxx
    if (nodeleft.vtype != ValueTypeString &&
        noderight.constval && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle) &&
        ConstToInteger(noderight.token.dvalue) == 0)
    {
        Warning(node.token, "XOR operation with 0 reduced to no operation; consider to remove the useless XOR");
        GenerateOperandAsInteger(expr, nodeleft);
        return;
    }

    // Special case: -1 XOR xxx, result same as NOT xxx
    if (noderight.vtype != ValueTypeString &&
        nodeleft.constval && (nodeleft.vtype == ValueTypeInteger || nodeleft.vtype == ValueTypeSingle) &&
        ConstToInteger(nodeleft.token.dvalue) == -1)
    {
        Warning(node.token, "XOR operation with -1 reduced to inversion; consider to replace XOR with NOT");
        GenerateOperandAsInteger(expr, noderight);
        AddLine("\tCOM\tR0\t; xxx XOR -1");
        return;
    }
    // Special case: xxx XOR -1, result same as NOT xxx
    if (nodeleft.vtype != ValueTypeString &&
        noderight.constval && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle) &&
        ConstToInteger(noderight.token.dvalue) == -1)
    {
        Warning(node.token, "XOR operation with -1 reduced to inversion; consider to replace XOR with NOT");
        GenerateOperandAsInteger(expr, nodeleft);
        AddLine("\tCOM\tR0\t; xxx XOR -1");
        return;
    }

    // Left part is constant, let's calculate right part first
    if (nodeleft.constval &&
        (nodeleft.vtype == ValueTypeInteger || nodeleft.vtype == ValueTypeSingle))
    {
        GenerateOperandAsInteger(expr, noderight);
        int ivalue = ConstToInteger(nodeleft.token.dvalue);
        AddLine("\tMOV\t#" + std::to_string(ivalue) + "., R1");
        AddLine("\tXOR\tR1, R0" + comment);  // XOR works only from register
        return;
    }
    // Right part is constant
    if (noderight.constval &&
        (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle))
    {
        GenerateOperandAsInteger(expr, nodeleft);
        int ivalue = ConstToInteger(noderight.token.dvalue);
        AddLine("\tMOV\t#" + std::to_string(ivalue) + "., R1");
        AddLine("\tXOR\tR1, R0" + comment);  // XOR works only from register
        return;
    }

    // Both right and left parts are not constant
    GenerateOperandAsInteger(expr, nodeleft);  // result in R0
    AddLine("\tMOV\tR0, -(SP)\t; PUSH");
    GenerateOperandAsInteger(expr, noderight);  // result in R0
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
        ConstToInteger(nodeleft.token.dvalue) == 0)
    {
        Warning(node.token, "EQV operation with 0 reduced to inversion; consider to replace EQV with NOT");
        GenerateOperandAsInteger(expr, noderight);
        AddLine("\tCOM\tR0\t; 0 EQV xxx");
        return;
    }
    // Special case: xxx EQV 0, result is NOT xxx
    if (nodeleft.vtype != ValueTypeString &&
        noderight.constval && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle) &&
        ConstToInteger(noderight.token.dvalue) == 0)
    {
        Warning(node.token, "EQV operation with 0 reduced to inversion; consider to replace EQV with NOT");
        GenerateOperandAsInteger(expr, nodeleft);
        AddLine("\tCOM\tR0\t; xxx EQV 0");
        return;
    }

    // Special case: -1 EQV xxx, result is xxx
    if (noderight.vtype != ValueTypeString &&
        nodeleft.constval && (nodeleft.vtype == ValueTypeInteger || nodeleft.vtype == ValueTypeSingle) &&
        ConstToInteger(nodeleft.token.dvalue) == -1)
    {
        Warning(node.token, "EQV operation with -1 reduced to no operation; consider to remove the useless EQV");
        GenerateOperandAsInteger(expr, noderight);
        return;
    }
    // Special case: xxx EQV -1, result is xxx
    if (nodeleft.vtype != ValueTypeString &&
        noderight.constval && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle) &&
        ConstToInteger(noderight.token.dvalue) == -1)
    {
        Warning(node.token, "EQV operation with -1 reduced to no operation; consider to remove the useless EQV");
        GenerateOperandAsInteger(expr, nodeleft);
        return;
    }

    // Left part is constant, let's calculate right part first
    if (nodeleft.constval &&
        (nodeleft.vtype == ValueTypeInteger || nodeleft.vtype == ValueTypeSingle))
    {
        GenerateOperandAsInteger(expr, noderight);
        int ivalue = ConstToInteger(nodeleft.token.dvalue);
        AddLine("\tMOV\t#" + std::to_string(ivalue) + "., R1");
        AddLine("\tXOR\tR1, R0");  // XOR works only from register
        AddLine("\tCOM\tR0" + comment);
        return;
    }
    // Right part is constant
    if (noderight.constval &&
        (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle))
    {
        GenerateOperandAsInteger(expr, nodeleft);
        int ivalue = ConstToInteger(noderight.token.dvalue);
        AddLine("\tMOV\t#" + std::to_string(ivalue) + "., R1");
        AddLine("\tXOR\tR1, R0");  // XOR works only from register
        AddLine("\tCOM\tR0" + comment);
        return;
    }

    // Both right and left parts are not constant
    GenerateOperandAsInteger(expr, nodeleft);  // result in R0
    AddLine("\tMOV\tR0, -(SP)\t; PUSH");
    GenerateOperandAsInteger(expr, noderight);  // result in R0
    AddLine("\tMOV\t(SP)+, R1\t; POP");
    AddLine("\tXOR\tR1, R0");  // XOR works only from register
    AddLine("\tCOM\tR0" + comment);
}

// X IMP Y == NOT(X) OR Y
void Generator::GenerateOperImp(const ExpressionModel& expr, const ExpressionNode& node, const ExpressionNode& nodeleft, const ExpressionNode& noderight)
{
    assert(nodeleft.vtype != ValueTypeString);
    assert(noderight.vtype != ValueTypeString);

    const string comment = "\t; Operation \'IMP\'";

    // Special case: 0 IMP xxx, result is -1
    if (noderight.vtype != ValueTypeString &&
        nodeleft.constval && (nodeleft.vtype == ValueTypeInteger || nodeleft.vtype == ValueTypeSingle) &&
        ConstToInteger(nodeleft.token.dvalue) == 0)
    {
        Warning(node.token, "IMP operation with 0 at left reduced to -1; consider to remove the useless IMP");
        AddLine("\tMOV\t#-1., R0\t; 0 IMP xxx");
        return;
    }
    // Special case: xxx IMP -1, result is -1
    if (nodeleft.vtype != ValueTypeString &&
        noderight.constval && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle) &&
        ConstToInteger(noderight.token.dvalue) == -1)
    {
        Warning(node.token, "IMP operation with -1 at right reduced to -1; consider to remove the useless IMP");
        AddLine("\tMOV\t#-1., R0\t; xxx IMP -1");
        return;
    }

    // Special case: -1 IMP xxx, result is xxx
    if (noderight.vtype != ValueTypeString &&
        nodeleft.constval && (nodeleft.vtype == ValueTypeInteger || nodeleft.vtype == ValueTypeSingle) &&
        ConstToInteger(nodeleft.token.dvalue) == -1)
    {
        Warning(node.token, "IMP operation with -1 at left reduced to no operation; consider to remove the useless IMP");
        GenerateOperandAsInteger(expr, noderight);
        return;
    }
    // Special case: xxx IMP 0, result same as NOT xxx
    if (nodeleft.vtype != ValueTypeString &&
        noderight.constval && (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle) &&
        ConstToInteger(noderight.token.dvalue) == 0)
    {
        Warning(node.token, "IMP operation with 0 at right reduced to inversion; consider to replace IMP with NOT");
        GenerateOperandAsInteger(expr, nodeleft);
        AddLine("\tCOM\tR0\t; xxx IMP 0");
        return;
    }

    // Left part is constant: NOT(const) is a constant too, so it's a single BIS
    if (nodeleft.constval &&
        (nodeleft.vtype == ValueTypeInteger || nodeleft.vtype == ValueTypeSingle))
    {
        GenerateOperandAsInteger(expr, noderight);
        int ivalue = ~(ConstToInteger(nodeleft.token.dvalue));
        AddLine("\tBIS\t#" + std::to_string(ivalue) + "., R0" + comment);
        return;
    }
    // Right part is constant
    if (noderight.constval &&
        (noderight.vtype == ValueTypeInteger || noderight.vtype == ValueTypeSingle))
    {
        GenerateOperandAsInteger(expr, nodeleft);
        int ivalue = ConstToInteger(noderight.token.dvalue);
        AddLine("\tCOM\tR0");
        AddLine("\tBIS\t#" + std::to_string(ivalue) + "., R0" + comment);
        return;
    }

    // Both right and left parts are not constant
    GenerateOperandAsInteger(expr, nodeleft);  // result in R0
    AddLine("\tMOV\tR0, -(SP)\t; PUSH");
    GenerateOperandAsInteger(expr, noderight);  // result in R0
    AddLine("\tMOV\t(SP)+, R1\t; POP");
    AddLine("\tCOM\tR1");
    AddLine("\tBIS\tR1, R0" + comment);
}

