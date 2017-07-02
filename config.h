#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#pragma once

typedef struct 
{
    char        addr[20];
    uint16_t    port;
    char        user[20];
    char        password[20];
    char        realname[20];
} config;

void read_config(config*, char*);
int deserialize(config*, FILE*);
int serialize(config*, char*);
