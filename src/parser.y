%{
// // // // // // // // // // // // // // // // // // // // // // // 
// CS445 - Calculator Example Program written in the style of the C-
// compiler for the class (Heavily modified for the actual C- compiler)
//
// Robert Heckendorn (original author)
// Jan 21, 2021
// 
// Chandler Calkins (modifying author)
// Fall 2022

#include <stdio.h>
#include <stdlib.h>
#include "scanType.h" // TokenData Type
#include "ast.h" // TreeNode Type
#include "yyerror.h" // handles yyerror parsing

TreeNode* ast = NULL; // the abstract syntax tree of the inputed program

extern int yylex();
extern FILE* yyin; // input stream (file / terminal) to the scanner / parser
extern int line; // error / debugging line number from the scanner
extern int errors; // counts how many errors there are
extern char* yytext;

// function that yacc runs when there's an error recognized in the program during parsing
#define YYERROR_VERBOSE // allows yyerror function to work

%}

// this is included in the tab.h file so scanType.h must be included before the tab.h file!!!!
// types of everything that appears in the grammar definition (yacc grammar code)
%union
{
	TokenData* tokenData;
	TreeNode* treeNode;
}

// tokens (terminals)
%token <tokenData> STRINGCONST CHARCONST INT BOOL CHAR INC DEC ADDASS SUBASS MULASS DIVASS LEQ GEQ EQ NEQ IF THEN ELSE WHILE FOR DO BREAK AND OR NOT STATIC
%token <tokenData> BY TO RETURN BOOLCONST ID NUMCONST SYMBOL '%' '*' '(' ')' '+' '=' '-' '{' '}' '[' ']' ':' ';' '<' '>' '?' ',' '/'

// symbols (nonterminals)
%type <treeNode> program declList decl varDecl scopedVarDecl varDeclList varDeclInit varDeclId typeSpec funDecl parms parmList parmTypeList parmIdList parmId
%type <treeNode> stmt expStmt compoundStmt localDecls stmtList closedStmt openStmt closedIfStmt openIfStmt closedWhileStmt openWhileStmt closedForStmt openForStmt
%type <treeNode> iterRange returnStmt breakStmt exp assignop simpleExp andExp unaryRelExp relExp relop sumExp sumop mulExp mulop unaryExp unaryop factor mutable
%type <treeNode> immutable call args argList constant

%%
program			: declList										{	ast = $1; }
				;

declList		: declList decl									{	$$ = $1;
																	// adds decl to the end of declList
																	addSib($$, $2);
																}
				| decl											{	$$ = $1; }
				;

decl			: varDecl										{	$$ = $1; }
				| funDecl										{	$$ = $1; }
				| error											{	$$ = NULL; }
				;

varDecl			: typeSpec varDeclList ';'						{	$$ = $2;
																	// change the exp type of all nodes in varDeclList to the type of typeSpec
																	for (TreeNode* scout = $$; scout != NULL; scout = scout->sibling)
																	{
																		scout->expType = $1->expType;
																	}
																	yyerrok;
																}
				| error varDeclList ';'							{	$$ = NULL; yyerrok; }
				| typeSpec error ';'							{	$$ = NULL; yyerrok; }
				;

scopedVarDecl	: STATIC typeSpec varDeclList ';'				{	$$ = $3;
																	// change the exp type of all nodes in varDeclList to the type of typeSpec and to be static
																	for (TreeNode* scout = $$; scout != NULL; scout = scout->sibling)
																	{
																		scout->expType = $2->expType;
																		scout->isStatic = true;
																		scout->memSpace = Static;
																	}
																	yyerrok;
																}
				| typeSpec varDeclList ';'						{	$$ = $2;
																	// change the exp type of all nodes in varDeclList to the type of typeSpec
																	for (TreeNode* scout = $$; scout != NULL; scout = scout->sibling)
																	{
																		scout->expType = $1->expType;
																	}
																	yyerrok;
																}
				| error varDeclList ';'							{	$$ = NULL; yyerrok; }
				;

varDeclList		: varDeclList ',' varDeclInit					{	$$ = $1;
																	// adds varDeclInit to the end of the varDeclList
																	addSib($$, $3);
																	yyerrok;
																}
				| varDeclInit									{	$$ = $1; }
				| varDeclList ',' error							{	$$ = NULL; }
				| error											{	$$ = NULL; }
				;

varDeclInit		: varDeclId										{	$$ = $1; }
				| varDeclId ':' simpleExp						{	$$ = $1;
																	if ($$ != NULL)
																	{
																		setChildren($$, $3);
																		$$->inited = true;
																	}
																}
				| error ',' simpleExp							{	$$ = NULL; yyerrok; }
				;

