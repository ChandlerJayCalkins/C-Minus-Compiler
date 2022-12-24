#include "semantics.h"

static TreeNode* currentFunc = NULL;

// traverse the ast and use a symbol table to check for some semantic errors
void semanticAnalysis()
{
	addBuiltInFuncs();

	// go through the tree node by node to check for semantic errors
	traverseTree(ast, "", true);

	// if there isn't a function named main, print error message
	TreeNode* mainFunc = symTable->lookup("main");
	if (mainFunc == NULL || mainFunc->nodeType != Func || mainFunc->children[0] != NULL)
	{
		printf("ERROR(LINKER): A function named 'main' with no parameters must be defined.\n");
		errors++;
	}

	// check for unused symbols in global scope
	symTable->checkGlobalUnused();
}

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------ adding pre built functions to the symbol table ------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

// creates tree nodes for built in C- functions and adds them to the symbol table
void addBuiltInFuncs()
{
	// add output functions to symbol table
	addOutputFunc("output", Int);
	addOutputFunc("outputb", Bool);
	addOutputFunc("outputc", Char);

	// add input functions to symbol table
	addInputFunc("input", Int);
	addInputFunc("inputb", Bool);
	addInputFunc("inputc", Char);

	// makee tree node for outnl() function
	TreeNode* func = (TreeNode*) malloc(sizeof(TreeNode));
	setChildren(func);
	setAtts(func, Func, -1, Void, NotOp, false, false);
	func->value.str = (char*) "outnl";

	// add outnl() to symbol table
	symTable->insertGlobal(func->value.str, func);
}

// makes a tree node for an output function and its parameter and adds it to the symbol table
void addOutputFunc(std::string id, ExpType type)
{
	// makes tree node for parameter
	TreeNode* dummy = (TreeNode*) malloc(sizeof(TreeNode));
	setChildren(dummy);
	setAtts(dummy, Parm, -1, type, NotOp, false, false);
	dummy->value.str = (char*) "*dummy*";

	// makes tree node for function itself
	TreeNode* func = (TreeNode*) malloc(sizeof(TreeNode));
	setChildren(func, dummy);
	setAtts(func, Func, -1, Void, NotOp, false, false);
	func->value.str = strdup(id.c_str());

	// adds function to symbol table
	symTable->insertGlobal(func->value.str, func);
}

// makes a tree node for an input function and adds it to the symbol table
void addInputFunc(std::string id, ExpType type)
{
	// makes tree node for function
	TreeNode* func = (TreeNode*) malloc(sizeof(TreeNode));
	setChildren(func);
	setAtts(func, Func, -1, type, NotOp, false, false);
	func->value.str = strdup(id.c_str());

	// adds function to symbol table
	symTable->insertGlobal(func->value.str, func);
}

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------ tree traversal / node type detection + handling -----------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

