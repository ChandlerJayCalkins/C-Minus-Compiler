#ifndef SEMANTICS_H
#define SEMANTICS_H

#include <string.h>
#include "ast.h"
#include "symbolTable.h"

extern int line; // for displaying what line errors occur on
extern int errors; // displays the number of errors
extern int warnings; // displays the number of warnings
extern const int maxChildren; // maximum number of children a TreeNode can have
extern TreeNode* ast; // abstract syntax tree
extern SymbolTable* symTable; // symbol table

// traverse the ast to check for some semantic errors
void semanticAnalysis();

// ---------- adding pre built functions to symbol table -----------

// creates tree nodes for built in C- functions and adds them to the symbol table
void addBuiltInFuncs();
// makes a tree node for an output function and its parameter and adds it to the symbol table
void addOutputFunc(std::string id, ExpType type);
// makes a tree node for an input function and adds it to the symbol table
void addInputFunc(std::string id, ExpType type);

// ---------- tree traversal / node type detection + handling

// traverse through the ast node by node recursively, constructing symbol table and checking for semantic errors
// newScope is to tell compound stmt nodes whether or not to create a new scope (no new scope if they are attached to function / while loop, etc)
// checkInit is to tell id nodes whether or not to check if they've been initialized yet for the warning (want them to not check when they're being assigned)
void traverseTree(TreeNode* node, std::string scopeName, bool checkInit);

// ---------- calling traverseTree() on children and siblings

// calls traverseTree on each of a node's children with an empty scopename and checkInit set to true
void callChildren(TreeNode* node);
// calls traverseTree on each of a node's children with a scopeName and checkInit set to true
void callChildren(TreeNode* node, std::string scopeName);
// calls traverseTree on each of a node's children with checkInit set to false on just child InitIgnoreIndex
void callChildren(TreeNode* node, int initIgnoreIndex);
// calls traverseTree on a node's sibling with empty scopeName if it has a sibling
void callSibs(TreeNode* node);
// calls traverseTree on a node's sibling with a scopeName if it has a sibling
void callSibs(TreeNode* node, std::string scopeName);

// ---------- node type handling ----------

// handles function declaration nodes
void handleFuncDecl(TreeNode* node);
// handles compound statement nodes
void handleCompound(TreeNode* node, std::string scopeName);
// handles if and while statement nodes
void handleIfWhile(TreeNode* node);
// handles for statement nodes
void handleFor(TreeNode* node);
// handles iteration range nodes
void handleRange(TreeNode* node);
// handles return statement nodes
void handleReturn(TreeNode* node);
// handles break statement nodes
void handleBreak(TreeNode* node);
// handles variable / parameter declaration nodes
void handleVarDecl(TreeNode* node, std::string scopeName);
// handles function call nodes
void handleCall(TreeNode* node);
// handles variable identifier nodes
void handleId(TreeNode* node, bool checkInit);
// handles constant value nodes
void handleConst(TreeNode* node);
// handles semantic checks for Inc (++) and Dec (--) operators
void handleIncOp(TreeNode* node);
// handles semantic checks for assignment operators like Assi (=) or Addas (+=)
void handleAssi(TreeNode* node, bool math);
// handles semantic checks for operators like addition (+) or greater than (>)
void handleBinaryOp(TreeNode* node, bool comparison);
// handles unary non-array operators (not, change-sign / negative (-), and random(?))
void handleUnary(TreeNode* node);
// handles the sizeof operator (*)
void handleSizeof(TreeNode* node);
// handles bracket operators (array[index])
void handleBracket(TreeNode* node, bool checkInit);

// ---------- node helper functions ----------

// returns whether or not a node is part of a constant expression or not
bool isConstExp(TreeNode* node);
// returns whether or not a node is a logical operator
bool isLogOp(TreeNode* node);
// return whether or not a node is a comparison operator
bool isCompOp(TreeNode* node);
// returns whether or not a node is a math operator
bool isMathOp(TreeNode* node);
// checks to make sure a function call node has the correct parameters compared to its declaration (dupe symbol)
void checkParms(TreeNode* node, TreeNode* dupe);
// given a lhs node, turns either lhs or rhs into the string " not" depending on whether or not the node is an array
void getArraySideStrings(TreeNode* node, char*& lhs, char*& rhs);

// ---------- error / warning messages ----------

// prints the error alert and line number of the error at the start of an error message and increments the error count
void printError(TreeNode* node);
// prints the warning alert and line number of the warning at the start of an warning message and increments the warning count
void printWarning(TreeNode* node);
// prints a debug error about not being able to find a duplicate tree node after an insert failed
void printSemNoDupeError();
// prints the symbol already declared error message
void printAlreadyDeclaredError(TreeNode* node, TreeNode* dupe);
// prints the symbol not declared error message
void printNotDeclaredError(TreeNode* node);
// prints the incorrect type on one side error
void printWrongTypeError(TreeNode* node, bool lhs);
// prints the incorrect type with a unary operator error
void printUnaryTypeError(TreeNode* node);
// prints the not matching types for an operator error
void printNotSameTypeError(TreeNode* node);
// prints the operation doesn't work on arrays error
void printNoArraysError(TreeNode* node);
// prints the not both sides are arrays error
void printBothArraysError(TreeNode* node);
// checks to see if a node is uninitialized or not
void checkForUninited(TreeNode* node, bool checkArray);
// prints the expecting boolean test condition error
void printTestTypeError(TreeNode* node, char* stmtType);
// prints the can't use array as test condition error
void printTestArrError(TreeNode* node, char* stmtType);
// prints the not type int in iter statement error
void printIterTypeError(TreeNode* node, TreeNode* child, int i);
// prints the can't use array in iter statement error
void printIterArrError(TreeNode* node, int i);
// prints the initializer expression for a variable is not a constant expression error
void printNotConstExpError(TreeNode* node);
// prints the initializer expression and variable types do not match error
void printInitTypeError(TreeNode* node);
// prints the initializer and variable must both be or not be arrays error
void printInitArrError(TreeNode* node);
// prints the uninitialized warning
void printUninitedWarning(TreeNode* node);

#endif