varDeclId		: ID											{	// creates new var node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Var, $1->linenum, Undefined, NotOp, false, false);
																	$$->value.str = $1->inputStr;
																	$$->size = 1;
																}
				| ID '[' NUMCONST ']'							{	// creates new var node that's an array
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Var, $1->linenum, Undefined, NotOp, true, false);
																	$$->value.str = $1->inputStr;
																	$$->size = $3->numValue + 1;
																}
				| ID '[' error									{	$$ = NULL; }
				| error ']'										{	$$ = NULL; yyerrok; }
				;

typeSpec		: BOOL											{	// creates new bool node that tells decls what type they are
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Type, $1->linenum, Bool, NotOp, false, false);
																}
				| CHAR											{	// creates new char node that tells decls what type they are
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Type, $1->linenum, Char, NotOp, false, false);
																}
				| INT											{	// creates new int node that tells decls what type they are
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Type, $1->linenum, Int, NotOp, false, false);
																}
				;

funDecl			: typeSpec ID '(' parms ')' compoundStmt		{	// creates new func node that is typeSpec's type and is the parent of parms and compoundStmt
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$, $4, $6);
																	setAtts($$, Func, $1->line, $1->expType, NotOp, false, false);
																	$$->value.str = $2->inputStr;
																	$$->memSpace = Global;
																}
				| ID '(' parms ')' compoundStmt					{	// creates new func node that's the parent of parms and compoundStmt
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$, $3, $5);
																	setAtts($$, Func, $1->linenum, Void, NotOp, false, false);
																	$$->value.str = $1->inputStr;
																	$$->memSpace = Global;
																}
				| typeSpec error								{	$$ = NULL; }
				| typeSpec ID '(' error							{	$$ = NULL; }
				| ID '(' error									{	$$ = NULL; }
				| ID '(' parms ')' error						{	$$ = NULL; }
				;

parms			: parmList										{	$$ = $1; }
				| %empty										{	$$ = NULL; }
				;

parmList		: parmList ';' parmTypeList						{	$$ = $1;
																	// adds parmTypeList to the end of the parmList
																	addSib($$, $3);
																}
				| parmTypeList									{	$$ = $1; }
				| parmList ';' error							{	$$ = NULL; }
				| error											{	$$ = NULL; }
				;

parmTypeList	: typeSpec parmIdList							{	$$ = $2;
																	// change the exp type of all nodes in parmIdList to the type of typeSpec
																	for (TreeNode* scout = $$; scout != NULL; scout = scout->sibling)
																	{
																		scout->expType = $1->expType;
																	}
																}
				| typeSpec error								{	$$ = NULL; }
				;

parmIdList		: parmIdList ',' parmId							{	$$ = $1;
																	// adds parmId to the end of the parmIdList
																	addSib($$, $3);
																	yyerrok;
																}
				| parmId										{	$$ = $1; }
				| parmIdList ',' error							{	$$ = NULL; }
				| error 										{	$$ = NULL; }
				;

parmId			: ID											{	// creaates new id node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Parm, $1->linenum, Undefined, NotOp, false, false);
																	$$->value.str = $1->inputStr;
																	$$->size = 1;
																	$$->memSpace = Parameter;
																}
				| ID '[' ']'									{	// creates new id node that is an array
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Parm, $1->linenum, Undefined, NotOp, true, false);
																	$$->value.str = $1->inputStr;
																	$$->size = 1;
																	$$->memSpace = Parameter;
																}
				| error ']'										{	$$ = NULL; yyerrok; }
				;

stmt			: closedStmt									{	$$ = $1; }
				| openStmt										{	$$ = $1; }
				;

closedStmt		: closedIfStmt									{	$$ = $1; }
				| closedWhileStmt								{	$$ = $1; }
				| closedForStmt									{	$$ = $1; }
				| expStmt										{	$$ = $1; }
				| compoundStmt									{	$$ = $1; }
				| returnStmt									{	$$ = $1; }
				| breakStmt										{	$$ = $1; }
				;

openStmt		: openIfStmt									{	$$ = $1; }
				| openWhileStmt									{	$$ = $1; }
				| openForStmt									{	$$ = $1; }
				;

closedIfStmt	: IF simpleExp THEN closedStmt ELSE closedStmt	{	// creates if node with matched else part and 3 children
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$, $2, $4, $6);
																	setAtts($$, If, $1->linenum, Undefined, NotOp, false, false);
																}
				| IF error										{	$$ = NULL; }
				| IF error ELSE closedStmt						{	$$ = NULL; yyerrok; }
				| IF error THEN closedStmt ELSE closedStmt		{	$$ = NULL; yyerrok; }
				;