// traverse through the ast node by node recursively, constructing symbol table and checking for semantic errors
// newScope is to tell compound stmt nodes whether or not to create a new scope (no new scope if they are attached to function / while loop, etc)
// checkInit is to tell id nodes whether or not to check if they've been initialized yet for the warning (want them to not check when they're being assigned)
void traverseTree(TreeNode* node, std::string scopeName, bool checkInit)
{
	if (node == NULL)
	{
		return;
	}

	// handle the node depending on what type it is
	switch (node->nodeType)
	{
		// function node
		case Func:
			handleFuncDecl(node);
			break;
		// compound statement node
		case Compound:
			handleCompound(node, scopeName);
			break;
		// if statement node
		case If:
			handleIfWhile(node);
			break;
		// while loop node
		case While:
			handleIfWhile(node);
			break;
		// for loop node
		case For:
			handleFor(node);
			break;
		// iteration range node
		case Range:
			handleRange(node);
			break;
		// return statement
		case Return:
			handleReturn(node);
			break;
		// break statement
		case Break:
			handleBreak(node);
			break;
		// variable declaration node
		case Var:
			handleVarDecl(node, "");
			break;
		// parameter declaration node
		case Parm:
			handleVarDecl(node, scopeName);
			break;
		// function call node
		case Call:
			handleCall(node);
			break;
		// variable identifier node
		case Id:
			handleId(node, checkInit);
			break;
		// constant value nodes
		case Const:
			handleConst(node);
			break;
		// assignment operator node
		case Assign:
			switch (node->opKind)
			{
				// increment operator (++)
				case Inc:
					handleIncOp(node);
					break;
				// decrement operator (--)
				case Dec:
					handleIncOp(node);
					break;
				// assignment operator (=)
				case Assi:
					handleAssi(node, false);
					break;
				// add assign operator (+=)
				case Addas:
					handleAssi(node, true);
					break;
				// subtract assign operator (-=)
				case Subas:
					handleAssi(node, true);
					break;
				// multiply assign operator (*=)
				case Mulas:
					handleAssi(node, true);
					break;
				// divide assign operator (/=)
				case Divas:
					handleAssi(node, true);
					break;
				default:
					// traverse this node's children and sibling(s)
					callChildren(node, true);
					callSibs(node, scopeName);
			}
			break;
		// operator node
		case Op:
			switch (node->opKind)
			{
				// or operator
				case Or:
					handleBinaryOp(node, false);
					break;
				// and operator
				case And:
					handleBinaryOp(node, false);
					break;
				// not operator
				case Not:
					handleUnary(node);
					break;
				// less than operator (<)
				case Less:
					handleBinaryOp(node, true);
					break;
				// less than or equal operator (<=)
				case Leq:
					handleBinaryOp(node, true);
					break;
				// greater than operator (>)
				case Gtr:
					handleBinaryOp(node, true);
					break;
				// greater than or equal to operator (>=)
				case Geq:
					handleBinaryOp(node, true);
					break;
				// equal to operator (==)
				case Eq:
					handleBinaryOp(node, true);
					break;
				// not equal to operator (!=)
				case Neq:
					handleBinaryOp(node, true);
					break;
				// add operator (+)
				case Add:
					handleBinaryOp(node, false);
					break;
				// subtract operator (-)
				case Sub:
					handleBinaryOp(node, false);
					break;
				// multiply operator (*)
				case Mul:
					handleBinaryOp(node, false);
					break;
				// divide operator (/)
				case Div:
					handleBinaryOp(node, false);
					break;
				// mod operator (%)
				case Mod:
					handleBinaryOp(node, false);
					break;
				// negative / change sign operator (-)
				case Neg:
					handleUnary(node);
					break;
				// size of operator (*)
				case Size:
					handleSizeof(node);
					break;
				// random integer operator (?)
				case Rand:
					handleUnary(node);
					break;
				// bracket operator ([])
				case Brak:
					handleBracket(node, checkInit);
					break;
				default:
					// traverse this node's children and sibling(s)
					callChildren(node, scopeName);
					callSibs(node, scopeName);
			}
			break;
		default:
			// go to the next nodes by default
			callChildren(node, scopeName);
			callSibs(node, scopeName);
	}
}

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------ calling traverseTree() on children and siblings -----------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

// calls traverseTree on each of a node's children with an empty scopename and checkInit set to true
void callChildren(TreeNode* node)
{
	for (int i = 0; i < maxChildren; i++)
	{
		if (node->children[i] != NULL)
		{
			traverseTree(node->children[i], "", true);
		}
	}
}

// calls traverseTree on each of a node's children with a scopeName and checkInit set to true
void callChildren(TreeNode* node, std::string scopeName)
{
	for (int i = 0; i < maxChildren; i++)
	{
		if (node->children[i] != NULL)
		{
			traverseTree(node->children[i], scopeName, true);
		}
	}
}

// calls traverseTree on each of a node's children with checkInit set to false on just child InitIgnoreIndex
void callChildren(TreeNode* node, int initIgnoreIndex)
{
	for (int i = 0; i < maxChildren; i++)
	{
		if (node->children[i] != NULL)
		{
			if (i == initIgnoreIndex)
			{
				traverseTree(node->children[i], "", false);
			}
			else
			{
				traverseTree(node->children[i], "", true);
			}
		}
	}
}

// calls traverseTree on a node's sibling with empty scopeName if it has a sibling
void callSibs(TreeNode* node)
{
	// if the node has a sibling
	if (node->sibling != NULL)
	{
		traverseTree(node->sibling, "", true);
	}
}

// calls traverseTree on a node's sibling with a scopeName if it has a sibling
void callSibs(TreeNode* node, std::string scopeName)
{
	// if the node has a sibling
	if (node->sibling != NULL)
	{
		traverseTree(node->sibling, scopeName, true);
	}
}

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------ node type handling ----------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

// handles function declaration nodes
void handleFuncDecl(TreeNode* node)
{
	TreeNode* dupe = symTable->lookupLocal(node->value.str);
	// try inserting the node into the symbol table
	// if it fails, check for duplicate node error
	if (!symTable->insert(node->value.str, node))
	{
		// print error message if duplicate symbol node couldn't be found
		if (dupe == NULL)
		{
			printSemNoDupeError();
		}
		// print error message about duplicate declaration
		printAlreadyDeclaredError(node, dupe);
	}

	// set currentFunc to this function
	currentFunc = node;
	// enter a new function scope
	symTable->enterFunc(node->value.str);
	std::string funcName(node->value.str);
	// traverse this node's children
	callChildren(node, funcName + "-cmpd-stmt");

	// check to see if function has a return statement somewhere
	// ignores this check if the function returns type void or this function's symbol is a duplicate
	if (!node->returned && node->expType != Void)
	{
		printWarning(node);
		printf("Expecting to return type %s but function '%s' has no return statement.\n", getTypeString(node->expType), node->value.str);
	}

	// leave this scope
	node->size = symTable->getCurrentFrameSize();
	symTable->leave();
	// traverse this node's sibling(s)
	callSibs(node);
}

