#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _COMMAND_MAX_LENGTH 256

static FILE *_fileToParse = NULL;

static char *currentCommand;
static char *_symbol;
static char *_dest;
static char *_comp;
static char *_jump;
static size_t currentCommandLength;
static int _lastCommandType;

static int _noNewLineCharAtEnd_ExpectEOF;

static int _ParserAllocateMemory(void);
static void _ParserFreeMemory(void);
static void _ParserTruncateAfterInclusive(char *string, size_t *length, const char **stringsToTrucate, size_t count);

int ParserInit(const char *filename)
{
    FILE *f;

    if (_fileToParse)
        return PARSER_ERROR_ALREADY_OPENED;
    else
    {
        if (_ParserAllocateMemory())
            return PARSER_ERROR_NO_MEMORY;
        else
        {
            f = fopen(filename, "r");
            if (f)
            {
                _fileToParse = f;
                memset(currentCommand, 0, _COMMAND_MAX_LENGTH);
                _lastCommandType = 0;
                _noNewLineCharAtEnd_ExpectEOF = 0;
                return 0;
            }
            else
            {
                free(currentCommand);
                return PARSER_ERROR_CANNOT_OPEN;
            }
        }
    }
}

int ParserExit(void)
{
    FILE *f;

    if (_fileToParse)
    {
        f = _fileToParse;
        _fileToParse = NULL;
        _ParserFreeMemory();
        return fclose(f);
    }
    else
        return PARSER_ERROR_FILE_CLOSED;
}

int hasMoreCommands(void)
{
    if (_fileToParse)
        return (feof(_fileToParse) ? 0 : 1);
    else
        return 0;
}

int advance(void)
{
    static const char newLineChar[] = {' ', '\r', '\n'};
    static const char indentChar[] = {' ', '\t'};
    static const char spacingCharacters[] = {' '};
    static const char *commentString[] = {"#", "//"};

    char buffer[_COMMAND_MAX_LENGTH];
    size_t length;

    int noNewLineChar = 0;

    if (_fileToParse == NULL)
        return PARSER_ERROR_FILE_CLOSED;

    _lastCommandType = 0;
    if (fgets(buffer, sizeof(buffer), _fileToParse) == NULL)
    {
        if (feof(_fileToParse))
            return PARSER_ERROR_EOF_REACHED;
        else
            return PARSER_ERROR_CANNOT_READ;
    }
    else
    {
        length = strlen(buffer);
        if (ParserRemoveAtEndOfLine(buffer, newLineChar, &length, sizeof(newLineChar)) == 0)
        {
            if (_noNewLineCharAtEnd_ExpectEOF == 0)
                noNewLineChar = _noNewLineCharAtEnd_ExpectEOF = 1;
            else
                goto advance_line_too_long;
        }
        _ParserTruncateAfterInclusive(buffer, &length, commentString, sizeof(commentString) / sizeof(commentString[0]));
        ParserRemoveAtEndOfLine(buffer, spacingCharacters, &length, sizeof(spacingCharacters));
        ParserRemoveAtStartOfLine(buffer, indentChar, &length, sizeof(indentChar));
        memcpy(currentCommand, buffer, length + 1);
        currentCommandLength = length;
        if (length == 0)
            return PARSER_ERROR_EMPTY_LINE;
        else if (_noNewLineCharAtEnd_ExpectEOF && !noNewLineChar)
            goto advance_line_too_long;
        else
            return 0;
    }

advance_line_too_long:
    *currentCommand = '\0';
    currentCommandLength = 0;
    return PARSER_ERROR_LINE_TOO_LONG;
}

int commandType(void)
{
    size_t length;
    char *p, *q;

    if (_fileToParse == NULL)
        return 0;
    if ((length = strlen(currentCommand)) == 0)
        return 0;
    if (_lastCommandType)
        return _lastCommandType;

    p = strchr(currentCommand, '(');
    q = strchr(currentCommand, ')');
    if (currentCommand[0] == '@')
        return _lastCommandType = A_COMMAND;
    else if (p != NULL && q != NULL && ((size_t)q > (size_t)p))
        return _lastCommandType = L_COMMAND;
    else
        return _lastCommandType = C_COMMAND;
}

const char *symbol(void)
{
    static const char spacingCharacters[] = {' '};

    size_t length;
    char *p, *q;

    if (_fileToParse == NULL)
        return NULL;

    switch (_lastCommandType)
    {
    case A_COMMAND:
        p = strchr(currentCommand, '@');
        memcpy(_symbol, p + 1, (length = currentCommandLength - ((size_t)(p + 1) - (size_t)currentCommand)) + 1);
        break;
    case C_COMMAND:
        return NULL;
    case L_COMMAND:
        p = strchr(currentCommand, '(');
        q = strchr(currentCommand, ')');
        memcpy(_symbol, p + 1, length = ((size_t)q - (size_t)(p + 1)));
        _symbol[length] = '\0';
        break;
    default:
        return NULL;
    }

    ParserRemoveAtBothSideOfLine(_symbol, spacingCharacters, &length, sizeof(spacingCharacters));
    if (length == 0)
        return NULL;
    return _symbol;
}

