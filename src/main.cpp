#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scanType.h"
#include "ast.h"
#include "symbolTable.h"
#include "semantics.h"
#include "yyerror.h"
#include "codeGen.h"
#include "parser.tab.h" // Must include this after scanType.h and ast.h or this won't compile!!!

extern FILE* yyin; // input stream (file / terminal) to the scanner / parser
extern int yydebug; // flag that determines whether or not yacc debug info prints out during parsing process
extern int line; // error / debugging line number from the scanner
int errors = 0; // counts how many errors there are
int warnings = 0; // counts how many warnings there are
extern TreeNode* ast; // abstract syntax tree
SymbolTable* symTable; // symbol table

// gets the name of a file without the file extension and the directory path before it
char* getFileName(char* file)
{
	char* fileName = file;

	// find last dot and create substring of all characters that come before it
	for (int i = strlen(file) - 1; i >= 0; i--)
	{
		if (file[i] == '.')
		{
			fileName = (char*) malloc(sizeof(char) * (i+1));
			memcpy(fileName, file, i+1);
			fileName[i] = '\0';
			break;
		}
	}

	// find last slash and create substring of all characters that come after it
	for (int i = strlen(fileName) - 1; i >= 0; i--)
	{
		if (fileName[i] == '/')
		{
			char* temp = fileName;
			fileName = (char*) malloc(sizeof(char) * (strlen(temp) - i));
			memcpy(fileName, &temp[i+1], strlen(temp) - i);
			break;
		}
	}

	return fileName;
}

// starts the compilation process
int main(int argc, char* argv[])
{
	initErrorProcessing(); // initializes string table for parse error messages in yyerror.h
	symTable = new SymbolTable; // symbol table to keep track of identifiers and scopes during semantics checking
	bool printTree = false; // flag that determines whether or not the ast gets printed
	bool printTypeTree = false; // flag that determines if type info gets printed with the ast
	bool printMemTree = false; // flag that determines if the memory info gets printed with the ast
	bool fileOpened = true; // flag to set to false to prevent semanticAnalysis from being run if a file cannot be opened
	char* fileName;
	// loops through each argument
	for (int i = 1; i < argc; i++)
	{
		// if the argument is an option
		if (argv[i][0] == '-')
		{
			// prints a usage / help message
			if (strcmp(argv[i], "-h") == 0)
			{
				printf("usage: -c [options] [sourcefile]\n");
				printf("options:\n");
				printf("-d \t- turn on parser debugging\n");
				printf("-D \t- turn on symbol table debugging\n");
				printf("-h \t- print this usage message\n");
				printf("-p \t- print the abstract syntax tree\n");
				printf("-P \t- print the abstract syntax tree plus type information\n");
				printf("-M \t- print the abstract syntax tree plus type and memory information\n");
				return 0;
			}
			// enables ast printing
			else if (strcmp(argv[i], "-p") == 0)
			{
				printTree = true;
			}
			// enables printing with type info
			else if (strcmp(argv[i], "-P") == 0)
			{
				printTypeTree = true;

			}
			// enables yacc / bison debug info printing
			else if (strcmp(argv[i], "-d") == 0)
			{
				yydebug = 1;
			}
			// enables symbol table debug info printing
			else if (strcmp(argv[i], "-D") == 0)
			{
				symTable->debug(true);
			}
			// enables printing the memory info
			else if (strcmp(argv[i], "-M") == 0)
			{
				printMemTree = true;
			}
			// unknown option
			else
			{
				// string injection vulnerability with %s, but whatever this is a C- compiler, i'm not sanitizing inputs for this lol
				printf("'%s' is not a known option\n", argv[i]);
				printf("type '-h' for help\n");
			}
		}
		else
		{
			// else assumes the argument is the file to compile
			if (yyin = fopen(argv[i], "r"))
			{
				fileName = argv[i];
			}
			// if file doesn't open, throw error and exit program
			else
			{
				printf("ERROR(ARGLIST): source file \"%s\" could not be opened.\n", argv[i]);
				errors++;
				printTree = false;
				printTypeTree = false;
				fileOpened = false;
				break;
			}
		}
	}
	
	// if there wasn't a file opening error
	if (fileOpened)
	{
		line = 1;
		yyparse(); // begin scanning and parsing
	}
	
	// print the tree if the -p option was used
	if (printTree)
	{
		printAst(ast, 0, (char*) "", -1, 0, false, false);
	}

	// if there wasn't a file opening error and there were no syntax errors during parsing
	if (fileOpened && errors < 1)
	{
		// check the program for semantic errors
		semanticAnalysis();
	}

	// print the tree with types if the -P option was used and there aren't any errors
	if (printTypeTree && errors < 1)
	{
		printAst(ast, 0, (char*) "", -1, 0, true, false);
	}

	// print the tree with types and memory info if the -M option was used and there aren't any errors
	if (printMemTree && errors < 1)
	{
		printAst(ast, 0, (char*) "", -1, 0, true, true);
		printf("Offset for end of global space: %d\n", symTable->getCurrentFrameSize());
	}

	// prints the number of warnings and errors
	printf("Number of warnings: %d\n", warnings);
	printf("Number of errors: %d\n", errors);

	// if there are no errors in the program, generate the code for it
	if (errors < 1)
	{
		generateCode(getFileName(fileName));
	}
	
	// delete ast and symbol table
	if (fileOpened)
	{
		deallocAst(ast);
		delete symTable;
	}

	return 0;
}