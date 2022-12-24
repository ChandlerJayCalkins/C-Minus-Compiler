#include "symbolTable.h"
#include <string>

// // // // // // // // // // // // // // // // // // // // 
//
// Introduction
//
// This symbol table library supplies basic insert and lookup for
// symbols linked to TreeNode pointers of data. The is expected to use
// ONLY the SymbolTable class and NOT the Scope class. The Scope class
// is used by SymbolTable in its implementation.
//
// Plenty of room for improvement inlcuding: better debugging setup,
// passing of refs rather than values and purpose built char *
// routines, and C support.
//
// WARNING: lookup will return NULL pointer if key is not in table.
// This means the TreeNode* cannot have zero as a legal value! Attempting
// to save a NULL pointer will get a error.
//
// A main() is commented out and has testing code in it.
//
// Original Author: Robert Heckendorn   Apr 3, 2021
//
// Modifying Author: Chandler Calkins	Fall 2022
//

   
// // // // // // // // // // // // // // // // // // // // 
//
// Some sample void * printing routines.   User shoud supply their own.
// The print routine will print the name of the symbol and then
// use a user supplied function to print the pointer.
//

// print nothing about the pointer
void pointerPrintNothing(void* data)
{
}
   
// print the pointer as a hex address
void pointerPrintAddr(void* data)
{
    printf("0x%016llx ", (unsigned long long int)(data));
}
   
// print the pointer as a long long int
void pointerPrintLongInteger(void* data)
{
    printf("%18lld ", (unsigned long long int)(data));
}
   
// print the pointer as a char * string
void pointerPrintStr(void* data)
{
    printf("%s ", (char *)(data));
}

// // // // // // // // // // // // // // // // // // // //
//
// Class: Scope
//
// Helper class for SymbolTable
//

class SymbolTable::Scope
{
	private:
		static bool debugFlg;                      // turn on tedious debugging
		std::string name;                          // name of scope
		std::map<std::string , TreeNode*> symbols;    // use an ordered map (not as fast as unordered)
		int currentOffset;							// keeps track of the used data space in this scope so far

	public:
		Scope(std::string newname, int startOffset);
		~Scope();
		std::string scopeName();                   // returns name of scope
		void debug(bool state);                    // sets the debug flag to state
		void print(void (*printData)(TreeNode*));     // prints the table using the supplied function to print the ast node
		void applyToAll(void (*action)(std::string , TreeNode*));  // applies func to all symbol/data pairs
		bool insert(std::string sym, TreeNode* node);   // inserts a new ptr associated with symbol sym
												// returns false if already defined
		TreeNode* lookup(std::string sym);             // returns the ptr associated with sym
												// returns NULL if symbol not found
		// checks for unused symbols and prints out warning messages about them
		void checkForUnused();
		// returns if a node is a function and its symbol is either main or one of the built in functions
		bool isRequiredFunc(TreeNode* node);
		void incOffset(TreeNode* node);
		int getCurrentOffset();					// returns the current frame offset
};


SymbolTable::Scope::Scope(std::string newname, int startOffset)
{
	name = newname;
	debugFlg = false;
	// if this is a function scope, start the offset at -2, otherwise start it at 0
	currentOffset = startOffset;
	if (name == "for-stmt")
	{
		currentOffset -= 2;
	}
}


SymbolTable::Scope::~Scope()
{
}

// returns char *name of scope
std::string SymbolTable::Scope::scopeName()
{
	return name;
}

// set scope debugging
void SymbolTable::Scope::debug(bool state)
{
	debugFlg = state;
}


// print the scope
void SymbolTable::Scope::print(void (*printData)(TreeNode*))
{
	printf("Scope: %-15s -----------------\n", name.c_str());
	for (std::map<std::string , TreeNode*>::iterator it=symbols.begin(); it!=symbols.end(); it++)
	{
		printf("%20s: ", (it->first).c_str());
		printData(it->second);
		printf("\n");
	}
}


// apply the function to each symbol in this scope
void SymbolTable::Scope::applyToAll(void (*action)(std::string , TreeNode*))
{
	for (std::map<std::string , TreeNode*>::iterator it=symbols.begin(); it!=symbols.end(); it++)
	{
		action(it->first, it->second);
	}
}