const char *dest(void)
{
    static const char spacingCharacters[] = {' '};

    size_t length;
    char *p;

    if (_lastCommandType != C_COMMAND)
        return NULL;

    p = strchr(currentCommand, '=');
    if (p == NULL)
        return NULL;
    else
    {
        memcpy(_dest, currentCommand, length = ((size_t)p - (size_t)currentCommand));
        _dest[length] = '\0';
    }

    ParserRemoveAtBothSideOfLine(_dest, spacingCharacters, &length, sizeof(spacingCharacters));
    return _dest;
}

const char *comp(void)
{
    static const char spacingCharacters[] = {' '};

    size_t length;
    char *p, *q;

    if (_lastCommandType != C_COMMAND)
        return NULL;

    p = strchr(currentCommand, '=');
    q = strchr(currentCommand, ';');
    if (!p && !q)
        memcpy(_comp, currentCommand, (length = currentCommandLength) + 1);
    else if (!p)
    {
        memcpy(_comp, currentCommand, length = ((size_t)q - (size_t)currentCommand));
        _comp[length] = '\0';
    }
    else if (!q)
        memcpy(_comp, p + 1, (length = currentCommandLength - ((size_t)(p + 1) - (size_t)currentCommand)) + 1);
    else
    {
        memcpy(_comp, p + 1, length = ((size_t)q - (size_t)(p + 1)));
        _comp[length] = '\0';
    }

    ParserRemoveAtBothSideOfLine(_comp, spacingCharacters, &length, sizeof(spacingCharacters));
    return _comp;
}

const char *jump(void)
{
    static const char spacingCharacters[] = {' '};

    size_t length;
    char *p;

    if (_lastCommandType != C_COMMAND)
        return NULL;

    p = strchr(currentCommand, ';');
    if (p == NULL)
        return NULL;
    else
        memcpy(_jump, p + 1, (length = currentCommandLength - ((size_t)(p + 1) - (size_t)currentCommand)) + 1);

    ParserRemoveAtBothSideOfLine(_jump, spacingCharacters, &length, sizeof(spacingCharacters));
    return _jump;
}

size_t ParserRemoveAtEndOfLine(char *string, const char *unwantedCharacters, size_t *stringLength, size_t characterLength)
{
    size_t i, j, k, l, m;

    k = *stringLength;
    m = characterLength;
    for (i = k - 1, j = 0; j < k; i -= 1)
    {
        for (l = 0; l < m; l += 1)
            if (string[i] == unwantedCharacters[l])
                break;
        if (l == m)
            break;
        else
            j += 1;
    }
    if (j > 0)
        string[ *stringLength = k - j] = '\0';
    return j;
}

size_t ParserRemoveAtStartOfLine(char *string, const char *unwantedCharacters, size_t *stringLength, size_t characterLength)
{
    size_t i, j, k, l, m;

    k = *stringLength;
    m = characterLength;
    for (i = 0, j = 0; j < k; i += 1)
    {
        for (l = 0; l < m; l += 1)
            if (string[i] == unwantedCharacters[l])
                break;
        if (l == m)
            break;
        else
            j += 1;
    }
    if (j > 0)
        memmove(string, string + j, (*stringLength = k - j) + 1);
    return j;
}

size_t ParserRemoveAtBothSideOfLine(char *string, const char *unwantedCharacters, size_t *stringLength, size_t characterLength)
{
    size_t i, j;

    i = ParserRemoveAtEndOfLine(string, unwantedCharacters, stringLength, characterLength);
    j = ParserRemoveAtStartOfLine(string, unwantedCharacters, stringLength, characterLength);

    return i + j;
}

// ================================

static int _ParserAllocateMemory(void)
{
    currentCommand = malloc(_COMMAND_MAX_LENGTH);
    _symbol = malloc(_COMMAND_MAX_LENGTH);
    _dest = malloc(_COMMAND_MAX_LENGTH);
    _comp = malloc(_COMMAND_MAX_LENGTH);
    _jump = malloc(_COMMAND_MAX_LENGTH);
    if (!currentCommand || !_symbol || !_dest || !_comp || !_jump)
        goto _ParserAllocateMemory_failure;
    return 0;

_ParserAllocateMemory_failure:
    _ParserFreeMemory();
    return PARSER_ERROR_NO_MEMORY;
}

static void _ParserFreeMemory(void)
{
    free(currentCommand);
    free(_symbol);
    free(_dest);
    free(_comp);
    free(_jump);
}

static void _ParserTruncateAfterInclusive(char *string, size_t *length, const char **stringsToTrucate, size_t count)
{
    char *p;
    size_t i;

    for (i = 0; i < count; i += 1)
    {
        p = strstr(string, stringsToTrucate[i]);
        if (p)
        {
            *p = '\0';
            *length = ((size_t)p - (size_t)string) / sizeof(*p);
        }
    }
}
