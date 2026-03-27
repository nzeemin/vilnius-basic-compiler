
#include <cassert>
#include <algorithm>

#include "main.h"


//////////////////////////////////////////////////////////////////////


RuntimeGenerator::RuntimeGenerator(const std::set<RuntimeSymbol>& needs, FinalModel* intermed)
    : m_needs(needs), m_final(intermed)
{
    assert(intermed != nullptr);
}

void RuntimeGenerator::ParseRuntimeTemplate(std::istream* pInput)
{
    std::vector<string> lines;  // lines for the current block
    RuntimeSymbol blockrtsymbol = RuntimeNone;  // name for the current block
    std::vector<RuntimeSymbol> blockneeds;  // dependencies for the current block
    char buffer[256];
    bool preambule = true;
    while (!pInput->eof())
    {
        pInput->getline(buffer, sizeof(buffer));
        if (*buffer == 0)  // skip empty lines
            continue;
        string line(buffer);
        if (preambule)
        {
            if (line.find(";####") == std::string::npos)  // not start of block
                continue;  // skip this line
            preambule = false;  // start of the first block
        }

        if (line.find(";####") == 0)  // start of block
        {
            if (!lines.empty())
            {
                RuntimeBlock block;
                block.rtsymbol = blockrtsymbol;
                block.lines = lines;
                block.needs = blockneeds;
                m_rtblocks.push_back(block);
            }

            lines.clear();
            blockrtsymbol = RuntimeNone;
            blockneeds.clear();
        }
        else if (line.find(";## Need ") == 0)  // list of dependencies
        {
            string needname = line.substr(9);
            RuntimeSymbol needrtsymbol = FindRuntimeSymbolByName(needname);
            if (needrtsymbol == RuntimeNone)
            {
                std::cerr << "Runtime template parsing ERROR: unknown Need name " << needname << std::endl;
                RegisterError();
                return;
            }
            blockneeds.push_back(needrtsymbol);
        }
        else if (line.find(";## ") == 0)  // block name
        {
            string blockname = line.substr(4);
            blockrtsymbol = FindRuntimeSymbolByName(blockname);
            if (blockrtsymbol == RuntimeNone)
            {
                std::cerr << "Runtime template parsing ERROR: unknown block name " << blockname << std::endl;
                RegisterError();
                return;
            }
        }
        else
        {
            lines.push_back(line);
        }
    }
    if (!lines.empty())
    {
        RuntimeBlock block;
        block.lines = lines;
        m_rtblocks.push_back(block);
    }
}

RuntimeBlock RuntimeGenerator::FindRuntimeBlock(RuntimeSymbol rtsymbol)
{
    for (auto it = std::begin(m_rtblocks); it != std::end(m_rtblocks); ++it)
    {
        if (rtsymbol == it->rtsymbol)
            return *it;
    }
    return RuntimeBlock();  // empty block
}

void RuntimeGenerator::GenerateRuntime()
{
    std::vector<RuntimeSymbol> listneeds;
    std::copy(m_needs.begin(), m_needs.end(), std::back_inserter(listneeds));

    // First collect all dependencies in m_needs
    for (RuntimeSymbol rtsymbol : listneeds)
    {
        string rtsymbolname = GetRuntimeSymbolName(rtsymbol);

        // Find symbol block
        RuntimeBlock rtblock = FindRuntimeBlock(rtsymbol);
        if (rtblock.rtsymbol == RuntimeNone)
        {
            //TODO: error
        }

        for (RuntimeSymbol need : rtblock.needs)
            m_needs.insert(need);
        //TODO: this should be recursive
    }

    //NOTE: Now we have all the dependencies in m_needs

    // Prepare for the second pass
    listneeds.clear();
    
    // Generate all the runtime code
    std::copy(m_needs.begin(), m_needs.end(), std::back_inserter(listneeds));
    for (RuntimeSymbol rtsymbol : listneeds)
    {
        string rtsymbolname = GetRuntimeSymbolName(rtsymbol);
        m_final->AddRuntimeLine("");
        //AddLine("; " + rtsymbolname);

        // Find symbol block
        RuntimeBlock rtblock = FindRuntimeBlock(rtsymbol);
        if (rtblock.rtsymbol == RuntimeNone)
        {
            //Error("Runtime generator for symbol " + rtsymbolname + " not found.");
            AddLine(rtsymbolname + (g_turbo8 ? ":" : "::"));
            AddLine("; TODO: Runtime generator for symbol " + rtsymbolname + " not found.");
            AddLine("\tRETURN ;STUB");
            continue;
        }

        if (!g_turbo8)
            AddLine("\t.GLOBL\t" + rtsymbolname);

        // copy lines to final model
        for (string line : rtblock.lines)
            AddLine(line);
    }
}

void RuntimeGenerator::NeedRuntime(RuntimeSymbol rtsymbol)
{
    m_needs.insert(rtsymbol);
}


//////////////////////////////////////////////////////////////////////