openIfStmt		: IF simpleExp THEN stmt						{	// creates new if node no matched to an else part
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$, $2, $4);
																	setAtts($$, If, $1->linenum, Undefined, NotOp, false, false);
																}
				| IF simpleExp THEN closedStmt ELSE openStmt	{	// creates new if node matched to else part with matched part inside and unmatched inside
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$, $2, $4, $6);
																	setAtts($$, If, $1->linenum, Undefined, NotOp, false, false);
																}
				| IF error THEN stmt							{	$$ = NULL; yyerrok; }
				| IF error THEN closedStmt ELSE openStmt		{	$$ = NULL; yyerrok; }
				;

closedWhileStmt	: WHILE simpleExp DO closedStmt					{	// creates new while node that's the parent of the simpleExp and the closedStmt
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$, $2, $4);
																	setAtts($$, While, $1->linenum, Undefined, NotOp, false, false);
																}
				| WHILE error DO closedStmt						{	$$ = NULL; yyerrok; }
				| WHILE error									{	$$ = NULL; }
				;

openWhileStmt	: WHILE simpleExp DO openStmt					{	// creates new while node that's the parent of the simpleExp and the closedStmt
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$, $2, $4);
																	setAtts($$, While, $1->linenum, Undefined, NotOp, false, false);
																}
				;

closedForStmt	: FOR ID '=' iterRange DO closedStmt			{	// creates new for node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	// creates new id node
																	$$->children[0] = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$->children[0]);
																	setAtts($$->children[0], Var, $1->linenum, Int, NotOp, false, false);
																	$$->children[0]->size = 1;
																	$$->children[0]->value.str = $2->inputStr;
																	$$->children[0]->inited = true;
																	$$->children[0]->isIterVar = true;
																	// make the for node the parent of the new id node, the iterRange, and the closedStmt
																	setChildren($$, $$->children[0], $4, $6);
																	setAtts($$, For, $1->linenum, Undefined, NotOp, false, false);
																}
				| FOR ID '=' error DO closedStmt				{	$$ = NULL; yyerrok; }
				| FOR error										{	$$ = NULL; }
				;

openForStmt		: FOR ID '=' iterRange DO openStmt				{	// creates new for node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	// creates new id node
																	$$->children[0] = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$->children[0]);
																	setAtts($$->children[0], Var, $1->linenum, Int, NotOp, false, false);
																	$$->children[0]->size = 1;
																	$$->children[0]->value.str = $2->inputStr;
																	$$->children[0]->inited = true;
																	$$->children[0]->isIterVar = true;
																	// make the for node the parent of the new id node, the iterRange, and the closedStmt
																	setChildren($$, $$->children[0], $4, $6);
																	setAtts($$, For, $1->linenum, Undefined, NotOp, false, false);
																}
				;

expStmt			: exp ';'										{	$$ = $1; }
				| ';'											{	$$ = NULL; }
				| error ';'										{	$$ = NULL; yyerrok; }
				;

compoundStmt	: '{' localDecls stmtList '}'					{	// create new compound node with localDecls and stmtList as the children
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$, $2, $3);
																	setAtts($$, Compound, $1->linenum, Undefined, NotOp, false, false);
																	yyerrok;
																}
				;

localDecls		: localDecls scopedVarDecl						{	// if there's no localDecls before scopedVarDecl, make scopedVarDecl the head of localDecls
																	if ($1 == NULL)
																	{
																		$$ = $2;
																	}
																	// otherwise add scopedVarDecl to the end of the localDecls
																	else
																	{
																		$$ = $1;
																		addSib($$, $2);
																	}
																}
				| %empty										{	$$ = NULL; }
				;

stmtList		: stmtList stmt									{	// if there's no stmtList before stmt, make stmt the head of the stmtList
																	if ($1 == NULL)
																	{
																		$$ = $2;
																	}
																	// otherwise add stmt to the end of the stmtList
																	else
																	{
																		$$ = $1;
																		addSib($$, $2);
																	}
																}
				| %empty										{	$$ = NULL; }
				;

iterRange		: simpleExp TO simpleExp						{	// creates new range node that's the parent of the simpleExps before and after it
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$, $1, $3);
																	setAtts($$, Range, $1->line, Undefined, NotOp, false, false);
																}
				| simpleExp TO simpleExp BY simpleExp			{	// creates new range node that's the parent of the 3 simpleExps in this production
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$, $1, $3, $5);
																	setAtts($$, Range, $1->line, Undefined, NotOp, false, false);
																}
				| simpleExp TO error							{	$$ = NULL; }
				| error BY error								{	$$ = NULL; yyerrok; }
				| simpleExp TO simpleExp BY error				{	$$ = NULL; }
				;

