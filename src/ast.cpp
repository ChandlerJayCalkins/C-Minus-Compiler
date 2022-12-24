#include "ast.h"

// -------- children Setters

// sets all a node's children to NULL
void setChildren(TreeNode* node)
{
	// sets each child to NULL
	for (int i = 0; i < maxChildren; i++)
	{
		node->children[i] = NULL;
	}
}

// gives a node 1 child
void setChildren(TreeNode* node, TreeNode* c)
{
	// sets first child to c
	node->children[0] = c;
	// sets rest of children to NULL
	for (int i = 1; i < maxChildren; i++)
	{
		node->children[i] = NULL;
	}
}

// gives a node 2 children
void setChildren(TreeNode* node, TreeNode* c1, TreeNode* c2)
{
	// sets first 2 children
	node->children[0] = c1;
	node->children[1] = c2;
	// sets remaining children to NULL
	for (int i = 2; i < maxChildren; i++)
	{
		node->children[i] = NULL;
	}
}

// gives a node 3 children
void setChildren(TreeNode* node, TreeNode* c1, TreeNode* c2, TreeNode* c3)
{
	// sets first 3 children
	node->children[0] = c1;
	node->children[1] = c2;
	node->children[2] = c3;
	// sets remaining children to NULL
	for (int i = 3; i < maxChildren; i++)
	{
		node->children[i] = NULL;
	}
}

// --------

// initializses all of the other attributes of a node (sets sibling to NULL and used + inited to false)
void setAtts(TreeNode* node, NodeType nt, int l, ExpType et, OpKind op, bool ia, bool is)
{
	node->sibling = NULL;
	node->nodeType = nt;
	node->line = l;
	node->expType = et;
	node->opKind = op;
	node->isArray = ia;
	node->isStatic = is;
	node->used = false;
	node->inited = false;
	node->initWarned = false;
	node->returned = false;
	node->isIterVar = false;
	node->size = 0;
	node->memSpace = None;
	node->foffset = 0;
}

// sets the sibling pointer of a node
void addSib(TreeNode* node, TreeNode* sib)
{
	// if the starting node is null or the sibling is null AND there are no errors
	if (node == NULL || (sib == NULL && errors == 0))
	{
		return;
	}
	
	// go to the end of the sibling list and add the new sibling
	TreeNode* scout;
	for (scout = node; scout->sibling != NULL; scout = scout->sibling);
	scout->sibling = sib;
}

// returns a string of an ExpType
char* getTypeString(ExpType expType)
{
	char* str;

	switch (expType)
	{
		case Bool:
			str = (char*) "bool";
			break;
		case Int:
			str = (char*) "int";
			break;
		case Char:
			str = (char*) "char";
			break;
		case Void:
			str = (char*) "void";
			break;
		default:
			str = (char*) "undefined type";
	}

	return str;
}

// returns a string of a MemSpace
char* getMemSpaceString(MemSpace memSpace)
{
	char* str;

	switch (memSpace)
	{
		case Global:
			str = (char*) "Global";
			break;
		case Static:
			str = (char*) "LocalStatic";
			break;
		case Parameter:
			str = (char*) "Parameter";
			break;
		case Local:
			str = (char*) "Local";
			break;
		default:
			str = (char*) "None";
	}

	return str;
}

// if types are enabled, then print the type of the node
void printType(bool types, TreeNode* node)
{
	if (types)
	{
		printf(" of ");

		// if the node is static
		if (node->isStatic)
		{
			printf("static ");
		}

		// if the node is an array
		if (node->isArray)
		{
			printf("array of ");
		}

		printf("type %s", getTypeString(node->expType));
	}
}

// if memory info is enabled, then print the memory info of the node
void printMem(bool mem, TreeNode* node)
{
	if (mem)
	{
		printf(" [mem: %s loc: %d size: %d]", getMemSpaceString(node->memSpace), node->foffset, node->size);
	}
}