// handles compound statement nodes
void handleCompound(TreeNode* node, std::string scopeName)
{
	// if no scope name is given, enter a new scope called "compound-stmt"
	if (scopeName == "")
	{
		symTable->enter("compound-stmt");
	}
	// otherwise, use the given scopeName
	else
	{
		symTable->enter(scopeName);
	}
	// traverse this node's children
	callChildren(node, "");
	// leave this compound statement's scope
	node->size = symTable->getCurrentFrameSize();
	symTable->leave();
	// traverse this node's sibling(s)
	callSibs(node);
}

// handles if and while statement nodes
void handleIfWhile(TreeNode* node)
{
	// strings to display the correct type of statement
	std::string scopeName;
	std::string compoundName;
	char* stmtType;
	if (node->nodeType == If)
	{
		scopeName = "if-stmt";
		compoundName = "if-cmpd-stmd";
		stmtType = (char*) "if";
	}
	else if (node->nodeType == While)
	{
		scopeName = "while-stmt";
		compoundName = "while-cmpd-stmd";
		stmtType = (char*) "while";
	}
	else
	{
		scopeName = "unknown-stmt";
		compoundName = "unknown-cmpd-stmd";
		stmtType = (char*) "unknown";
	}

	// enter new scope for this statement
	symTable->enter(scopeName);

	// traverse this node's children manually so else part can be traversed later after leaving scope if this is an if-else statement
	for (int i = 0; i < 2; i++)
	{
		if (node->children[i] != NULL)
		{
			traverseTree(node->children[i], compoundName, true);
		}
	}

	// if the test condition is not of type bool and has a defined type
	if (node->children[0]->expType != Bool && node->children[0]->expType != Undefined)
	{
		printTestTypeError(node, stmtType);
	}

	// if the test condition is an array
	if (node->children[0]->isArray)
	{
		printTestArrError(node, stmtType);
	}

	// leave this scope
	node->size = symTable->getCurrentFrameSize();
	symTable->leave();

	// if this is an if statement that has an else part (3rd child)
	if (node->children[2] != NULL)
	{
		traverseTree(node->children[2], "else-cmpd-stmt", true);
	}

	// traverse this node's sibling(s)
	callSibs(node);
}

// handles for statement nodes
void handleFor(TreeNode* node)
{
	// enter new scope for this for loop
	symTable->enter("for-stmt");
	// traverse this node's children
	callChildren(node, "for-cmpd-stmt");
	// leave this scope
	node->size = symTable->getCurrentFrameSize();
	symTable->leave();
	// traverse this node's sibling(s)
	callSibs(node);
}

// handles iteration range nodes
void handleRange(TreeNode* node)
{
	// traverse children to determine their types
	callChildren(node, true);

	// check each child for errors
	for (int i = 0; i < maxChildren; i++)
	{
		// if the child exists and it has a defined type
		if (node->children[i] != NULL && node->children[i]->expType != Undefined)
		{
			// if the child is not type int
			if (node->children[i]->expType != Int)
			{
				printIterTypeError(node, node->children[i], i+1);
			}

			// if the child is an array
			if (node->children[i]->isArray)
			{
				printIterArrError(node, i+1);
			}
		}
	}

	// traverse this node's sibling(s)
	callSibs(node);
}

// handles return statement nodes
void handleReturn(TreeNode* node)
{
	// traverse this node's children first to determine their types
	callChildren(node);

	// if this statement is returning a value and it's an array
	if (node->children[0] != NULL && node->children[0]->isArray)
	{
		// print error message
		printError(node);
		printf("Cannot return an array.\n");
	}

	// make sure the function this return is in has its returned flag set to true
	currentFunc->returned = true;
	// if the function is void and the return statement is returning something
	if (node->children[0] != NULL && currentFunc->expType == Void)
	{
		printError(node);
		printf("Function '%s' at line %d is expecting no return value, but return has a value.\n", currentFunc->value.str, currentFunc->line);
	}
	// if the function is supposed to return something but this return statement is not returning anything
	else if (node->children[0] == NULL && currentFunc->expType != Void)
	{
		printError(node);
		printf("Function '%s' at line %d is expecting to return type %s but return has no value.\n", currentFunc->value.str, currentFunc->line, getTypeString(currentFunc->expType));
	}
	// if this return statement is returning something of the wrong type and that thing has a defined type
	else if (node->children[0] != NULL && node->children[0]->expType != currentFunc->expType && node->children[0]->expType != Undefined)
	{
		printError(node);
		printf("Function '%s' at line %d is expecting to return type %s but returns type %s.\n", currentFunc->value.str, currentFunc->line, getTypeString(currentFunc->expType), getTypeString(node->children[0]->expType));
	}

	// traverse this node's sibling(s)
	callSibs(node);
}

