#include "code.h"
#include "parser.h"

#include <stdlib.h>
#include <string.h>

typedef struct
{
    const char *mnemonic;
    const char *bitString;
} MnemonicToBitString_t;

typedef const char *(*Code_dest_t)(const char *);
typedef const char *(*Code_comp_t)(const char *);
typedef const char *(*Code_jump_t)(const char *);

static MnemonicToBitString_t _jumpTable[] = {
    {"JGT", "001"},
    {"JEQ", "010"},
    {"JGE", "011"},
    {"JLT", "100"},
    {"JNE", "101"},
    {"JLE", "110"},
    {"JMP", "111"}};

static MnemonicToBitString_t _compTable[] = {
    {"0", "0101010"},
    {"1", "0111111"},
    {"-1", "0111010"},
    {"D", "0001100"},
    {"A", "0110000"},
    {"!D", "0001101"},
    {"!A", "0110001"},
    {"-D", "0001111"},
    {"-A", "0110011"},
    {"D+1", "0011111"},
    {"A+1", "0110111"},
    {"D-1", "0001110"},
    {"A-1", "0110010"},
    {"D+A", "0000010"},
    {"D-A", "0010011"},
    {"A-D", "0000111"},
    {"D&A", "0000000"},
    {"D|A", "010101"},
    {"M", "1110000"},
    {"!M", "1110001"},
    {"-M", "1110011"},
    {"M+1", "1110111"},
    {"M-1", "1110010"},
    {"D+M", "1000010"},
    {"D-M", "1010011"},
    {"M-D", "1000111"},
    {"D&M", "1000000"},
    {"D|M", "1010101"}};

static MnemonicToBitString_t _destTable[] = {
    {"M", "001"},
    {"D", "010"},
    {"DM", "011"},
    {"A", "100"},
    {"AM", "101"},
    {"AD", "110"},
    {"ADM", "111"}};

static int _Code_cmp_Mnemonic(const void *a, const void *b);
static int _Code_cmp_Char(const void *a, const void *b);

static const char *Code_jump_setup(const char *_jump);
static const char *Code_jump_quick(const char *_jump);
static Code_jump_t _jump_ptr = Code_jump_setup;

static const char *Code_dest_setup(const char *_dest);
static const char *Code_dest_quick(const char *_dest);
static Code_dest_t _dest_ptr = Code_dest_setup;

static const char *Code_comp_setup(const char *_comp);
static const char *Code_comp_quick(const char *_comp);
static Code_comp_t _comp_ptr = Code_comp_setup;

static void _Code_StringMultipleConcat(char *dst, const void **src, const size_t *srcLength, size_t count);

const char *Code_dest(const char *_dest)
{
    return _dest_ptr(_dest);
}

const char *Code_comp(const char *_comp)
{
    return _comp_ptr(_comp);
}

const char *Code_jump(const char *_jump)
{
    return _jump_ptr(_jump);
}

void Code_int2bitString(char *buffer16, int value)
{
    int i = 0;
    unsigned int v = *(unsigned int *)(&value);

    memset(buffer16, '0', 15);
    buffer16[15] = '\0';
    while (v > 0)
    {
        if (v % 2)
            buffer16[14 - i] = '1';
        v >>= 1;
        i += 1;
        if (i > 14)
            break;
    }
}

// ================================

static int _Code_cmp_Mnemonic(const void *a, const void *b)
{
    MnemonicToBitString_t *c = (MnemonicToBitString_t *)a;
    MnemonicToBitString_t *d = (MnemonicToBitString_t *)b;

    return strcmp(c->mnemonic, d->mnemonic);
}

static int _Code_cmp_Char(const void *a, const void *b)
{
    char *c = (char *)a;
    char *d = (char *)b;

    if (*c > *d)
        return 1;
    else if (*c < *d)
        return -1;
    else
        return 0;
}

static const char *Code_jump_setup(const char *_jump)
{
    qsort(_jumpTable, sizeof(_jumpTable) / sizeof(_jumpTable[0]), sizeof(_jumpTable[0]), _Code_cmp_Mnemonic);
    _jump_ptr = Code_jump_quick;
    return Code_jump_quick(_jump);
}

static const char *Code_jump_quick(const char *_jump)
{
    MnemonicToBitString_t key;
    MnemonicToBitString_t *p;

    if (_jump == NULL)
        return NULL;
    key.mnemonic = _jump;
    p = (MnemonicToBitString_t *)bsearch(&key, _jumpTable, sizeof(_jumpTable) / sizeof(_jumpTable[0]), sizeof(_jumpTable[0]), _Code_cmp_Mnemonic);
    if (!p)
        return NULL;
    else
        return p->bitString;
}

