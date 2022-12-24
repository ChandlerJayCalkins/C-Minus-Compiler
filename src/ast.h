#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <stdio.h>

extern int errors; // number of errors
const int maxChildren = 3; // max number of children a tree node can have

// the type of a tree node
typedef enum
{
	Var, Type, Func, Parm, Compound, If, While, For, Range, Return, Break, Assign, Op, Id, Call, Const
}
NodeType;

// the expression type of a tree node
typedef enum
{
	Void, Int, Bool, Char, Undefined
}
ExpType;

// kind of operator that a node is (assignment ops, multiplication ops, sum ops, and unary ops) (and open square bracket too for some reason?)
typedef enum
{
	NotOp, Inc, Dec, Assi, Addas, Subas, Mulas, Divas, Or, And, Not, Less, Leq, Gtr, Geq, Eq, Neq, Add, Sub, Mul, Div, Mod, Neg, Size, Rand, Brak
}
OpKind;

// the value of a node
union Attribute
{
	int num; // number / integer value of the node
	unsigned char ch; // character value of the node
	char* str; // string value of the node for string consts, char arrays, operators, and the names of identifiers
};

// the type of place in memory where something will be stored in code generation
typedef enum
{
	Global, Static, Parameter, Local, None
}
MemSpace;

// node of an abstract syntax tree
typedef struct TreeNode
{
	struct TreeNode* children[maxChildren]; // array of children of this node
	struct TreeNode* sibling; // the next sibling of this node
	
	NodeType nodeType; // the type of this node (defined by the enum above)
	int line; // the line of the first token in this node
	ExpType expType; // the expression type of this node (defined by the enum above)
	OpKind opKind; // type of operator that a node is
	bool isArray; // whether or not this node is an array variable
	bool isStatic; // whether or not this node is a static variable
	bool used; // whether or not a symbol node has been used yet
	bool inited; // whether or not the symbol has been initialized yet
	bool initWarned; // whether or not the symbol has been warned about not being initialized yet
	bool returned; // whether or not a function has a return statement somewhere in it
	bool isIterVar; // whether or not this is an interation variable for a for loop

	int size; // the number of data words that something will be taking up
	MemSpace memSpace; // the type of place in memory where this thing will be stored in code generation
	int foffset; // offset of this thing from the current memory frame
	
	Attribute value; // the value that this node holds (defined by the union above)
}
TreeNode;

// assign children nodes to a node
void setChildren(TreeNode* node);
void setChildren(TreeNode* node, TreeNode* c);
void setChildren(TreeNode* node, TreeNode* c1, TreeNode* c2);
void setChildren(TreeNode* node, TreeNode* c1, TreeNode* c2, TreeNode* c3);

// initializes all of the other attributes of a node (sets sibling to NULL)
void setAtts(TreeNode* node, NodeType nt, int l, ExpType et, OpKind op, bool ia, bool is);

// sets the sibling pointer of a node
void addSib(TreeNode* node, TreeNode* sib);

// returns a string of an ExpType
char* getTypeString(ExpType expType);
// returns a string of a MemSpace
char* getMemSpaceString(MemSpace memSpace);

// prints out all of the nodes in the ast
void printAst(TreeNode* node, int level, char* relation, int childNum, int sibNum, bool types, bool mem);
// if types are enabled, then print the type of the node
void printType(bool types, TreeNode* node);
// if memory info is enabled, then print the memory info of the node
void printMem(bool mem, TreeNode* node);

// deallocates the ast
void deallocAst(TreeNode* node);

#endif