// handles break statement nodes
void handleBreak(TreeNode* node)
{
	// if this break statement is not inside of a while or for loop
	if (!symTable->inLoop())
	{
		printError(node);
		printf("Cannot have a break statement outside of loop.\n");
	}

	// traverse this node's children and sibling(s)
	callChildren(node);
	callSibs(node);
}

// handles variable / parameter declaration nodes
void handleVarDecl(TreeNode* node, std::string scopeName)
{
	// traverse this node's children
	callChildren(node, "");

	// try inserting the node into the symbol table and make sure there isn't already a parameter with the same name
	if (symTable->lookupParm(node->value.str, node->isIterVar) == NULL && symTable->insert(node->value.str, node))
	{
		// if this declaration is for a parameter, set it's initialized flag to true and set the mem space to parameter
		if (node->nodeType == Parm)
		{
			node->inited = true;
			node->memSpace = Parameter;
		}
		else if (node->isStatic)
		{
			node->memSpace = Static;
		}
		else if (symTable->depth() == 1)
		{
			node->memSpace = Global;
		}
		else
		{
			node->memSpace = Local;
		}
	}
	// if this symbol is already in the table
	else
	{
		TreeNode* dupe = symTable->lookupLocal(node->value.str);
		// if the dupe is null, try checking parameter space for the duplicate
		if (dupe == NULL)
		{
			dupe = symTable->lookupParm(node->value.str, node->isIterVar);
		}
		// print error message if duplicate symbol node couldn't be found
		if (dupe == NULL)
		{
			printSemNoDupeError();
		}
		// print error message about duplicate declaration
		printAlreadyDeclaredError(node, dupe);
	}

	// if the node has an initializer, do initializer checks
	if (node->children[0] != NULL)
	{
		// if the initializer is not a constant expression
		if (!isConstExp(node->children[0]))
		{
			printNotConstExpError(node);
		}

		// if the initializer is not the same type as the variable and is not undefined
		if (node->expType != node->children[0]->expType && node->children[0]->expType != Undefined)
		{
			printInitTypeError(node);
		}

		// if the variable and the initializer are not both arrays
		if (node->isArray != node->children[0]->isArray)
		{
			printInitArrError(node);
		}
	}

	// traverse this node's sibling(s)
	callSibs(node, scopeName);
}

// handles function call nodes
void handleCall(TreeNode* node)
{
	// lookup the symbol table entry for the id that gets called
	TreeNode* dupe = symTable->lookup(node->value.str);
	if (dupe == NULL)
	{
		printNotDeclaredError(node);
		// traverse this node's children
		callChildren(node, true);
	}
	else
	{
		// copy all of the values from the original function to this call of it
		node->expType = dupe->expType;
		dupe->used = true;
		node->size = dupe->size;
		node->memSpace = dupe->memSpace;
		node->foffset = dupe->foffset;

		// if symbol is not a function, throw error
		if (dupe->nodeType != Func)
		{
			printError(node);
			printf("'%s' is a simple variable and cannot be called.\n", node->value.str);
			// traverse this node's children
			callChildren(node, true);
		}
		else
		{
			// traverse this node's children first to determine their types
			callChildren(node, true);
			checkParms(node, dupe);
		}
	}
	// traverse this node's children and sibling(s)
	callSibs(node);
}

// handles variable identifier nodes
void handleId(TreeNode* node, bool checkInit)
{
	// lookup the symbol table entry for the id that gets called
	TreeNode* dupe = symTable->lookup(node->value.str);
	if (dupe == NULL)
	{
		printNotDeclaredError(node);
	}
	else
	{
		// copy all of the values from the original variable to this call of it
		node->expType = dupe->expType;
		node->isArray = dupe->isArray;
		node->isStatic = dupe->isStatic;
		dupe->used = true;
		node->inited = dupe->inited;
		node->size = dupe->size;
		node->memSpace = dupe->memSpace;
		node->foffset = dupe->foffset;

		// check to make sure the symbol is not a function
		if (dupe->nodeType == Func)
		{
			printError(node);
			printf("Cannot use function '%s' as a variable.\n", node->value.str);
		}
		// if the checkInit flag is true, this symbol hasn't been initialized yet, and there hasn't been a warning about it for this symbol yet
		else if (checkInit && !node->inited && !node->isStatic && !dupe->initWarned)
		{
			dupe->initWarned = true;
			printUninitedWarning(node);
		}
	}

	// traverse this node's children and sibling(s)
	callChildren(node, true);
	callSibs(node);
}

