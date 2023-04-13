
#include <cstdio>
#include <iostream>
#include <fstream>
#include <iomanip>

#include "main.h"


string g_infilename;

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
        if (line.gotoLine > 0)
            std::cout << " " << line.gotoLine;
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

int main(int argc, char* argv[])
{
    std::cout << "BasicCompiler  " << __DATE__ << std::endl;

    if (argc <= 1)
    {
        //print_help();
        std::cerr << "Input file not specified." << std::endl;
        exit(EXIT_FAILURE);
    }

    g_infilename = argv[1];

    //std::cout << std::endl;
    //ShowTokenization();
    
    std::cout << std::endl;
    ShowParsing();

    //std::cout << std::endl;
    //ShowGeneration();
}
