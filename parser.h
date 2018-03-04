#ifndef _PARSER_H_LOADED
#define _PARSER_H_LOADED

int ParserInit(const char *filename);
int ParserExit(void);

#define A_COMMAND 1
#define C_COMMAND 2
#define L_COMMAND 3

int hasMoreCommands(void);
int advance(void);
int commandType(void);
const char *symbol(void);
const char *dest(void);
const char *comp(void);
const char *jump(void);

#define PARSER_ERROR_ALREADY_OPENED 1
#define PARSER_ERROR_CANNOT_OPEN 2
#define PARSER_ERROR_FILE_CLOSED 3
#define PARSER_ERROR_EOF_REACHED 4
#define PARSER_ERROR_CANNOT_READ 5
#define PARSER_ERROR_NO_MEMORY 6
#define PARSER_ERROR_EMPTY_LINE 7
#define PARSER_ERROR_LINE_TOO_LONG 8

#include <string.h>
size_t ParserRemoveAtEndOfLine(char *string, const char *unwantedCharacters, size_t *stringLength, size_t characterLength);
size_t ParserRemoveAtStartOfLine(char *string, const char *unwantedCharacters, size_t *stringLength, size_t characterLength);
size_t ParserRemoveAtBothSideOfLine(char *string, const char *unwantedCharacters, size_t *stringLength, size_t characterLength);

#endif