// returns true if insert was successful and false if symbol already in this scope
bool SymbolTable::Scope::insert(std::string sym, TreeNode* node)
{
	if (symbols.find(sym) == symbols.end())
	{
		if (debugFlg) printf("DEBUG(Scope): insert in \"%s\" the symbol \"%s\".\n", name.c_str(), sym.c_str());
		if (node==NULL)
		{
			printf("ERROR(SymbolTable): Attempting to save a NULL pointer for the symbol '%s'.\n", sym.c_str());
		}
		symbols[sym] = node;
		// if the symbol is not static and it's not a function
		if (!node->isStatic && node->nodeType != Func)
		{
			// set the foffset of the node variable
			incOffset(node);
		}
		return true;
	}
	else
	{
		if (debugFlg) printf("DEBUG(Scope): insert in \"%s\" the symbol \"%s\" but symbol already there!\n", name.c_str(), sym.c_str());
		return false;
	}
}

TreeNode* SymbolTable::Scope::lookup(std::string sym)
{
	if (symbols.find(sym) != symbols.end())
	{
		if (debugFlg) printf("DEBUG(Scope): lookup in \"%s\" for the symbol \"%s\" and found it.\n", name.c_str(), sym.c_str());
		return symbols[sym];
	}
	else
	{
		if (debugFlg) printf("DEBUG(Scope): lookup in \"%s\" for the symbol \"%s\" and did NOT find it.\n", name.c_str(), sym.c_str());
		return NULL;
	}
}

// checks for unused symbols and prints out warning messages about them
void SymbolTable::Scope::checkForUnused()
{
	// check each symbol in this scope
	for (std::map<std::string , TreeNode*>::iterator it=symbols.begin(); it!=symbols.end(); it++)
	{
		// if the current symbol has not been used and it's not the main() function
		if (!it->second->used && !isRequiredFunc(it->second))
		{
			// print a warning message

			// get a string of the type of symbol this is for the warning message
			char* type;
			switch (it->second->nodeType)
			{
				case Var:
					type = (char*) "variable";
					break;
				case Func:
					type = (char*) "function";
					break;
				case Parm:
					type = (char*) "parameter";
					break;
				default:
					type = (char*) "unknown symbol";
			}

			warnings++;
			printf("WARNING(%d): ", it->second->line);
			printf("The %s '%s' seems not to be used.\n", type, it->second->value.str);
		}
	}
}

// returns if a node is a function and its symbol is either main or one of the built in functions
bool SymbolTable::Scope::isRequiredFunc(TreeNode* node)
{
	return node->nodeType == Func && (strcmp(node->value.str, (char*) "main") == 0 || strcmp(node->value.str, (char*) "output") == 0 || strcmp(node->value.str, (char*) "outputb") == 0
	|| strcmp(node->value.str, (char*) "outputc") == 0 || strcmp(node->value.str, (char*) "input") == 0 || strcmp(node->value.str, (char*) "inputb") == 0
	|| strcmp(node->value.str, (char*) "inputc") == 0 || strcmp(node->value.str, (char*) "outnl") == 0);
}

void SymbolTable::Scope::incOffset(TreeNode* node)
{
	// if the node is a non parameter array, set it's foffset to the current offset + 1 since an extra space needs to be used to store its size
	// otherwise, just set its foffset to the currentOffset
	node->foffset = node->isArray && node->memSpace != Parameter ? currentOffset - 1 : currentOffset;
	currentOffset -= node->size;
}

// returns the current frame offset
int SymbolTable::Scope::getCurrentOffset()
{
	return currentOffset;
}

bool SymbolTable::Scope::debugFlg;




// // // // // // // // // // // // // // // // // // // // 
//
// Class: SymbolTable
//
//  This is a stack of scopes that represents a symbol table
//

SymbolTable::SymbolTable()
{
	currentOffset = 0;
	debugFlg = false;
	enter((std::string )"Global");
}


void SymbolTable::debug(bool state)
{
	debugFlg = state;
}


// Returns the number of scopes in the symbol table
int SymbolTable::depth()
{
	return stack.size();
}


// print all scopes using data printing func
void SymbolTable::print(void (*printData)(TreeNode*))
{
	printf("===========  Symbol Table  ===========\n");
	for (std::vector<Scope *>::iterator it=stack.begin(); it!=stack.end(); it++)
	{
		(*it)->print(printData);
	}
	printf("===========  ============  ===========\n");
}


// Enter a scope
void SymbolTable::enter(std::string name)
{
	if (debugFlg) printf("DEBUG(SymbolTable): enter scope \"%s\".\n", name.c_str());
	int startSize = stack.size() < 1 ? 0 : stack.back()->getCurrentOffset();
	if (debugFlg) printf("DEBUG(SymbolTable): start size = %d\n", startSize);
	stack.push_back(new Scope(name, startSize));
}


