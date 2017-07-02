#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include "stringse.h"

char *strsub(char *input, int start, int len)
{
    char* substr;
    substr = malloc((len - start) + 1); //Extra byte for null terminator
    memcpy(substr, &input[start], len - start);
    substr[len] = '\0';
    return substr;
}

void delete_line(char *buf)
{
    printw("\r"); //Carriage return to move back to the beginning of the line
    for (uint8_t i = 0; i < strlen(buf); i++) //u8 to stop gcc from bitching about comparing a signed to an unsigned
    {
        printw(" ");
    }
    refresh();
}