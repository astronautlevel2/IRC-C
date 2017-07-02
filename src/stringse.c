#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "stringse.h"

char *strsub(char *input, int start, int len)
{
    char* substr;
    substr = malloc((len - start) + 1); //Extra byte for null terminator
    memcpy(substr, &input[start], len - start);
    substr[len] = '\0';
    return substr;
}