// Enter a function scope
void SymbolTable::enterFunc(std::string name)
{
	if (debugFlg) printf("DEBUG(SymbolTable): enter function scope \"%s\".\n", name.c_str());
	stack.push_back(new Scope(name, -2));
}


// Leave a scope (not allowed to leave global)
void SymbolTable::leave()
{
	if (debugFlg) printf("DEBUG(SymbolTable): leave scope \"%s\".\n", (stack.back()->scopeName()).c_str());
	if (stack.size()>1)
	{
		stack.back()->checkForUnused();

		delete stack.back();
		stack.pop_back();
	}
	else
	{
		printf("ERROR(SymbolTable): You cannot leave global scope.  Number of scopes: %d.\n", (int)stack.size());
	}
}


// Lookup a symbol anywhere in the stack of scopes
// Returns NULL if symbol not found, otherwise it returns the stored TreeNode* associated with the symbol
TreeNode* SymbolTable::lookup(std::string sym)
{
	TreeNode* data;
	std::string name;

	data = NULL;  // set even though the scope stack should never be empty
	for (std::vector<Scope *>::reverse_iterator it=stack.rbegin(); it!=stack.rend(); it++)
	{
		data = (*it)->lookup(sym);
		name = (*it)->scopeName();
		if (data!=NULL) break;
    }

	if (debugFlg)
	{
		printf("DEBUG(SymbolTable): lookup the symbol \"%s\" and ", sym.c_str());
		if (data) printf("found it in the scope named \"%s\".\n", name.c_str());
		else printf("did NOT find it!\n");
	}

	return data;
}


// Lookup a symbol in the current local scope
// Returns NULL if symbol not found, otherwise it returns the stored TreeNode* associated with the symbol
TreeNode* SymbolTable::lookupLocal(std::string sym)
{
	TreeNode* data;

	data = stack.back()->lookup(sym);
	if (debugFlg)
	{
		printf("DEBUG(SymbolTable): lookup the symbol \"%s\" in scope \"%s\" and %s.\n", sym.c_str(), stack.back()->scopeName().c_str(), (data ? "found it" : "did NOT find it"));
	}

	return data;
}


// returns ptr associated with sym in current function's parameter scope (return NULL if symbol not found)
TreeNode* SymbolTable::lookupParm(std::string sym, bool isIterVar)
{
	int index = stack.size() - 2;
	if (index < 1 || isIterVar || (index != 1 && stack[index]->scopeName() != "for-stmt"))
	{
		return NULL;
	}

	TreeNode* data;

	data = stack[index]->lookup(sym);
	if (debugFlg)
	{
		printf("DEBUG(SymbolTable): lookup the symbol \"%s\" in scope \"%s\" and %s.\n", sym.c_str(), stack.back()->scopeName().c_str(), (data ? "found it" : "did NOT find it"));
	}

	return data;
}


// Lookup a symbol in the global scope
// returns NULL if symbol not found, otherwise it returns the stored TreeNode* associated with the symbol
TreeNode* SymbolTable::lookupGlobal(std::string sym)
{
	TreeNode* data;

	data = stack[0]->lookup(sym);
	if (debugFlg)
	{
		printf("DEBUG(SymbolTable): lookup the symbol \"%s\" in the Globals and %s.\n", sym.c_str(), (data ? "found it" : "did NOT find it"));
	}

	return data;
}


// Insert a symbol into the most recent scope
// Returns true if insert was successful and false if symbol already in the most recent scope
bool SymbolTable::insert(std::string sym, TreeNode* node)
{
	if (debugFlg)
	{
		printf("DEBUG(symbolTable): insert in scope \"%s\" the symbol \"%s\"", (stack.back()->scopeName()).c_str(), sym.c_str());
		if(node==NULL) printf(" WARNING: The inserted pointer is NULL!!");
		printf("\n");
	}

	// if the node is a static variable, allocate its space in the global scope
	if (node->isStatic)
	{
		stack[0]->incOffset(node);
	}

	return (stack.back())->insert(sym, node);
}


// Insert a symbol into the global scope
// Returns true is insert was successful and false if symbol already in the global scope
bool SymbolTable::insertGlobal(std::string sym, TreeNode* node)
{
	if (debugFlg)
	{
		printf("DEBUG(Scope): insert the global symbol \"%s\"", sym.c_str());
		if(node==NULL) printf(" WARNING: The inserted pointer is NULL!!");
		printf("\n");
	}

	return stack[0]->insert(sym, node);
}


