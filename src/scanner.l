%{
#include <string.h>
#include "scanType.h" // TokenData Type
#include "ast.h" // TreeNode Type
#include "parser.tab.h" // token definitions from the bison (NEEDS TO BE INCLUDED LAST)

extern int errors; // number of errors
extern int warnings; // number of warnings
int line; // current line the scanner is parsing / what line a token is on
char* lastToken; // keeps track of what the last token that was scanned was

// prints an error message that a char constant was empty
void printEmptyCharError(char* token)
{
	errors++;
	printf("ERROR(%d): Empty character %s. Characters ignored.\n", line, token);
}

// prints a warning that says a character constant token has too many characters in it and only the first character will be used
void printLongCharWarning(char* token)
{
	warnings++;
	printf("WARNING(%d): character is %d characters long and not a single character: '%s'. The first char will be used.\n", line, strlen(token)-2, token);
}

static int setValue(int linenum, int tokenClass, char* svalue)
{
	// store the string value of the token that was just scanned so yyerror can use it if it needs to
	lastToken = strdup(svalue);

	// create the tokenData object that will be used for parsing and semantic analysis of this token
	yylval.tokenData = new TokenData;
	
	// set some of the token data values
	yylval.tokenData->tokenClass = tokenClass;
	yylval.tokenData->linenum = linenum;
	yylval.tokenData->inputStr = strdup(svalue);  // duplicating string!!
	
	// if the token is a string constant
	switch (tokenClass)
	{
		case STRINGCONST:
		{
			yylval.tokenData->tokenName = (char*) "STRINGCONST";
			
			// scan for backslashes to format valueStr
			// remove invalid backslashes, replace valid ones with escape chars
			int inputLen = strlen(yylval.tokenData->inputStr);
			yylval.tokenData->valueStr = (char*) malloc(inputLen);
			int offset = 0;
			int i;
			for (i = 0; i < inputLen; i++)
			{
				if (svalue[i] == '\\')
				{
					i++;
					offset++;
					if (svalue[i] == 'n')
					{
						yylval.tokenData->valueStr[i-offset] = '\n';
					}
					else if (svalue[i] == '0')
					{
						yylval.tokenData->valueStr[i-offset] = '\0';
					}
					else
					{
						yylval.tokenData->valueStr[i-offset] = svalue[i];
					}
				}
				else
				{
					yylval.tokenData->valueStr[i-offset] = svalue[i];
				}
			}
			yylval.tokenData->valueStr[i-offset] = '\0';
			yylval.tokenData->strLen = inputLen - offset - 2;
			break;
		}
		// if the token is a char constant
		case CHARCONST:
		{
			yylval.tokenData->tokenName = (char*) "CHARCONST";

			// if the char doesn't have a backslash, just copy it over
			if (svalue[1] != '\\')
			{
				// if the char const is empty
				if (strlen(svalue) < 3)
				{
					printEmptyCharError(svalue);
					break;
				}

				yylval.tokenData->valueChar = svalue[1];

				// if the charconst has more than 1 char in it, print a warning
				if (strlen(svalue) > 3)
				{
					printLongCharWarning(svalue);
				}
			}
			// if the char does have a backslash
			else
			{
				// if the char const is empty
				if (strlen(svalue) < 4)
				{
					printEmptyCharError(svalue);
					break;
				}

				// if the char has an escape sequence char after the backslash, then copy over that escape sequence
				if (svalue[2] == 'n')
				{
					yylval.tokenData->valueChar = '\n';
				}
				else if (svalue[2] == '0')
				{
					yylval.tokenData->valueChar = '\0';
				}
				// if the char does not have an escape sequence char after the backslash, then just copy over the second char
				else
				{
					yylval.tokenData->valueChar = svalue[2];
				}

				// if the charconst has more than 1 char in it, print a warning
				if (strlen(svalue) > 4)
				{
					printLongCharWarning(svalue);
				}
			}
			yylval.tokenData->strLen = strlen(yylval.tokenData->inputStr) -2;
			break;
		}
		case INT:
		{
			yylval.tokenData->tokenName = (char*) "INT";
			break;
		}
		case BOOL:
		{
			yylval.tokenData->tokenName = (char*) "BOOL";
			break;
		}
		case CHAR:
		{
			yylval.tokenData->tokenName = (char*) "CHAR";
			break;
		}
		case INC:
		{
			yylval.tokenData->tokenName = (char*) "INC";
			break;
		}
		case DEC:
		{
			yylval.tokenData->tokenName = (char*) "DEC";
			break;
		}
		case ADDASS:
		{
			yylval.tokenData->tokenName = (char*) "ADDASS";
			break;
		}
		case SUBASS:
		{
			yylval.tokenData->tokenName = (char*) "SUBASS";
			break;
		}
		case MULASS:
		{
			yylval.tokenData->tokenName = (char*) "MULASS";
			break;
		}
		case DIVASS:
		{
			yylval.tokenData->tokenName = (char*) "DIVASS";
			break;
		}
		case LEQ:
		{
			yylval.tokenData->tokenName = (char*) "LEQ";
			break;
		}
		case GEQ:
		{
			yylval.tokenData->tokenName = (char*) "GEQ";
			break;
		}
		case EQ:
		{
			yylval.tokenData->tokenName = (char*) "EQ";
			break;
		}
		case NEQ:
		{
			yylval.tokenData->tokenName = (char*) "NEQ";
			break;
		}
		case IF:
		{
			yylval.tokenData->tokenName = (char*) "IF";
			break;
		}
		case THEN:
		{
			yylval.tokenData->tokenName = (char*) "THEN";
			break;
		}
		case ELSE:
		{
			yylval.tokenData->tokenName = (char*) "ELSE";
			break;
		}
		case WHILE:
		{
			yylval.tokenData->tokenName = (char*) "WHILE";
			break;
		}
		case FOR:
		{
			yylval.tokenData->tokenName = (char*) "FOR";
			break;
		}
		case DO:
		{
			yylval.tokenData->tokenName = (char*) "DO";
			break;
		}
		case BREAK:
		{
			yylval.tokenData->tokenName = (char*) "BREAK";
			break;
		}
		case AND:
		{
			yylval.tokenData->tokenName = (char*) "AND";
			break;
		}
		case OR:
		{
			yylval.tokenData->tokenName = (char*) "OR";
			break;
		}
		case NOT:
		{
			yylval.tokenData->tokenName = (char*) "NOT";
			break;
		}
		case STATIC:
		{
			yylval.tokenData->tokenName = (char*) "STATIC";
			break;
		}
		case BY:
		{
			yylval.tokenData->tokenName = (char*) "BY";
			break;
		}
		case TO:
		{
			yylval.tokenData->tokenName = (char*) "TO";
			break;
		}
		case RETURN:
		{
			yylval.tokenData->tokenName = (char*) "RETURN";
			break;
		}
		case BOOLCONST:
		{
			yylval.tokenData->tokenName = (char*) "BOOLCONST";
			yylval.tokenData->numValue = strcmp(svalue, "true") ? 0 : 1;
			break;
		}
		case ID:
		{
			yylval.tokenData->tokenName = (char*) "ID";
			break;
		}
		case NUMCONST:
		{
			yylval.tokenData->tokenName = (char*) "NUMCONST";
			yylval.tokenData->numValue = atoi(svalue);
			break;
		}
		default:
		{
			yylval.tokenData->tokenName = strdup(svalue);
			break;
		}
	}
	// return the tokenclass
	return tokenClass;
}

%}

