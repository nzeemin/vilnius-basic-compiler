
#include <cstdio>
#include <iostream>
#include <fstream>
#include <iomanip>

#include "main.h"

#ifdef __GNUC__
#define _stricmp    strcasecmp
#endif

string g_infilename;    // Input file name
string g_outfilename;   // Output file name

bool g_quiet = false;   // Be quiet
bool g_tokenizeonly = false;  // Show tokenization and quit
bool g_parsingonly = false;  // Show parsing result and quit
bool g_validationonly = false;  // Show validation result and quit
bool g_showgeneration = false;

SourceModel g_source;
FinalModel g_intermed;

static int g_errorcount = 0;

void RegisterError()
{
    g_errorcount++;
}

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

        if (token.type == TokenTypeEOT)
            break;
    }

    instream.close();
}

void PrintExpression(ExpressionModel& expr, int number, int indent = 1)
{
    std::cout << std::endl << std::setw(indent * 2) << "  exp" << number << ":";
    if (expr.IsEmpty())
    {
        std::cout << " empty";
        return;
    }

    std::cout << " root:" << expr.root;
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

void PrintLineModel(SourceLineModel& line)
{
    std::cout << "Line " << line.number << " " << line.statement.text << line.statement.symbol;
    if (line.paramline > 0)
        std::cout << " " << line.paramline;
    if (line.ident.type != TokenTypeNone)
        std::cout << " ident:" << line.ident.text;
    if (line.args.size() > 0)
        std::cout << " args(" << line.args.size() << ")";
    if (line.params.size() > 0)
        std::cout << " params(" << line.params.size() << ")";
    if (line.variables.size() > 0)
        std::cout << " vars(" << line.variables.size() << ")";
    if (line.args.size() > 0)
    {
        for (size_t i = 0; i < line.args.size(); i++)
        {
            ExpressionModel& expr = line.args[i];
            PrintExpression(expr, i);
        }
    }
    if (line.params.size() > 0)
    {
        for (size_t i = 0; i < line.params.size(); i++)
        {
            Token& token = line.params[i];
            std::cout << std::endl << std::setw(2) << "  par" << i << ": ";
            token.Dump(std::cout);
        }
    }
    if (line.variables.size() > 0)
    {
        for (size_t i = 0; i < line.variables.size(); i++)
        {
            VariableModel& var = line.variables[i];
            std::cout << std::endl << std::setw(2) << "  var" << i << ": ";
            std::cout << var.name;
            if (var.indices.size() > 0)
            {
                std::cout << "(";
                for (size_t j = 0; j < var.indices.size(); j++)
                {
                    if (j > 0) std::cout << ",";
                    std::cout << var.indices[j];
                }
                std::cout << ")";
            }
        }
    }
    std::cout << std::endl;
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

        if (!line.error)
            std::cout << line.text << std::endl;
        PrintLineModel(line);

        if (line.error)
            break;

        g_source.lines.push_back(line);
    }

    instream.close();

    //if (g_errorcount > 0)
    //    std::cout << std::endl << "ERRORS: " << g_errorcount << std::endl;
}

void ShowValidation()
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

        g_source.lines.push_back(line);
    }

    instream.close();

    Validator validator(&g_source);

    for (size_t i = 0; i < g_source.lines.size(); i++)
    {
        validator.ProcessLine();
        SourceLineModel& line = g_source.lines[i];
        std::cout << line.text << std::endl;
        PrintLineModel(line);
    }
}

void ProcessFiles()
{
    std::ifstream instream;
    instream.open(g_infilename);
    if (!instream.is_open())
    {
        std::cerr << "Failed to open the input file " + g_infilename << std::endl;
        exit(EXIT_FAILURE);
    }

    Tokenizer tokenizer(&instream);
    Parser parser(&tokenizer);
    g_errorcount = 0;
    while (true)
    {
        SourceLineModel line = parser.ParseNextLine();
        if (line.number == 0)
            break;

        g_source.lines.push_back(line);
    }
    if (g_errorcount > 0)
    {
        std::cerr << "Parsing ERRORS: " << g_errorcount << std::endl;
        exit(EXIT_FAILURE);
    }

    instream.close();

    Validator validator(&g_source);
    g_errorcount = 0;
    while (validator.ProcessLine())
        ;
    if (g_errorcount > 0)
    {
        std::cerr << "Validation ERRORS: " << g_errorcount << std::endl;
        exit(EXIT_FAILURE);
    }

    std::ofstream outstream;
    outstream.open(g_outfilename, std::ofstream::out | std::ofstream::trunc);
    if (!outstream.is_open())
    {
        std::cerr << "Failed to open the output file " << g_outfilename << std::endl;
        exit(EXIT_FAILURE);
    }
    outstream << "; Generated with vibasc [" << __DATE__ << "] on " << g_infilename << std::endl;
    outstream << ";" << std::endl;

    Generator generator(&g_source, &g_intermed);
    g_errorcount = 0;
    while (generator.ProcessLine())
        ;
    for (size_t i = 0; i < g_intermed.lines.size(); i++)
    {
        string& intermed = g_intermed.lines[i];
        outstream << intermed << std::endl;
        if (g_showgeneration)
            std::cout << intermed << std::endl;
    }
    if (g_errorcount > 0)
    {
        std::cerr << "Generation ERRORS: " << g_errorcount << std::endl;
        exit(EXIT_FAILURE);
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
            if (_stricmp(arg + 1, "t") == 0 || _stricmp(arg, "--tokenizeonly") == 0)
                g_tokenizeonly = true;
            if (_stricmp(arg + 1, "p") == 0 || _stricmp(arg, "--parsingonly") == 0)
                g_parsingonly = true;
            if (_stricmp(arg + 1, "v") == 0 || _stricmp(arg, "--validationonly") == 0)
                g_validationonly = true;
            if (_stricmp(arg + 1, "g") == 0 || _stricmp(arg, "--showgeneration") == 0)
                g_showgeneration = true;
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

    size_t dotpos = g_infilename.find_last_of('.');
    if (dotpos == string::npos)
        g_outfilename = g_infilename + ".MAC";
    else
        g_outfilename = g_infilename.substr(0, dotpos) + ".MAC";

    if (!g_quiet)
        std::cout << "vibasc  " << __DATE__ << std::endl;

    if (g_tokenizeonly)
    {
        std::cout << std::endl;
        ShowTokenization();
        return EXIT_SUCCESS;
    }
    if (g_parsingonly)
    {
        std::cout << std::endl;
        ShowParsing();
        return EXIT_SUCCESS;
    }
    if (g_validationonly)
    {
        std::cout << std::endl;
        ShowValidation();
        return EXIT_SUCCESS;
    }

    std::cout << std::endl;
    ProcessFiles();

    return EXIT_SUCCESS;
}
