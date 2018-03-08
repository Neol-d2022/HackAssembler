#include "code.h"
#include "parser.h"
#include "symboltable.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    char bitString[16];
    const char *bitStrings[3];
    const char *_symbol, *_dest, *_comp, *_jump;
    unsigned int lineCount, variableAddressCount, instructionAddressCount;
    int i, r, t;
    int error, pass;
    int inputValue;

    for (i = 1; i < argc; i += 1)
    {
        if ((r = SymbolTableInit()) != 0)
        {
            fprintf(stderr, "[ERROR] Module SymbolTable failed to initialize (%d).\n", r);
            return 1;
        }
        pass = 1;
        variableAddressCount = 15;
        instructionAddressCount = 0;
    main_pass_loop:
        if ((r = ParserInit(argv[i])) != 0)
        {
            fprintf(stderr, "[WARNING] Module Parser failed to parse file '%s' (%d).\n", argv[i], r);
            continue;
        }

        error = 0;
        lineCount = 1;
        while (hasMoreCommands())
        {
            switch (r = advance())
            {
            case 0:
                switch (t = commandType())
                {
                case A_COMMAND:
                    _symbol = symbol();
                    if (_symbol == NULL)
                    {
                        error = 1;
                        fprintf(stderr, "[ERROR] Module Parser failed to parse file '%s' on line %u:\n\tSymbol is NULL (A).\n", argv[i], lineCount);
                        break;
                    }
                    if (pass == 1)
                        instructionAddressCount += 1;
                    else
                    {
                        if (sscanf(_symbol, "%d", &inputValue) == 1)
                        {
                            Code_int2bitString(bitString, inputValue);
                            fprintf(stdout, "0%s\n", bitString);
                        }
                        else if (contains(_symbol))
                        {
                            inputValue = GetAddress(_symbol);
                            fprintf(stderr, "[INFO] Module Symbol Table retrieve symbol '%s' with address %d on line %u.\n", _symbol, inputValue, lineCount);
                            Code_int2bitString(bitString, inputValue);
                            fprintf(stdout, "0%s\n", bitString);
                        }
                        else if ((r = addEntry(_symbol, variableAddressCount += 1)) != 0)
                        {
                            error = 1;
                            fprintf(stderr, "[ERROR] Module Symbol Table failed to add the symbol(var) '%s' on line %u\n\tFile '%s'.\n", _symbol, lineCount, argv[i]);
                        }
                        else
                        {
                            fprintf(stderr, "[INFO] Module Symbol Table add symbol '%s' with variable address %d on line %u.\n", _symbol, variableAddressCount, lineCount);
                            Code_int2bitString(bitString, variableAddressCount);
                            fprintf(stdout, "0%s\n", bitString);
                        }
                    }
                    break;
                case C_COMMAND:
                    if (pass == 1)
                        instructionAddressCount += 1;
                    else
                    {
                        _dest = dest();
                        _comp = comp();
                        _jump = jump();
                        bitStrings[1] = Code_comp(_comp);

                        if (!_dest)
                            bitStrings[0] = "000";
                        else
                            bitStrings[0] = Code_dest(_dest);
                        if (!_comp)
                        {
                            error = 1;
                            fprintf(stderr, "[ERROR] Module Parser failed to parse file '%s' on line %u:\n\tcomp is NULL.\n", argv[i], lineCount);
                            break;
                        }
                        if (!_jump)
                            bitStrings[2] = "000";
                        else
                            bitStrings[2] = Code_jump(_jump);
                        if (!bitStrings[0])
                        {
                            error = 1;
                            fprintf(stderr, "[ERROR] Module Code failed to turn dest '%s' to bit string on line %u.\n", _dest, lineCount);
                            break;
                        }
                        if (!bitStrings[1])
                        {
                            error = 1;
                            fprintf(stderr, "[ERROR] Module Code failed to turn comp '%s' to bit string on line %u.\n", _comp, lineCount);
                            break;
                        }
                        if (!bitStrings[2])
                        {
                            error = 1;
                            fprintf(stderr, "[ERROR] Module Code failed to turn jump '%s' to bit string on line %u.\n", _jump, lineCount);
                            break;
                        }
                        fprintf(stdout, "111%s%s%s\n", bitStrings[1], bitStrings[0], bitStrings[2]);
                    }
                    break;
                case L_COMMAND:
                    _symbol = symbol();
                    if (_symbol == NULL)
                    {
                        error = 1;
                        fprintf(stderr, "[ERROR] Module Parser failed to parse file '%s' on line %u:\n\tSymbol is NULL (L).\n", argv[i], lineCount);
                        break;
                    }
                    if (pass == 1)
                    {
                        if (contains(_symbol))
                        {
                            fprintf(stderr, "[WARNING] Module Symbol Table detected duplicated symbols '%s' on line %u.\n", _symbol, lineCount);
                        }
                        else
                        {
                            if (addEntry(_symbol, instructionAddressCount) != 0)
                            {
                                error = 1;
                                fprintf(stderr, "[ERROR] Module Symbol Table failed to add the symbol(label) '%s' on line %u\n\tFile '%s'.\n", _symbol, lineCount, argv[i]);
                            }
                            else
                                fprintf(stderr, "[INFO]  Module Symbol Table add symbol '%s' with instruction address %d on %u line(s).\n", _symbol, instructionAddressCount, lineCount);
                        }
                    }
                    break;
                default:
                    error = 1;
                    fprintf(stderr, "[ERROR] Module Parser failed to parse file '%s' on line %u:\n\tUnknown command type: %d.\n", argv[i], lineCount, t);
                    break;
                }
                break;
            case PARSER_ERROR_FILE_CLOSED:
                error = 1;
                fprintf(stderr, "[ERROR] Module Parser failed to parse file '%s' with unexpected error code (%d, FILE NOT OPENED) on line %u.\n", argv[i], r, lineCount);
                break;
            case PARSER_ERROR_EOF_REACHED:
                fprintf(stderr, "[INFO] Module Parser reach EOF parsing file '%s' after %u line(s).\n", argv[i], lineCount);
                break;
            case PARSER_ERROR_CANNOT_READ:
                error = 1;
                fprintf(stderr, "[ERROR] Module Parser failed to parse file '%s' on line %u:\n\tI/O read error, error code from OS: %d.\n", argv[i], lineCount, errno);
                break;
            case PARSER_ERROR_EMPTY_LINE:
                fprintf(stderr, "[INFO] Module Parser detected an empty line parsing file '%s' on line %u.\n", argv[i], lineCount);
                break;
            case PARSER_ERROR_LINE_TOO_LONG:
                error = 1;
                fprintf(stderr, "[ERROR] Module Parser failed to parse file '%s' on line %u:\n\tToo much character on a single line.\n", argv[i], lineCount);
                break;
            default:
                error = 1;
                fprintf(stderr, "[ERROR] Module Parser failed to parse file '%s' with unexpected error code (%d) on line %u.\n", argv[i], r, lineCount);
                break;
            }
            if (error)
            {
                fprintf(stderr, "[ERROR] Module Parser failed to parse file '%s'.\n", argv[i]);
                break;
                ;
            }
            lineCount += 1;
        }
        ParserExit();
        if (error)
        {
            fprintf(stderr, "[WARNING] Skipping the file '%s' that failed to parse with pass = %d.\n", argv[i], pass);
            SymbolTableExit();
            continue;
        }
        if (pass < 2)
        {
            fprintf(stderr, "[INFO] Module Parser has finished parsing file '%s' with pass = %d.\n", argv[i], pass);
            pass += 1;
            goto main_pass_loop;
        }
        SymbolTableExit();
    }

    return 0;
}
