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
#include <ncurses.h>
#include "config.h"
#include "stringse.h"
#include "IRC.h"

#define BUF_SIZE 512

int validate_ip(char *ipAddress) { //Code shamelessly adapted from SO
  struct sockaddr_in sa;
  int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
  return result != 0;
}

struct addrinfo *dns_magic(char *addr, int port_i, struct addrinfo *hints)
{
    struct addrinfo *res;
    char port[5];
    sprintf(port, "%i", port_i);
    int status;
    if ((status = getaddrinfo(addr, port, hints, &res)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        endwin();
        exit(2);
    }
    return res;
}

int irc_connect(config *conf)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        perror("sock");
        endwin();
        exit(1);
    }

    struct sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(conf->port);
    inet_aton(conf->addr, &serv_addr.sin_addr);
    memset(&(serv_addr.sin_zero), '\0', 8);

    int status = connect(sock, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr));

    if (status == -1)
    {
        perror("connect");
        endwin();
        exit(1);
    }

    return sock;
}

void irc_login(int sock_fd, config *conf)
{
    char NICK_STRING[64];
    char USER_STRING[128];

    sprintf(NICK_STRING, "NICK %s\n", conf->user);
    sprintf(USER_STRING, "USER %s * 0 :%s\n", conf->user, strlen(conf->realname) == 0 ? conf->user : conf->realname);

    send(sock_fd, NICK_STRING, strlen(NICK_STRING), 0);
    send(sock_fd, USER_STRING, strlen(USER_STRING), 0);
}

void irc_pong(int sock_fd, char *buf)
{
    int len = strlen(buf);
    char pong_string[len];
    char *pinged_input = strsub(buf, 5, len-1);
    sprintf(pong_string, "PONG %s\n", pinged_input);
    send(sock_fd,pong_string, len, 0);
}

char *irc_switch(int sock_fd, char *buf, char *cur_chan)
{
    char *channel = strsub(buf, 6, strlen(buf) - 2);
    char join_buffer[64];
    if (strcmp(cur_chan, "NONE"))
    {
        sprintf(join_buffer, "PART %s\n JOIN %s\n", cur_chan, channel);
    } else {
        sprintf(join_buffer, "JOIN %s\n", channel);
    }
    send(sock_fd, join_buffer, strlen(join_buffer), 0);
    return channel;
}

int irc_message(int sock_fd, char *channel, char *buf)
{
    char full_message[BUF_SIZE]; //If you're sending more than 500 characters people will hate you anyway so yolo
    if (strcmp(channel, "NONE") == 0)
    {
        printw("You can't send a message to a NONE channel!\n");
        refresh();
        return 1;
    }
    sprintf(full_message, "PRIVMSG %s :%s", channel, buf);
    send(sock_fd, full_message, strlen(full_message), 0);
    return 0;
}