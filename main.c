#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int i;

    printf("Input arguments: %d\n", argc);
    for (i = 0; i < argc; i += 1)
        printf("%s\n", argv[i]);

    return 0;
}