// Apply function to each simple in the local scope.   The function gets both the
// string and the associated pointer.
void SymbolTable::applyToAll(void (*action)(std::string , TreeNode*))
{
	stack[stack.size()-1]->applyToAll(action);
}


// Apply function to each simple in the global scope.   The function gets both the
// string and the associated pointer.
void SymbolTable::applyToAllGlobal(void (*action)(std::string , TreeNode*))
{
	stack[0]->applyToAll(action);
}


// Return whether or not the symbol table is currently in the scope of a while or for loop right now
bool SymbolTable::inLoop()
{
	for (std::vector<Scope *>::reverse_iterator it=stack.rbegin(); it!=stack.rend(); it++)
	{
		if ((*it)->scopeName() == "while-stmt" || (*it)->scopeName() == "while-cmpd-stmt" || (*it)->scopeName() == "for-stmt" || (*it)->scopeName() == "for-cmpd-stmt")
		{
			return true;
		}
    }

	return false;
}


// checks the global scope for unused symbols
void SymbolTable::checkGlobalUnused()
{
	stack[0]->checkForUnused();
}


// returns the current frame offset
int SymbolTable::getCurrentFrameSize()
{
	return stack.back()->getCurrentOffset();
}


// allocates a string constant in global space
void SymbolTable::allocString(TreeNode* node)
{
	stack[0]->incOffset(node);
}




// // // // // // // // // // // // // // // // // // // // 
//
// Some tests
//


std::string words[] = {"alfa", "bravo", "charlie", "dog", "echo", "foxtrot", "golf"};
int wordsLen = 7;



int counter;
void countSymbols(std::string sym, void *ptr)
{
	counter++;
	printf("%d %20s: ", counter, sym.c_str());
	pointerPrintAddr(ptr);
	printf("\n");
}

/*
bool SymbolTable::test()
{
	Scope s("global");

	s.debug(true);
	s.insert("dog", (char *)"woof");
	s.insert("cat", (char *)"meow");
	printf("%s\n", (char *)(s.lookup("cat")));
	printf("%s\n", (char *)(s.lookup("dog")));

	if (s.lookup("fox")==NULL) printf("not found\n");
	else printf("found\n");
	s.print(pointerPrintAddr);
	s.print(pointerPrintStr);

	SymbolTable st;
	st.debug(true);

	printf("Print symbol table.\n");
	st.print(pointerPrintStr);
	st.insert("alfa", (char *)"ant"); 
	st.insert("bravo", (char *)"bat"); 
	st.insert("charlie", (char *)"cob"); 

	st.enter("First");
	st.insert("charlie", (char *)"cow"); 
	st.enter((std::string )"Second");
	st.insert("delta", (char *)"dog"); 
	st.insertGlobal("echo", (char *)"elk"); 

	printf("Print symbol table.\n");
	st.print(pointerPrintStr);


	printf("This is how you might use insert and lookup in your compiler.\n");
	st.leave();    // second no longer exists
	st.enter((std::string )"Third");
	if (st.insert("charlie", (char *)"cat")) printf("success\n"); else  printf("FAIL\n");
	if (st.insert("charlie", (char *)"pig")) printf("success\n"); else  printf("FAIL\n"); 
	if (st.lookup("charlie")) printf("success\n"); else  printf("FAIL\n"); 
	if (st.lookup("mirage")) printf("success\n"); else  printf("FAIL\n"); 

	printf("Print symbol table.\n");
	st.print(pointerPrintStr);
	fflush(stdout);

	printf("\nGeneral Lookup\n");
	for (int i=0; i<wordsLen; i++) {
		void *data;

		if ((data = st.lookup(words[i]))==NULL) printf("%s: %s\n", words[i].c_str(), (char *)"NULL");
		else printf("%s: %s\n", words[i].c_str(), (char *)data);
	}

	printf("\nGlobal Lookup\n");
	for (int i=0; i<wordsLen; i++) {
		void *data;

		if ((data = st.lookupGlobal(words[i]))==NULL) printf("%s: %s\n", words[i].c_str(), (char *)"NULL");
		else printf("%s: %s\n", words[i].c_str(), (char *)data);
	}

	printf("Test that apply works.\n");
	counter = 0;
	st.applyToAllGlobal(countSymbols);
	printf("Number of global symbols: %d\n", counter);

	st.insert((char *)"gnu", (char *)"goat");
	st.lookup((char *)"gnu");

	st.insertGlobal((char *)"gnu", (char *)"grebe");
	st.lookup((char *)"gnu");
	st.lookupGlobal((char *)"gnu");

	return true;
}
*/