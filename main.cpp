
#include <cstdio>
#include <iostream>
#include <fstream>
#include <iomanip>

#include "main.h"


string g_infilename;    // Input file name
bool g_quiet = false;   // Be quiet
bool g_showtokens = false;  // Show tokenization and quit
bool g_showparsing = false;  // Show parsing result and quit

SourceModel g_source;
IntermedModel g_intermed;


void ShowTokenization()
{
    std::ifstream instream;
    instream.open(g_infilename);
    if (!instream.is_open())
    {
        std::cerr << "Failed to open the Input file." << std::endl;
        exit(EXIT_FAILURE);
    }

    Tokenizer tokenizer(&instream);

    while (true)
    {
        Token token = tokenizer.GetNextToken();

        token.Dump(std::cout);
        std::cout << std::endl;

        if (token.type == TokenTypeEOF)
            break;
    }

    instream.close();
}

void PrintExpression(ExpressionModel& expr, int number, int indent = 1)
{
    std::cout << std::endl << std::setw(indent * 2) << "  " << number << ": root:" << expr.root;
    std::cout << " nodes(" << expr.nodes.size() << "): [";
    for (size_t j = 0; j < expr.nodes.size(); j++)
    {
        ExpressionNode& node = expr.nodes[j];
        std::cout << std::endl << std::setw(indent * 2 + 2) << "  " << j << ": ";
        if (!node.node.text.empty())
            std::cout << std::left << std::setw(6) << node.node.text;
        if (node.node.type == TokenTypeSymbol)
            std::cout << std::left << std::setw(6) << node.node.symbol;
        std::cout << " ";

        node.Dump(std::cout);

        if (j == expr.root)
            std::cout << " root";

        if (node.args.size() > 0)
        {
            std::cout << " args(" << node.args.size() << "): [";
            for (size_t i = 0; i < node.args.size(); i++)
            {
                ExpressionModel& exprin = node.args[i];
                PrintExpression(exprin, i, indent + 2);
            }
            std::cout << " ]";
        }
    }
    std::cout << " ]";
}

void ShowParsing()
{
    std::ifstream instream;
    instream.open(g_infilename);
    if (!instream.is_open())
    {
        std::cerr << "Failed to open the Input file." << std::endl;
        exit(EXIT_FAILURE);
    }

    Tokenizer tokenizer(&instream);

    Parser parser(&tokenizer);

    while (true)
    {
        SourceLineModel line = parser.ParseNextLine();
        if (line.number == 0)
            break;

        std::cout << "Line " << line.number << " " << line.statement.text;
        if (line.paramline > 0)
            std::cout << " " << line.paramline;
        if (line.ident.type != TokenTypeNone)
            std::cout << " ident:" << line.ident.text;
        if (line.args.size() > 0)
        {
            std::cout << " args(" << line.args.size() << "): [";
            for (size_t i = 0; i < line.args.size(); i++)
            {
                ExpressionModel& expr = line.args[i];
                PrintExpression(expr, i);
            }
            std::cout << " ]";
        }
        if (line.params.size() > 0)
        {
            std::cout << " params(" << line.params.size() << "): [";
            for (size_t i = 0; i < line.params.size(); i++)
            {
                Token& token = line.params[i];
                std::cout << std::endl << std::setw(2) << "  " << i << ": ";
                token.Dump(std::cout);
            }
            std::cout << " ]";
        }
        std::cout << std::endl;

        g_source.lines.push_back(line);
    }

    instream.close();
}

void ShowGeneration()
{
    Generator generator(&g_source, &g_intermed);

    while (generator.ProcessLine())
        ;

    for (size_t i = 0; i < g_intermed.intermeds.size(); i++)
    {
        string& intermed = g_intermed.intermeds[i];
        std::cout << intermed << std::endl;
    }
}

void ParseCommandLine(int argc, char** argv)
{
    for (int argn = 1; argn < argc; argn++)
    {
        const char* arg = argv[argn];
        
        if (*arg == '-'
#ifdef _MSC_VER
            || *arg == '/'
#endif
            )  // Parse options
        {
            if (_stricmp(arg + 1, "q") == 0 || _stricmp(arg, "--quiet") == 0)
                g_quiet = true;
            if (_stricmp(arg + 1, "t") == 0 || _stricmp(arg, "--showtokens") == 0)
                g_showtokens = true;
            if (_stricmp(arg + 1, "p") == 0 || _stricmp(arg, "--showparsing") == 0)
                g_showparsing = true;
        }
        else
        {
            g_infilename = arg;
        }
    }

    // Validate command line params
    if (g_infilename.empty())
    {
        //print_help();
        std::cerr << "Input file not specified." << std::endl;
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[])
{
    ParseCommandLine(argc, argv);

    if (!g_quiet)
        std::cout << "BasicCompiler  " << __DATE__ << std::endl;

    if (g_showtokens)
    {
        std::cout << std::endl;
        ShowTokenization();
        return EXIT_SUCCESS;
    }
    if (g_showparsing)
    {
        std::cout << std::endl;
        ShowParsing();
        return EXIT_SUCCESS;
    }

    //std::cout << std::endl;
    //ShowGeneration();
}
