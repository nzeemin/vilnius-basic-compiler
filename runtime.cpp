
#include <cassert>
#include <algorithm>

#include "main.h"


//////////////////////////////////////////////////////////////////////


const RuntimeGeneratorSymbolSpec RuntimeGenerator::m_symbolspecs[] =
{
    { RuntimeWRCHR, &RuntimeGenerator::GenerateWRCHR },
    { RuntimeWREOL, &RuntimeGenerator::GenerateWREOL },
    { RuntimeWRAT,  &RuntimeGenerator::GenerateWRAT },
    { RuntimeWRSTR, &RuntimeGenerator::GenerateWRSTR },
};

RuntimeGeneratorMethodRef RuntimeGenerator::FindRuntimeGeneratorMethodRef(RuntimeSymbol rtsymbol)
{
    for (auto it = std::begin(m_symbolspecs); it != std::end(m_symbolspecs); ++it)
    {
        if (rtsymbol == it->rtsymbol)
            return it->methodref;
    }
    return nullptr;
}


//////////////////////////////////////////////////////////////////////


RuntimeGenerator::RuntimeGenerator(const std::set<RuntimeSymbol>& needs, FinalModel* intermed)
    : m_needs(needs), m_final(intermed)
{
    assert(intermed != nullptr);
}

void RuntimeGenerator::GenerateRuntime()
{
    std::vector<RuntimeSymbol> listneeds;

    // First pass to collect all dependencies in m_needs
    std::copy(m_needs.begin(), m_needs.end(), std::back_inserter(listneeds));
    for (RuntimeSymbol rtsymbol : listneeds)
    {
        string rtsymbolname = GetRuntimeSymbolName(rtsymbol);

        // Find symbol generator implementation
        RuntimeGeneratorMethodRef methodref = FindRuntimeGeneratorMethodRef(rtsymbol);
        if (methodref != nullptr)
            (this->*methodref)();
    }

    //NOTE: Now we have all the dependencies in m_needs

    // Prepare for the second pass
    m_final->runtimelines.clear();
    listneeds.clear();
    
    // Second pass to actually generate all the runtime code
    std::copy(m_needs.begin(), m_needs.end(), std::back_inserter(listneeds));
    for (RuntimeSymbol rtsymbol : listneeds)
    {
        string rtsymbolname = GetRuntimeSymbolName(rtsymbol);
        AddLine("");
        //AddLine("; " + rtsymbolname);

        // Find symbol generator implementation
        RuntimeGeneratorMethodRef methodref = FindRuntimeGeneratorMethodRef(rtsymbol);
        if (methodref == nullptr)
        {
            //Error("Runtime generator for symbol " + rtsymbolname + " not found.");
            AddLine(rtsymbolname + "::");
            AddLine("; TODO: Runtime generator for symbol " + rtsymbolname + " not found.");
            AddLine("\tRETURN ;STUB");
            continue;
        }

        (this->*methodref)();
    }
}

void RuntimeGenerator::NeedRuntime(RuntimeSymbol rtsymbol)
{
    m_needs.insert(rtsymbol);
}

void RuntimeGenerator::GenerateWRCHR()
{
    AddLine("; Печать одного символа; R0 = символ");
    AddLine("WRCHR::");
    AddLine("20$:\tTSTB	@#177564\t; Источник канала 0 готов?");
    AddLine("\tBPL\t20$\t\t; нет => ждём");
    AddLine("\tMOV\tR0, @#177566\t; передаём символ в канал 0");
    AddLine("\tRETURN");
}

void RuntimeGenerator::GenerateWREOL()
{
    AddLine("; Печать перевода строки");
    AddLine("WREOL::");
    AddLine("\tMOV\t#CRLF, R0");
    AddLine("\tJMP\tWRSTR");
    AddLine("CRLF:\t.ASCIZ\t<2><015><012>");
    AddLine("\t.EVEN");
    NeedRuntime(RuntimeWRSTR);
}

void RuntimeGenerator::GenerateWRAT()
{
    AddLine("; Подпрограмма PRINT AT(C,R), перемещение курсора");
    AddLine("; R0 = колонка 0..255, R1 = строка 0..255");
    AddLine("WRAT::");
    //TODO: Нормировать R0
    //TODO: Нормировать R1
    //TODO
    AddLine(";TODO");
}

void RuntimeGenerator::GenerateWRSTR()
{
    AddLine("; Печать строки");
    AddLine("; R0 = адрес строки, первый байт строки содержит длину строки");
    AddLine("WRSTR::");
    AddLine("\tCLR\tR2");
    AddLine("\tBIS\t(R0)+, R2\t; берём длину строки");
    AddLine("\tBEQ\t9$\t\t; пустая строка => выходим");
    AddLine("1$:\tMOVB\t(R0)+, R1");
    AddLine("2$:\tTSTB\t@#177564\t; Источник канала 0 готов?");
    AddLine("\tBPL\t2$\t\t; нет => ждём");
    AddLine("\tMOV\tR1, @#177566\t; передаём символ в канал 0");
    AddLine("SOB\tR2, 1$");
    AddLine("9$:\tRETURN");
}


//////////////////////////////////////////////////////////////////////