returnStmt		: RETURN ';'									{	// creates new return stmt node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Return, $1->linenum, Undefined, NotOp, false, false);
																}
				| RETURN exp ';'								{	// creates new return stmt node that's the parent of the exp after it
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$, $2);
																	setAtts($$, Return, $1->linenum, Undefined, NotOp, false, false);
																	yyerrok;
																}
				| RETURN error ';'								{	$$ = NULL; yyerrok; }
				;

breakStmt		: BREAK ';'										{	// creates new break stmt node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Break, $1->linenum, Undefined, NotOp, false, false);
																}
				;

exp				: mutable assignop exp							{	$$ = $2;
																	// make assignment operator node parent of its variable and expression
																	setChildren($$, $1, $3);
																}
				| mutable INC									{	// creates new assignment operator node that's the parent of the mutable before it
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$, $1);
																	setAtts($$, Assign, $1->line, Int, Inc, false, false);
																	$$->value.str = (char*) "++";
																}
				| mutable DEC									{	// creates new assignment operator node that's the parent of the mutable before it
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$, $1);
																	setAtts($$, Assign, $1->line, Int, Dec, false, false);
																	$$->value.str = (char*) "--";
																}
				| simpleExp										{	$$ = $1; }
				| error assignop exp							{	$$ = NULL; yyerrok; }
				| mutable assignop error						{	$$ = NULL; }
				| error INC										{	$$ = NULL; yyerrok; }
				| error DEC										{	$$ = NULL; yyerrok; }
				;

assignop		: '='											{	// create new assignment operator node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Assign, $1->linenum, Undefined, Assi, false, false);
																	$$->value.str = (char*) "=";
																}
				| ADDASS										{	// create new assignment operator node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Assign, $1->linenum, Int, Addas, false, false);
																	$$->value.str = (char*) "+=";
																}
				| SUBASS										{	// create new assignment operator node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Assign, $1->linenum, Int, Subas, false, false);
																	$$->value.str = (char*) "-=";
																}
				| MULASS										{	// create new assignment operator node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Assign, $1->linenum, Int, Mulas, false, false);
																	$$->value.str = (char*) "*=";
																}
				| DIVASS										{	// create new assignment operator node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Assign, $1->linenum, Int, Divas, false, false);
																	$$->value.str = (char*) "/=";
																}
				;

simpleExp		: simpleExp OR andExp							{	// create new or operator node that's the parent of the exps before and after it
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$, $1, $3);
																	setAtts($$, Op, $2->linenum, Bool, Or, false, false); // SEG FAULT when $3 is NULL
																	$$->value.str = (char*) "or";
																}
				| andExp										{	$$ = $1; }
				| simpleExp OR error							{	$$ = NULL; }
				;

andExp			: andExp AND unaryRelExp						{	// create new and operator node that's the parent of the exps before and after it
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$, $1, $3);
																	setAtts($$, Op, $2->linenum, Bool, And, false, false);
																	$$->value.str = (char*) "and";
																}
				| unaryRelExp									{	$$ = $1; }
				| andExp AND error								{	$$ = NULL; }
				;

unaryRelExp		: NOT unaryRelExp								{	// create new not operator node that's the parent of the exp after it
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$, $2);
																	setAtts($$, Op, $1->linenum, Bool, Not, false, false);
																	$$->value.str = (char*) "not";
																}
				| relExp										{	$$ = $1; }
				| NOT error										{	$$ = NULL; }
				;

relExp			: sumExp relop sumExp							{	$$ = $2;
																	// make operator node the parent of its operands
																	setChildren($$, $1, $3);
																}
				| sumExp										{	$$ = $1; }
				| sumExp relop error							{	$$ = $1; }
				;

relop			: '<'											{	// create new operator node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Op, line, Bool, Less, false, false);
																	$$->value.str = (char*) "<";
																}
				| LEQ											{	// create new operator node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Op, $1->linenum, Bool, Leq, false, false);
																	$$->value.str = (char*) "<=";
																}
				| '>'											{	// create new operator node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Op, line, Bool, Gtr, false, false);
																	$$->value.str = (char*) ">";
																}
				| GEQ											{	// create new operator node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Op, $1->linenum, Bool, Geq, false, false);
																	$$->value.str = (char*) ">=";
																}
				| EQ											{	// create new operator node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Op, $1->linenum, Bool, Eq, false, false);
																	$$->value.str = (char*) "==";
																}
				| NEQ											{	// create new operator node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Op, $1->linenum, Bool, Neq, false, false);
																	$$->value.str = (char*) "!=";
																}
				;