// prints out all of the nodes in the ast
void printAst(TreeNode* node, int level, char* relation, int childNum, int sibNum, bool types, bool mem)
{
	// print out dots and spaces for indenting levels of the tree
	for (int i = 0; i < level; i++)
	{
		printf(".   ");
	}
	
	// print if current node is a child or a sibling (or nothing if it's the root node);
	printf("%s", relation);
	
	// if current node is a child, print the child number
	if (childNum > -1)
	{
		printf(": %d ", childNum);
	}
	// if current node is a sibling, print the sibling number
	else if (sibNum > 0)
	{
		printf(": %d ", sibNum);
	}
	
	// print out data specific to the node type of the current node
	switch (node->nodeType)
	{
		// variable
		case Var:
			// print the name of the variable
			printf("Var: %s", node->value.str);
			printType(types, node);
			printMem(mem, node);
			break;
		// function
		case Func:
			// print the name and expression type of the node
			printf("Func: %s returns type %s", node->value.str, getTypeString(node->expType));
			printMem(mem, node);
			break;
		// parameter
		case Parm:
			// print the name of the parameter
			printf("Parm: %s", node->value.str);
			printType(types, node);
			printMem(mem, node);
			break;
		// if statement
		case If:
			printf("If");
			break;
		// compound statement
		case Compound:
			printf("Compound");
			printMem(mem, node);
			break;
		// while statement
		case While:
			printf("While");
			break;
		// for statement
		case For:
			printf("For");
			printMem(mem, node);
			break;
		// range statement
		case Range:
			printf("Range");
			break;
		// return statement
		case Return:
			printf("Return");
			break;
		// break statement
		case Break:
			printf("Break");
			break;
		// assignment operator
		case Assign:
			// print out the type of assignment operator the node is
			printf("Assign: %s", node->value.str);
			printType(types, node);
			break;
		// expression operator
		case Op:
			// print out the type of operator the node is
			printf("Op: %s", node->value.str);
			printType(types, node);
			break;
		// identifier
		case Id:
			printf("Id: %s", node->value.str);
			printType(types, node);
			printMem(mem, node);
			break;
		// function call
		case Call:
			printf("Call: %s", node->value.str);
			printType(types, node);
			break;
		// constant
		case Const:
			printf("Const ");
			// prints out the value of the constant
			switch (node->expType)
			{
				// if the node is an integer
				case Int:
					printf("%d", node->value.num);
					break;
				// if the node is a character or a string
				case Char:
					// if the node is a string
					if (node->isArray)
					{
						printf("%s", node->value.str);
					}
					// if the node is a single char
					else
					{
						printf("'%c'", node->value.ch);
					}
					break;
				// if the node is a bool
				case Bool:
					if (node->value.num)
					{
						printf("true");
					}
					else
					{
						printf("false");
					}
					break;
				default:
					printf("%s", node->value.str);
			}
			printType(types, node);
			// if the const is an array (a string), print the memory values if it's required
			if (node->isArray)
			{
				printMem(mem, node);
			}
			break;
		// only other defined node type is 
		default:
			printf("Unknown Node Type");
	}
	
	// print out the line of current node
	printf(" [line: %d]\n", node->line);
	
	// recursively prints out each of the children of current node
	for (int i = 0; i < maxChildren; i++)
	{
		if (node->children[i] != NULL)
		{
			printAst(node->children[i], level + 1, (char*) "Child", i, 0, types, mem);
		}
	}
	
	// recursively prints out each of the siblings of current node
	if (node->sibling != NULL)
	{
		printAst(node->sibling, level, (char*) "Sibling", -1, sibNum + 1, types, mem);
	}
}

// deallocates the ast
void deallocAst(TreeNode* node)
{
	if (node == NULL)
	{
		return;
	}

	// deallocates all of the node's children first
	for (int i = 0; i < maxChildren; i++)
	{
		if (node->children[i] != NULL)
		{
			deallocAst(node->children[i]);
		}
	}

	// deallocates all of the node's siblings first
	if (node->sibling != NULL)
	{
		deallocAst(node->sibling);
	}

	// deallocates the node itself, then goes back to deallocate the node's parent / previous sibling
	free(node);
}