// handles constant value nodes
void handleConst(TreeNode* node)
{
	// if the constant is a string
	if (node->expType == Char && node->isArray)
	{
		// set it's mem values and allocate space for it in the global scope
		symTable->allocString(node);
	}

	// traverse this nodes children and sibling(s)
	callChildren(node);
	callSibs(node);
}

// handles semantic checks for Inc (++) and Dec (--) operators
void handleIncOp(TreeNode* node)
{
	TreeNode* dupe = NULL;
	// traverse this node's children to make sure its children get their types determined before they get checked
	callChildren(node, true);
	// lookup symbol to make sure it's been declared
	// if the first child is a bracket operator (dereferencing an array)
	if (node->children[0]->nodeType == Op && node->children[0]->opKind == Brak)
	{
		// lookup that child's first child's symbol
		dupe = symTable->lookup(node->children[0]->children[0]->value.str);
	}
	else
	{
		// just lookup the first child's symbol
		dupe = symTable->lookup(node->children[0]->value.str);
	}
	// if the symbol exists
	if (dupe != NULL)
	{
		// check to make sure the child is the right type
		if (node->children[0]->expType != Int && node->children[0]->expType != Undefined)
		{
			printUnaryTypeError(node);
		}

		// check to make sure the child isn't an array
		if (node->children[0]->isArray)
		{
			printNoArraysError(node);
		}
	}
	// traverse this node's sibling(s)
	callSibs(node);
}

// handles semantic checks for assignment operators like Assi (=) or Addas (+=)
void handleAssi(TreeNode* node, bool math)
{
	// traverse this node's children to make sure its children get their types determined before they get checked 
	callChildren(node, 0);

	TreeNode* dupe = NULL;
	// lookup first child symbol to make sure it's been declared
	// if the first child is a bracket operator (dereferencing an array)
	if (node->children[0]->nodeType == Op && node->children[0]->opKind == Brak)
	{
		// lookup that child's first child's symbol
		dupe = symTable->lookup(node->children[0]->children[0]->value.str);
	}
	else if (node->children[0]->nodeType == Id)
	{
		// just lookup the first child's symbol
		dupe = symTable->lookup(node->children[0]->value.str);
	}
	// if the symbol exists
	if (dupe != NULL)
	{
		// make sure the symbol on the left is checked as initialized
		dupe->inited = true;

		// if this is a math assignment operator like Addas (+=) or Divas (/=)
		if (math)
		{
			// check to make sure each child is the right type
			if (node->children[0]->expType != Int && node->children[0]->expType != Undefined)
			{
				printWrongTypeError(node, true);
			}
			if (node->children[1]->expType != Int && node->children[1]->expType != Undefined)
			{
				printWrongTypeError(node, false);
			}

			// check to make sure children aren't arrays
			if (node->children[0]->isArray || node->children[1]->isArray)
			{
				printNoArraysError(node);
			}
		}
		// if this is a plain assignment operator (=)
		else
		{
			// set the expType and the isArray flag to the left child's values
			node->expType = node->children[0]->expType;
			node->isArray = node->children[0]->isArray;

			// check to make sure children are the same type
			if (node->children[0]->expType != node->children[1]->expType && node->children[0]->expType != Undefined && node->children[1]->expType != Undefined)
			{
				printNotSameTypeError(node);
			}

			// checks to make sure both children are either arrays or not arrays
			if ((node->children[0]->isArray && !node->children[1]->isArray) || (!node->children[0]->isArray && node->children[1]->isArray))
			{
				printBothArraysError(node);
			}
		}
	}

	// reset dupe so it doesn't keep a previous address for the next check
	dupe = NULL;
	// lookup second child to make sure it's been declared
	// if the second child is a bracket operator (dereferencing an array)
	if (node->children[1]->nodeType == Op && node->children[1]->opKind == Brak)
	{
		// lookup that child's first child's symbol
		dupe = symTable->lookup(node->children[1]->children[0]->value.str);
	}
	else if (node->children[1]->nodeType == Id)
	{
		// just lookup the first child's symbol
		dupe = symTable->lookup(node->children[1]->value.str);
	}

	// traverse this node's sibling(s)
	callSibs(node);
}

