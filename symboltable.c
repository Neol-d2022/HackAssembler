#include "symboltable.h"
#include "avl_tree.h"

#include <stdlib.h>
#include <string.h>

typedef struct
{
    const char *symbol;
    int value;
} SymbolTableEntry_t;

static int _SymbolTable_cmp_SymbolTableEntry(void *a, void *b);
static void _SymbolTable_free_SymbolTableEntry(void *p);
static SymbolTableEntry_t *_SymbolTable_getBySymbol(const char *symbol);

static const SymbolTableEntry_t _symbolTableBuiltIn[] = {
    {"R0", 0},
    {"R1", 1},
    {"R2", 2},
    {"R3", 3},
    {"R4", 4},
    {"R5", 5},
    {"R6", 6},
    {"R7", 7},
    {"R8", 8},
    {"R9", 9},
    {"R10", 10},
    {"R11", 11},
    {"R12", 12},
    {"R13", 13},
    {"R14", 14},
    {"R15", 15},
    {"SCREEN", 16384},
    {"KBD", 24576},
    {"SP", 0},
    {"LCL", 1},
    {"ARG", 2},
    {"THIS", 3},
    {"THAT", 4}};
static AVL_TREE *_symbolTableTree = NULL;

int SymbolTableInit(void)
{
    size_t i, n;
    SymbolTableEntry_t *entry;

    if (_symbolTableTree != NULL)
        return SYMBOL_TABLE_ERROR_TREE_CREATED;

    _symbolTableTree = AVL_Create(_SymbolTable_cmp_SymbolTableEntry, _SymbolTable_free_SymbolTableEntry);
    if (_symbolTableTree == NULL)
        return SYMBOL_TABLE_ERROR_NO_MEMORY;

    n = sizeof(_symbolTableBuiltIn) / sizeof(_symbolTableBuiltIn[0]);
    for (i = 0; i < n; i += 1)
    {
        entry = (SymbolTableEntry_t *)malloc(sizeof(*entry));
        if (entry == NULL)
        {
            SymbolTableExit();
            return SYMBOL_TABLE_ERROR_NO_MEMORY;
        }
        entry->symbol = (const char *)strdup(_symbolTableBuiltIn[i].symbol);
        if (entry->symbol == NULL)
        {
            free(entry);
            SymbolTableExit();
            return SYMBOL_TABLE_ERROR_NO_MEMORY;
        }
        entry->value = _symbolTableBuiltIn[i].value;
        memcpy(entry, _symbolTableBuiltIn + i, sizeof(*entry));
        if (AVL_Insert(_symbolTableTree, entry) != 1)
        {
            _SymbolTable_free_SymbolTableEntry(entry);
            SymbolTableExit();
            return SYMBOL_TABLE_ERROR_NO_MEMORY;
        }
    }

    return 0;
}

int SymbolTableExit(void)
{
    if (_symbolTableTree == NULL)
        return SYMBOL_TABLE_ERROR_TREE_DESTROYED;
    AVL_Destroy(_symbolTableTree);
    _symbolTableTree = NULL;
    return 0;
}

int addEntry(const char *symbol, int address)
{
    SymbolTableEntry_t *entry;

    if (_symbolTableTree == NULL)
        return SYMBOL_TABLE_ERROR_TREE_DESTROYED;

    if (contains(symbol))
        return SYMBOL_TABLE_ERROR_SYMBOL_EXIST;

    entry = (SymbolTableEntry_t *)malloc(sizeof(*entry));
    if (entry == NULL)
        return SYMBOL_TABLE_ERROR_NO_MEMORY;

    entry->symbol = strdup(symbol);
    if (entry->symbol == NULL)
    {
        free(entry);
        return SYMBOL_TABLE_ERROR_NO_MEMORY;
    }
    entry->value = address;

    if (AVL_Insert(_symbolTableTree, entry) != 1)
    {
        _SymbolTable_free_SymbolTableEntry(entry);
        return SYMBOL_TABLE_ERROR_NO_MEMORY;
    }

    return 0;
}

int contains(const char *symbol)
{
    SymbolTableEntry_t *result = _SymbolTable_getBySymbol(symbol);

    if (result == NULL)
        return 0;
    else
        return 1;
}

int GetAddress(const char *symbol)
{
    SymbolTableEntry_t *result = _SymbolTable_getBySymbol(symbol);

    if (result == NULL)
        return -1;
    else
        return result->value;
}

// ================================

static int _SymbolTable_cmp_SymbolTableEntry(void *a, void *b)
{
    SymbolTableEntry_t *c = (SymbolTableEntry_t *)a;
    SymbolTableEntry_t *d = (SymbolTableEntry_t *)b;

    return strcmp(c->symbol, d->symbol);
}

static void _SymbolTable_free_SymbolTableEntry(void *p)
{
    SymbolTableEntry_t *q = (SymbolTableEntry_t *)p;

    free((void *)q->symbol);
    free(q);
}

static SymbolTableEntry_t *_SymbolTable_getBySymbol(const char *symbol)
{
    SymbolTableEntry_t key;

    if (_symbolTableTree == NULL)
        return NULL;

    key.symbol = symbol;
    return (SymbolTableEntry_t *)AVL_Retrieve(_symbolTableTree, &key);
}
