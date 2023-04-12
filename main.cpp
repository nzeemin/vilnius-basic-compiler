
#include <cstdio>
#include <iostream>
#include <fstream>
#include <iomanip>

#include "main.h"


string infilename;

SourceModel g_source;
IntermedModel g_intermed;


void ShowTokenization()
{
    std::ifstream instream;
    instream.open(infilename);
    if (!instream.is_open())
    {
        std::cerr << "Failed to open the Input file." << std::endl;
        exit(EXIT_FAILURE);
    }

    Tokenizer tokenizer(&instream);

    while (true)
    {
        Token token = tokenizer.GetNextToken();
        if (token.type == TokenTypeEOF)
            break;

        switch (token.type)
        {
        case TokenTypeNumber:
            std::cout << "Number\t" << token.text << std::endl;
            break;
        case TokenTypeString:
            std::cout << "String\t" << token.text << std::endl;
            break;
        case TokenTypeKeyword:
            std::cout << "Keyword\t" << token.text << std::endl;
            break;
        case TokenTypeIdentifier:
            std::cout << "Ident\t" << token.text << std::endl;
            break;
        case TokenTypeDivider:
            std::cout << "Divider\t\'" << token.text << "\'" << std::endl;
            break;
        case TokenTypeSymbol:
            std::cout << "Symbol\t\'" << token.symbol << "\'" << std::endl;
            break;
        case TokenTypeEOL:
            std::cout << "EOL" << std::endl;
            break;
        }
    }

    instream.close();
}

void PrintExpression(ExpressionModel& expr, int number, int indent = 1)
{
    std::cout << std::endl << std::setw(indent * 2) << "  " << number << ": root:" << expr.root;
    std::cout << " nodes(" << expr.nodes.size() << "):[";
    for (size_t j = 0; j < expr.nodes.size(); j++)
    {
        ExpressionNode& node = expr.nodes[j];
        std::cout << std::endl << std::setw(indent * 2 + 2) << "  " << j << ": " << node.node.symbol << node.node.text;
        std::cout << "  ";
        if (node.left >= 0 || node.right >= 0)
        {
            if (node.left == -1)
                std::cout << "_";
            else
                std::cout << node.left;
            std::cout << ":";
            if (node.right == -1)
                std::cout << "_";
            else
                std::cout << node.right;
        }
        int pri = node.GetOperationPriority();
        if (pri > 0)
            std::cout << " !" << pri;
        if (node.brackets)
            std::cout << "  brackets";
        if (j == expr.root)
            std::cout << "  root";

        if (node.args.size() > 0)
        {
            std::cout << " args(" << node.args.size() << "):[";
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
    instream.open(infilename);
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
            std::cout << " args(" << line.args.size() << "):[";
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

    infilename = argv[1];

    //std::cout << std::endl;
    //ShowTokenization();
    
    std::cout << std::endl;
    ShowParsing();

    //std::cout << std::endl;
    //ShowGeneration();
}