// handles semantic checks for operators like addition (+) or greater than (>)
void handleBinaryOp(TreeNode* node, bool comparison)
{
	// traverse this node's children to make sure its children get their types determined before they get checked
	callChildren(node, true);

	// if the node is a comparison operator like equal (==) or greater than (>)
	if (comparison)
	{
		// check to make sure both children are the same type
		if (node->children[0]->expType != node->children[1]->expType && node->children[0]->expType != Undefined && node->children[1]->expType != Undefined)
		{
			printNotSameTypeError(node);
		}

		// check to make sure children are either both arrays or both not arrays
		if ((node->children[0]->isArray && !node->children[1]->isArray) || (!node->children[0]->isArray && node->children[1]->isArray))
		{
			printBothArraysError(node);
		}
	}
	// if the node is a math or logical operator like Add (+) or And (and)
	else
	{
		// check to make sure each child is the right type
		if (node->children[0]->expType != node->expType && node->children[0]->expType != Undefined)
		{
			printWrongTypeError(node, true);
		}
		if (node->children[1]->expType != node->expType && node->children[1]->expType != Undefined)
		{
			printWrongTypeError(node, false);
		}

		// check to make sure children aren't arrays
		if (node->children[0]->isArray || node->children[1]->isArray)
		{
			printNoArraysError(node);
		}
	}

	// traverse this node's sibling(s)
	callSibs(node);
}

// handles unary non-array operators (not, change-sign / negative (-), and random(?))
void handleUnary(TreeNode* node)
{
	// traverse this node's child to make sure it gets its type determined before it gets checked
	callChildren(node, true);

	TreeNode* dupe = NULL;

	// lookup symbol to make sure it's been declared
	// if the first child is a bracket operator (dereferencing an array)
	if (node->children[0]->nodeType == Op && node->children[0]->opKind == Brak)
	{
		// lookup that child's first child's symbol
		dupe = symTable->lookup(node->children[0]->children[0]->value.str);
	}
	else if (node->children[0]->nodeType == Id || node->children[0]->nodeType == Call)
	{
		// just lookup the first child's symbol
		dupe = symTable->lookup(node->children[0]->value.str);
	}
	// if the symbol exists or if it's a const or operator
	if (dupe != NULL || node->children[0]->nodeType == Const || node->children[0]->nodeType == Op || node->children[0]->nodeType == Assign)
	{
		// if the node's child isn't the right type
		if (node->children[0]->expType != node->expType && node->children[0]->expType != Undefined)
		{
			// print an error message
			printUnaryTypeError(node);
		}

		// check to make sure the child isn't an array
		if (node->children[0]->isArray)
		{
			printNoArraysError(node);
		}
	}

	// traverse this node's sibling(s)
	callSibs(node);
}

// handles the sizeof operator (*)
void handleSizeof(TreeNode* node)
{
	// traverse this node's child to make sure it gets its type determined before it gets checked
	//callChildrenNoInit(node, true);
	callChildren(node, true);

	// lookup symbol to make sure it's been declared
	TreeNode* dupe = symTable->lookup(node->children[0]->value.str);
	// if the symbol exists or it's not a symbol
	if (node->nodeType != Id || (dupe != NULL && node->nodeType == Id))
	{
		// check to make sure this operator is being used on an array
		if (!node->children[0]->isArray)
		{
			printError(node);
			printf("The operation '%s' only works with arrays.\n", node->value.str);
		}
	}

	// traverse this node's sibling(s)
	callSibs(node);
}

// handles bracket operators (array[index])
void handleBracket(TreeNode* node, bool checkInit)
{
	// ignore initialization check on just index child
	if (checkInit)
	{
		callChildren(node, true);
	}
	// ignore initialization check on both children
	else
	{
		callChildren(node, 0);
	}

	// check to make sure left child is an array
	if (!node->children[0]->isArray)
	{
		printError(node);
		printf("Cannot index nonarray '%s'.\n", node->children[0]->value.str);
	}

	// lookup symbol to make sure it's been declared
	TreeNode* dupe1 = symTable->lookup(node->children[0]->value.str);
	TreeNode* dupe2 = NULL;
	if (node->children[1]->nodeType == Id || node->children[1]->nodeType == Call)
	{
		dupe2 = symTable->lookup(node->children[1]->value.str);
	}
	// if the symbol exists
	if (dupe1 != NULL)
	{
		node->expType = node->children[0]->expType;

		// if the index node is a valid identifier or is an expression or constant
		if ((dupe2 != NULL && !(node->children[1]->nodeType == Id && dupe2->nodeType != Var) && !(node->children[1]->nodeType == Call && dupe2->nodeType != Func)) || (node->children[1]->nodeType != Id && node->children[1]->nodeType != Call))
		{
			// check to make sure array is being indexed by an integer
			if (node->children[1]->expType != Int && node->children[1]->expType != Undefined)
			{
				printError(node);
				printf("Array '%s' should be indexed by type int but got type %s.\n", node->children[0]->value.str, getTypeString(node->children[1]->expType));
			}

			// check to make sure index is not an array
			if (node->children[1]->isArray)
			{
				printError(node);
				printf("Array index is the unindexed array '%s'.\n", node->children[1]->value.str);
			}
		}
	}

	callSibs(node);
}

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------ node helper functions -------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

