#ifndef _SYMBOL_TABLE_H_LOADED
#define _SYMBOL_TABLE_H_LOADED

int SymbolTableInit(void);
int SymbolTableExit(void);

int addEntry(const char *symbol, int address);
int contains(const char *symbol);
int GetAddress(const char *symbol);

#define SYMBOL_TABLE_ERROR_NO_MEMORY 1
#define SYMBOL_TABLE_ERROR_TREE_DESTROYED 2
#define SYMBOL_TABLE_ERROR_TREE_CREATED 3
#define SYMBOL_TABLE_ERROR_SYMBOL_EXIST 4

#endif
