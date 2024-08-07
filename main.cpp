
#include <cstdio>
#include <iostream>
#include <fstream>
#include <iomanip>

#include "main.h"

string g_infilename;    // Input file name
string g_outfilename;   // Output file name

bool g_quiet = false;   // Be quiet
bool g_tokenizeonly = false;  // Show tokenization and quit
bool g_parsingonly = false;  // Show parsing result and quit
bool g_validationonly = false;  // Show validation result and quit
bool g_showgeneration = false;

SourceModel g_source;
FinalModel g_final;

static int g_errorcount = 0;

void RegisterError()
{
    g_errorcount++;
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

        if ((int)j == expr.root)
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
    std::cout << "Line " << line.number << " " << line.statement.token.text << line.statement.token.symbol;
    if (line.statement.paramline > 0)
        std::cout << " " << line.statement.paramline;
    if (line.statement.ident.type != TokenTypeNone)
        std::cout << " ident:" << line.statement.ident.text;
    if (line.statement.args.size() > 0)
        std::cout << " args(" << line.statement.args.size() << ")";
    if (line.statement.params.size() > 0)
        std::cout << " params(" << line.statement.params.size() << ")";
    if (line.statement.variables.size() > 0)
        std::cout << " vars(" << line.statement.variables.size() << ")";
    if (line.statement.varexprs.size() > 0)
        std::cout << " varexs(" << line.statement.varexprs.size() << ")";
    if (line.statement.args.size() > 0)
    {
        for (size_t i = 0; i < line.statement.args.size(); i++)
        {
            ExpressionModel& expr = line.statement.args[i];
            PrintExpression(expr, i);
        }
    }
    if (line.statement.params.size() > 0)
    {
        for (size_t i = 0; i < line.statement.params.size(); i++)
        {
            Token& token = line.statement.params[i];
            std::cout << std::endl << std::setw(2) << "  par" << i << ": ";
            token.Dump(std::cout);
        }
    }
    if (line.statement.variables.size() > 0)
    {
        for (size_t i = 0; i < line.statement.variables.size(); i++)
        {
            VariableModel& var = line.statement.variables[i];
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
    if (line.statement.varexprs.size() > 0)
    {
        for (size_t i = 0; i < line.statement.varexprs.size(); i++)
        {
            VariableExpressionModel& var = line.statement.varexprs[i];
            std::cout << std::endl << std::setw(2) << "  varex" << i << ": ";
            std::cout << var.name;
            if (!var.args.empty())
            {
                for (size_t j = 0; j < line.statement.args.size(); j++)
                {
                    ExpressionModel& expr = var.args[j];
                    PrintExpression(expr, j, 2);
                }
            }
        }
    }
    std::cout << std::endl;
}

void ShowTokenization(Tokenizer& tokenizer)
{
    while (true)
    {
        Token token = tokenizer.GetNextToken();

        token.Dump(std::cout);
        std::cout << std::endl;

        if (token.type == TokenTypeEOT)
            break;
    }
}

void ShowParsing(Parser& parser)
{
    while (true)
    {
        SourceLineModel line = parser.ParseNextLine();
        if (line.number == 0)
            break;

        if (!line.error)
            std::cout << line.text << std::endl;
        PrintLineModel(line);

        g_source.lines.push_back(line);
    }
}

void ShowValidation(Validator& validator)
{
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

    if (g_tokenizeonly)
    {
        ShowTokenization(tokenizer);

        instream.close();
        return;
    }

    Parser parser(&tokenizer);

    if (g_parsingonly)
    {
        ShowParsing(parser);

        instream.close();
        return;
    }

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

    if (g_validationonly)
    {
        ShowValidation(validator);

        return;
    }

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

    Generator generator(&g_source, &g_final);
    g_errorcount = 0;
    while (generator.ProcessLine())
        ;
    for (size_t i = 0; i < g_final.lines.size(); i++)
    {
        string& intermed = g_final.lines[i];
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
            else if (_stricmp(arg + 1, "t") == 0 || _stricmp(arg, "--tokenizeonly") == 0)
                g_tokenizeonly = true;
            else if (_stricmp(arg + 1, "p") == 0 || _stricmp(arg, "--parsingonly") == 0)
                g_parsingonly = true;
            else if (_stricmp(arg + 1, "v") == 0 || _stricmp(arg, "--validationonly") == 0)
                g_validationonly = true;
            else if (_stricmp(arg + 1, "g") == 0 || _stricmp(arg, "--showgeneration") == 0)
                g_showgeneration = true;
            else
            {
                std::cerr << "Unknown option: " << arg << std::endl;
                exit(EXIT_FAILURE);
            }
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

    std::cout << std::endl;
    ProcessFiles();

    return EXIT_SUCCESS;
}