// returns whether or not a node is part of a constant expression or not
bool isConstExp(TreeNode* node)
{
	// if the current node is not a constant or a valid initializer operator
	if (node->nodeType != Const && !isLogOp(node) && !isCompOp(node) && !isMathOp(node))
	{
		return false;
	}

	// if any of the children are not constants of valid initializer operators
	for (int i = 0; i < maxChildren; i++)
	{
		if (node->children[i] != NULL && !isConstExp(node->children[i]))
		{
			return false;
		}
	}

	// otherwise, return true
	return true;
}

// returns whether or not a node is a logical operator
bool isLogOp(TreeNode* node)
{
	return node->opKind == Or || node->opKind == And || node->opKind == Not;
}

// return whether or not a node is a comparison operator
bool isCompOp(TreeNode* node)
{
	return node->opKind == Less || node->opKind == Leq || node->opKind == Gtr || node->opKind == Geq || node->opKind == Eq || node->opKind == Neq;
}

// returns whether or not a node is a math operator
bool isMathOp(TreeNode* node)
{
	return node->opKind == Neg || node->opKind == Add || node->opKind == Sub || node->opKind == Mul || node->opKind == Div || node->opKind == Mod;
}

// checks to make sure a function call node has the correct parameters compared to its declaration (dupe symbol)
void checkParms(TreeNode* node, TreeNode* dupe)
{
	// loop through all of the arguments in the call and the function definition until one or both of them end
	TreeNode* nScout = node->children[0]; // node scout for traversing node's parameters / children
	TreeNode* dScout = dupe->children[0]; // dupe scout for traversing dupe's parameters / children
	for (int i = 1; nScout != NULL && dScout != NULL; i++)
	{
		// if the called parm type isn't the same as the expected parm type
		if (nScout->expType != dScout->expType && nScout->expType != Undefined)
		{
			printError(node);
			printf("Expecting type %s in parameter %d of call to '%s' declared on line %d but got type %s.\n", getTypeString(dScout->expType), i, node->value.str, dupe->line, getTypeString(nScout->expType));
		}

		// if an array is passed when a non array was expected
		if (nScout->isArray && !dScout->isArray)
		{
			printError(node);
			printf("Not expecting array in parameter %d of call to '%s' declared on line %d.\n", i, node->value.str, dupe->line);
		}
		// if a non array is passed when an array was expected
		else if (!nScout->isArray && dScout->isArray)
		{
			printError(node);
			printf("Expecting array in parameter %d of call to '%s' declared on line %d.\n", i, node->value.str, dupe->line);
		}

		// traverse to next parameter
		nScout = nScout->sibling;
		dScout = dScout->sibling;
	}

	// if there are less arguments called than there are supposed to be
	if (nScout == NULL && dScout != NULL)
	{
		printError(node);
		printf("Too few parameters passed for function '%s' declared on line %d.\n", node->value.str, dupe->line);
	}
	// if there are more arguments called than there are supposed to be
	else if (nScout != NULL && dScout == NULL)
	{
		printError(node);
		printf("Too many parameters passed for function '%s' declared on line %d.\n", node->value.str, dupe->line);
	}
}

// given a lhs node, turns either lhs or rhs into the string " not" depending on whether or not the node is an array
void getArraySideStrings(TreeNode* node, char*& lhs, char*& rhs)
{
	if (node->isArray)
	{
		rhs = (char*) " not";
	}
	else
	{
		lhs = (char*) " not";
	}
}

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------ error / warning messages ----------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

// prints the error alert and line number of the error at the start of an error message and increments the error count
void printError(TreeNode* node)
{
	errors++;
	printf("ERROR(%d): ", node->line);
}

// prints the warning alert and line number of the warning at the start of an warning message and increments the warning count
void printWarning(TreeNode* node)
{
	warnings++;
	printf("WARNING(%d): ", node->line);
}

// prints a debug error about not being able to find a duplicate tree node after an insert failed
void printSemNoDupeError()
{
	printf("ERROR(SemanticAnalyzer): duplicate node couldn't be found\n");
}