static const char *Code_dest_setup(const char *_dest)
{
    qsort(_destTable, sizeof(_destTable) / sizeof(_destTable[0]), sizeof(_destTable[0]), _Code_cmp_Mnemonic);
    _dest_ptr = Code_dest_quick;
    return Code_dest_quick(_dest);
}

static const char *Code_dest_quick(const char *_dest)
{
    MnemonicToBitString_t key;
    MnemonicToBitString_t *p;
    size_t length;
    char *q;

    if (_dest == NULL)
        return NULL;
    length = strlen(_dest);
    q = (char *)malloc(length + 1);
    if (q == NULL)
        return NULL;
    memcpy(q, _dest, length + 1);
    qsort(q, length, sizeof(*q), _Code_cmp_Char);
    key.mnemonic = q;
    p = (MnemonicToBitString_t *)bsearch(&key, _destTable, sizeof(_destTable) / sizeof(_destTable[0]), sizeof(_destTable[0]), _Code_cmp_Mnemonic);
    free(q);
    if (!p)
        return NULL;
    else
        return p->bitString;
}

static const char *Code_comp_setup(const char *_comp)
{
    qsort(_compTable, sizeof(_compTable) / sizeof(_compTable[0]), sizeof(_compTable[0]), _Code_cmp_Mnemonic);
    _comp_ptr = Code_comp_quick;
    return Code_comp_quick(_comp);
}

static const char *Code_comp_quick(const char *_comp)
{
    static const char spacingCharacters[] = {' '};

    char *p, *q, *r, *s;
    size_t length[4];
    MnemonicToBitString_t key;
    MnemonicToBitString_t *result;

    char *multipleConcatArray[3];
    size_t multipleConcatLengthArray[sizeof(multipleConcatArray) / sizeof(multipleConcatArray[0])];

    char tempString[2];

    length[2] = strlen(_comp);
    p = strpbrk(_comp, "-!+&|");
    if (p == NULL)
    {
        s = (char *)malloc(length[2] + 1);
        if (s == NULL)
            return NULL;
        memcpy(s, _comp, length[2] + 1);
    }
    else
    {
        length[0] = (size_t)p - (size_t)_comp;
        length[1] = length[2] - ((size_t)(p + 1) - (size_t)_comp);
        q = (char *)malloc(length[0] + 1);
        r = (char *)malloc(length[1] + 1);
        if (!q || !r)
            goto Code_comp_quick_failure;
        memcpy(q, _comp, length[0]);
        memcpy(r, p + 1, length[1] + 1);
        q[length[0]] = '\0';
        ParserRemoveAtBothSideOfLine(q, spacingCharacters, length + 0, sizeof(spacingCharacters));
        ParserRemoveAtBothSideOfLine(r, spacingCharacters, length + 1, sizeof(spacingCharacters));
        if ((length[0] != 0 && length[0] != 1) || length[1] != 1)
            goto Code_comp_quick_failure;
        length[3] = length[0] + length[1] + 1;
        s = (char *)malloc(length[3] + 1);
        if (s == NULL)
            goto Code_comp_quick_failure;
        tempString[0] = *p;
        tempString[1] = '\0';
        multipleConcatArray[0] = q;
        multipleConcatArray[1] = tempString;
        multipleConcatArray[2] = r;
        multipleConcatLengthArray[0] = length[0];
        multipleConcatLengthArray[1] = 1;
        multipleConcatLengthArray[2] = length[1];
        _Code_StringMultipleConcat(s, (const void **)multipleConcatArray, multipleConcatLengthArray, sizeof(multipleConcatArray) / sizeof(multipleConcatArray[0]));
        free(q);
        free(r);
    }

    key.mnemonic = s;
    result = (MnemonicToBitString_t *)bsearch(&key, _compTable, sizeof(_compTable) / sizeof(_compTable[0]), sizeof(_compTable[0]), _Code_cmp_Mnemonic);
    free(s);
    if (result == NULL)
        return NULL;
    else
        return result->bitString;

Code_comp_quick_failure:
    free(q);
    free(r);
    return NULL;
}

static void _Code_StringMultipleConcat(char *dst, const void **src, const size_t *srcLength, size_t count)
{
    size_t i, j;

    for (i = 0, j = 0; i < count; i += 1)
    {
        memcpy(dst + j, src[i], srcLength[i]);
        j += srcLength[i];
    }
}
