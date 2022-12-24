#ifndef _SYMBOLTABLE_H_
#define _SYMBOLTABLE_H_
#include <map>
#include <vector>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

extern int warnings;

// // // // // // // // // // // // // // // // // // // // 
//
// Some sample void * printing routines.   User should supply their own.
//
void pointerPrintNothing(void* data);
void pointerPrintAddr(void* data);
void pointerPrintLongInteger(void* data);
void pointerPrintStr(void* data);

// // // // // // // // // // // // // // // // // // // // 
//
// Introduction
//
// This symbol table library supplies basic insert and lookup for
// symbols linked to TreeNode pointers of data.
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
// Class: SymbolTable
//
// Is a stack of scopes.   The global scope is created when the table is
// is constructed and remains for the lifetime of the object instance.
// SymbolTable manages nested scopes as a result.
// 

class SymbolTable
{
	private:
		class Scope;
		std::vector<Scope *> stack;
		int currentOffset;
		bool debugFlg;

	public:
		SymbolTable();
		// sets the debug flags
		void debug(bool state);
		// runs tests to validate the SymbolTable class
		//bool test();
		// what is the depth of the scope stack?
		int depth();
		// print all scopes using data printing function
		void print(void (*printData)(TreeNode*));
		// enter a scope with given name
		void enter(std::string name);
		// enter a function scope with given name
		void enterFunc(std::string name);
		// leave a scope (not allowed to leave global)
		void leave();
		// returns ptr associated with sym anywhere in symbol table (returns NULL if symbol not found)
		TreeNode* lookup(std::string sym);
		// returns ptr associated with sym in current local scope (return NULL if symbol not found)
		TreeNode* lookupLocal(std::string sym);
		// returns ptr associated with sym in current function's parameter scope (return NULL if symbol not found)
		TreeNode* lookupParm(std::string sym, bool isIterVar);
		// returns ptr associated with sym in globals (returns NULL if symbol not found)
		TreeNode* lookupGlobal(std::string sym);
		// inserts new ptr associated with symbol sym in current scope (doesn't insert and returns false if already defined)
		bool insert(std::string sym, TreeNode* node);
		// inserts a new ptr associated with symbol sym  (returns false if already defined)
		bool insertGlobal(std::string sym, TreeNode* node);
		// apply func to all symbol/data pairs in local scope
		void applyToAll(void (*action)(std::string , TreeNode*));
		// apply func to all symbol/data pairs in global scope
		void applyToAllGlobal(void (*action)(std::string , TreeNode*));
		// returns true if the symbol table is currently in the scope of a while or for loop, returns false otherwise
		bool inLoop();
		// checks the global scope for unused symbols
		void checkGlobalUnused();
		// returns the current frame size
		int getCurrentFrameSize();
		// allocates a string constant in global space
		void allocString(TreeNode* node);
};

#endif