sumExp			: sumExp sumop mulExp							{	$$ = $2;
																	// make operator node the parent of its operands
																	setChildren($$, $1, $3);
																}
				| mulExp										{	$$ = $1; }
				| sumExp sumop error							{	$$ = NULL; }
				;

sumop			: '+'											{	// create new operator node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Op, line, Int, Add, false, false);
																	$$->value.str = (char*) "+";
																}
				| '-'											{	// create new operator node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Op, line, Int, Sub, false, false);
																	$$->value.str = (char*) "-";
																}
				;

mulExp			: mulExp mulop unaryExp							{	$$ = $2;
																	// make operator node the parent of its operands
																	setChildren($$, $1, $3);
																}
				| unaryExp										{	$$ = $1; }
				| mulExp mulop error							{	$$ = NULL; }
				;

mulop			: '*'											{	// create new operator node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Op, line, Int, Mul, false, false);
																	$$->value.str = (char*) "*";
																}
				| '/'											{	// create new operator node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Op, line, Int, Div, false, false);
																	$$->value.str = (char*) "/";
																}
				| '%'											{	// create new operator node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Op, line, Int, Mod, false, false);
																	$$->value.str = (char*) "%";
																}
				;

unaryExp		: unaryop unaryExp								{	$$ = $1;
																	// make operator node the parent of its operand
																	setChildren($$, $2);
																}
				| factor										{	$$ = $1; }
				| unaryop error									{	$$ = NULL; }
				;

unaryop			: '-'											{	// create new operator node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Op, $1->linenum, Int, Neg, false, false);
																	$$->value.str = (char*) "chsign";
																}
				| '*'											{	// create new operator node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Op, $1->linenum, Int, Size, false, false);
																	$$->value.str = (char*) "sizeof";
																}
				| '?'											{	// create new operator node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Op, $1->linenum, Int, Rand, false, false);
																	$$->value.str = (char*) "?";
																}
				;

factor			: mutable										{	$$ = $1; }
				| immutable										{	$$ = $1; }
				;

mutable			: ID											{	// create new id node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Id, $1->linenum, Undefined, NotOp, false, false);
																	$$->value.str = $1->inputStr;
																}
				| ID '[' exp ']'								{	// create new '[' node and new id node, make new id nde and exp the '[' node's children
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	$$->children[0] = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$->children[0]);
																	setAtts($$->children[0], Id, $1->linenum, Undefined, NotOp, false, false);
																	$$->children[0]->value.str = $1->inputStr;
																	setChildren($$, $$->children[0], $3);
																	setAtts($$, Op, $1->linenum, Undefined, Brak, false, false);
																	$$->value.str = (char*) "[";
																}
				;

immutable		: '(' exp ')'									{	$$ = $2; yyerrok; }
				| call											{	$$ = $1; }
				| constant										{	$$ = $1; }
				| '(' error										{	$$ = NULL; }
				;

call			: ID '(' args ')'								{	// create new id node and make args its child
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$, $3);
																	setAtts($$, Call, $1->linenum, Undefined, NotOp, false, false);
																	$$->value.str = $1->inputStr;
																}
				| error '('										{	$$ = NULL; yyerrok; }
				;

args			: argList										{	$$ = $1; }
				| %empty										{	$$ = NULL; }
				;

argList			: argList ',' exp								{	$$ = $1;
																	addSib($$, $3);
																	yyerrok;
																}
				| exp											{	$$ = $1; }
				| argList ',' error								{	$$ = NULL; }
				;

constant		: NUMCONST										{	// create new int / num node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Const, $1->linenum, Int, NotOp, false, false);
																	$$->value.num = $1->numValue;
																	$$->size = 1;
																}
				| CHARCONST										{	// create new character node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Const, $1->linenum, Char, NotOp, false, false);
																	$$->value.ch = $1->valueChar;
																	$$->size = 1;
																}
				| STRINGCONST									{	// create new string node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Const, $1->linenum, Char, NotOp, true, false);
																	$$->value.str = $1->inputStr;
																	$$->size = $1->strLen + 1;
																	$$->memSpace = Global;
																}
				| BOOLCONST										{	// create new bool node
																	$$ = (TreeNode*) malloc(sizeof(TreeNode));
																	setChildren($$);
																	setAtts($$, Const, $1->linenum, Bool, NotOp, false, false);
																	$$->value.num = $1->numValue;
																	$$->size = 1;
																}
				;
%%

// c++ code can go here