#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include "config.h"

#pragma once

int validate_ip(char*);
struct addrinfo *dns_magic(char*, int, struct addrinfo*);
int irc_connect(config*);
void irc_login(int, config*);
void irc_pong(int, char*);
char *irc_switch(int, char*, char*);
int irc_message(int, char*, char*);