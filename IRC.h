#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"

#pragma once

char *strsub(char*, int, int);
int irc_connect(config*);
int irc_login(int, config*);
int irc_pong(int, char*);