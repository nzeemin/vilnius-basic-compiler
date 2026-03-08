
#include <cassert>
#include <algorithm>

#include "main.h"


//////////////////////////////////////////////////////////////////////


const RuntimeGeneratorSymbolSpec RuntimeGenerator::m_symbolspecs[] =
{
    { RuntimeWRCHR, &RuntimeGenerator::GenerateWRCHR },
    { RuntimeWREOL, &RuntimeGenerator::GenerateWREOL },
};


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
        RuntimeGeneratorMethodRef methodref = nullptr;
        for (auto it = std::begin(m_symbolspecs); it != std::end(m_symbolspecs); ++it)
        {
            if (rtsymbol == it->rtsymbol)
            {
                methodref = it->methodref;
                break;
            }
        }

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
        RuntimeGeneratorMethodRef methodref = nullptr;
        for (auto it = std::begin(m_symbolspecs); it != std::end(m_symbolspecs); ++it)
        {
            if (rtsymbol == it->rtsymbol)
            {
                methodref = it->methodref;
                break;
            }
        }

        if (methodref == nullptr)
        {
            //Error("Runtime generator for symbol " + rtsymbolname + " not found.");
            AddLine(rtsymbolname + "::");
            AddLine("; TODO: Runtime generator for symbol " + rtsymbolname + " not found.");
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


//////////////////////////////////////////////////////////////////////