// prints the symbol already declared error message
void printAlreadyDeclaredError(TreeNode* node, TreeNode* dupe)
{
	printError(node);
	printf("Symbol '%s' is already declared at line %d.\n", node->value.str, dupe->line);
}

// prints the symbol not declared error message
void printNotDeclaredError(TreeNode* node)
{
	printError(node);
	printf("Symbol '%s' is not declared.\n", node->value.str);
}

// prints the incorrect type for a binary operator on one side error
void printWrongTypeError(TreeNode* node, bool lhs)
{
	char* side;
	int index;
	char* rightType;
	char* wrongType;

	if (lhs)
	{
		side = (char*) "lhs";
		index = 0;
	}
	else
	{
		side = (char*) "rhs";
		index = 1;
	}

	rightType = getTypeString(node->expType);
	wrongType = getTypeString(node->children[index]->expType);

	printError(node);
	printf("'%s' requires operands of type %s but %s is of type %s.\n", node->value.str, rightType, side, wrongType);
}

// prints the incorrect type with a unary operator error
void printUnaryTypeError(TreeNode* node)
{
	char* rightType;
	char* wrongType;

	rightType = getTypeString(node->expType);
	wrongType = getTypeString(node->children[0]->expType);

	printError(node);
	printf("Unary '%s' requires an operand of type %s but was given type %s.\n", node->value.str, rightType, wrongType);
}

// prints the not matching types for an operator error
void printNotSameTypeError(TreeNode* node)
{
	char* leftType;
	char* rightType;

	leftType = getTypeString(node->children[0]->expType);
	rightType = getTypeString(node->children[1]->expType);

	printError(node);
	printf("'%s' requires operands of the same type but lhs is type %s and rhs is type %s.\n", node->value.str, leftType, rightType);
}

// prints the operation doesn't work on arrays error
void printNoArraysError(TreeNode* node)
{
	printError(node);
	printf("The operation '%s' does not work with arrays.\n", node->value.str);
}

// prints the not both sides are arrays error
void printBothArraysError(TreeNode* node)
{
	char* lhs = (char*) "";
	char* rhs = (char*) "";

	// turns either lhs or rhs into the string " not" depending on whether or not the lhs child is an array
	getArraySideStrings(node->children[0], lhs, rhs);

	printError(node);
	printf("'%s' requires both operands be arrays or not but lhs is%s an array and rhs is%s an array.\n", node->value.str, lhs, rhs);
}

// prints the expecting boolean test condition error
void printTestTypeError(TreeNode* node, char* stmtType)
{
	printError(node);
	printf("Expecting Boolean test condition in %s statement but got type %s.\n", stmtType, getTypeString(node->children[0]->expType));
}

// prints the can't use array as test condition error
void printTestArrError(TreeNode* node, char* stmtType)
{
	printError(node);
	printf("Cannot use array as test condition in %s statement.\n", stmtType);
}

// prints the not type int in iter statement error
void printIterTypeError(TreeNode* node, TreeNode* child, int i)
{
	printError(node);
	printf("Expecting type int in position %d in range of for statement but got type %s.\n", i, getTypeString(child->expType));
}

// prints the can't use array in iter statement error
void printIterArrError(TreeNode* node, int i)
{
	printError(node);
	printf("Cannot use array in position %d in range of for statement.\n", i);
}

// prints the initializer expression for a variable is not a constant expression error
void printNotConstExpError(TreeNode* node)
{
	printError(node);
	printf("Initializer for variable '%s' is not a constant expression.\n", node->value.str);
}

// prints the initializer expression and variable types do not match error
void printInitTypeError(TreeNode* node)
{
	printError(node);
	// WHY DOES THE EXPECTED OUTPUT NOT HAVE A PERIOD AT THE END OF THIS ERROR MESSAGE LIKE EVERY OTHER ERROR AND WARNING MESSAGE L;DKSUARO34H2PFV8UEW9PQIOFN123KOUY89VAVHNAP
	printf("Initializer for variable '%s' of type %s is of type %s\n", node->value.str, getTypeString(node->expType), getTypeString(node->children[0]->expType));
}

// prints the initializer and variable must both be or not be arrays error
void printInitArrError(TreeNode* node)
{
	char* lhs = (char*) "";
	char* rhs = (char*) "";

	// turns either lhs or rhs into the string " not" depending on whether or not the lhs child is an array
	getArraySideStrings(node, lhs, rhs);

	printError(node);
	printf("Initializer for variable '%s' requires both operands be arrays or not but variable is%s an array and rhs is%s an array.\n", node->value.str, lhs, rhs);
}

// prints the uninitialized warning
void printUninitedWarning(TreeNode* node)
{
	printWarning(node);
	printf("Variable '%s' may be uninitialized when used here.\n", node->value.str);
}