%option noyywrap

%%
\/\/.*						;
\"(((\\)?[^\n\"])|\\\")*\"	{ return setValue(line, STRINGCONST, yytext); }
'(((\\)?[^\n'])|\\')*'		{ return setValue(line, CHARCONST, yytext); }
int							{ return setValue(line, INT, yytext); }
bool						{ return setValue(line, BOOL, yytext); }
char						{ return setValue(line, CHAR, yytext); }
\+\+						{ return setValue(line, INC, yytext); }
--							{ return setValue(line, DEC, yytext); }
\+=							{ return setValue(line, ADDASS, yytext); }
-=							{ return setValue(line, SUBASS, yytext); }
\*=							{ return setValue(line, MULASS, yytext); }
\/=							{ return setValue(line, DIVASS, yytext); }
\<=							{ return setValue(line, LEQ, yytext); }
>=							{ return setValue(line, GEQ, yytext); }
==							{ return setValue(line, EQ, yytext); }
!=							{ return setValue(line, NEQ, yytext); }
if							{ return setValue(line, IF, yytext); }
then						{ return setValue(line, THEN, yytext); }
else						{ return setValue(line, ELSE, yytext); }
while						{ return setValue(line, WHILE, yytext); }
for							{ return setValue(line, FOR, yytext); }
do							{ return setValue(line, DO, yytext); }
break						{ return setValue(line, BREAK, yytext); }
and							{ return setValue(line, AND, yytext); }
or							{ return setValue(line, OR, yytext); }
not							{ return setValue(line, NOT, yytext); }
static						{ return setValue(line, STATIC, yytext); }
by							{ return setValue(line, BY, yytext); }
to							{ return setValue(line, TO, yytext); }
return						{ return setValue(line, RETURN, yytext); }
true|false					{ return setValue(line, BOOLCONST, yytext); }
[a-zA-Z]+[a-zA-Z0-9]*		{ return setValue(line, ID, yytext); }
[0-9]+						{ return setValue(line, NUMCONST, yytext); }
[%*()+=\-{}[\]:;<>?,/]		{ return setValue(line, yytext[0], yytext); }
[\n\r\f]					{ line++; }
[ \t]						{ ; }
.							{ printf("ERROR(%d): Invalid or misplaced input character: '%c'. Character Ignored.\n", line, yytext[0]); lastToken = strdup(yytext); errors++; }
%%
