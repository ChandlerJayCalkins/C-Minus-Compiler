#ifndef CODEGEN_H
#define CODEGEN_H

#include <fstream>
#include "ast.h"

// main function for generating code for the tiny machine vm
void generateCode(char* fileName);
// generates code and comments that go at the top of the output code file
void genHeader(char* fileName);
// traverses the ast to generate code
void traverseAST(TreeNode* node);
// generates inititalization code
void genInitCode();

// calls traverseAST on all of a node's children
void traverseChildren(TreeNode* node);
// calls traverseAST on a node's sibling if it has one
void traverseSib(TreeNode* node);

// generates code for a variable declaration
void genVarCode(TreeNode* node);
// generates code for a function declaration
void genFuncCode(TreeNode* node);
// generates code for a compound statement
void genCompoundCode(TreeNode* node);
// generates code for an if statement
void genIfCode(TreeNode* node);
// generates code for a while loop
void genWhileCode(TreeNode* node);
// generates code for a for loop
void genForCode(TreeNode* node);
// generates code for a break statement
void genBreakCode(TreeNode* node);
// generates code for a call statement
void genCallCode(TreeNode* node);
// generates code for a return statement
void genReturnCode(TreeNode* node);
// generates code for an increment operator
void genIncCode(TreeNode* node);
// generates code for an assign operator
void genAssiCode(TreeNode* node);
// generates code for binary operators
void genBinOpCode(TreeNode* node);
// generates code for unary operators
void genUnaryOpCode(TreeNode* node);
// generates code for a sizeof operator
void genSizeCode(TreeNode* node);
// generates code for a bracket operator
void genBrakCode(TreeNode* node);

#endif