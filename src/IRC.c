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
  return result != 0; // if inet_pton returns non-zero, invalid IP
}

struct addrinfo *dns_magic(char *addr, int port_i, struct addrinfo *hints)
{
    struct addrinfo *res;
    char port[6]; //Convert ports int to char*
    sprintf(port, "%i", port_i);
    int status;
    if ((status = getaddrinfo(addr, port, hints, &res)) != 0) // Non-zero? Oops, an error has occured!
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        endwin();
        exit(2);
    }
    return res;
}

int irc_connect(config *conf) // Connect ot the main IRC server
{
    int sock = socket(AF_INET, SOCK_STREAM, 0); // create our sock_fd

    if (sock < 0)
    {
        perror("sock");
        endwin();
        exit(1);
    }

    struct sockaddr_in serv_addr;

    // Create our sockaddr_in, IPv4, and using port and addr from conf
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(conf->port);
    inet_aton(conf->addr, &serv_addr.sin_addr);
    memset(&(serv_addr.sin_zero), '\0', 8); // So we can cast to sockaddr easily

    int status = connect(sock, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr)); // Do the actual connection

    if (status == -1) // Fug, connection failed
    {
        perror("connect");
        endwin();
        exit(1);
    }

    return sock; // Return the sock_fd created
}

void irc_login(int sock_fd, config *conf) // Handle IRC login procedures
{
    char NICK_STRING[64]; 
    char USER_STRING[128];

    sprintf(NICK_STRING, "NICK %s\n", conf->user); // NICK user
    sprintf(USER_STRING, "USER %s * 0 :%s\n", conf->user, strlen(conf->realname) == 0 ? conf->user : conf->realname); // USER user * 0 :Real name

    send(sock_fd, NICK_STRING, strlen(NICK_STRING), 0); // Send them both off to the IRC
    send(sock_fd, USER_STRING, strlen(USER_STRING), 0);
}

void irc_pong(int sock_fd, char *buf) // PONG all PINGs. Handles both initial login PINGs and preiodic heartbeats
{
    int len = strlen(buf);
    char pong_string[len];
    char *pinged_input = strsub(buf, 5, len-1); // Use my horribly broken strsub to grab the PONG message
    sprintf(pong_string, "PONG %s\n", pinged_input); // PONG :PingString
    send(sock_fd,pong_string, len, 0);
}

char *irc_switch(int sock_fd, char *buf, char *cur_chan) // Join/Switch channel
{
    char *channel = strsub(buf, 6, strlen(buf) - 2);
    char join_buffer[64];
    if (strcmp(cur_chan, "NONE")) // If you're currently in a channel, leave it and then join a new one
    {
        sprintf(join_buffer, "PART %s\n JOIN %s\n", cur_chan, channel);
    } else {
        sprintf(join_buffer, "JOIN %s\n", channel); // If you're not, just join a new one
    }
    send(sock_fd, join_buffer, strlen(join_buffer), 0);
    return channel;
}

int irc_message(int sock_fd, char *channel, char *buf)
{
    char full_message[BUF_SIZE]; //If you're sending more than 500 characters people will hate you anyway so
    if (strcmp(channel, "NONE") == 0) // If you're not in a channel, you can't send a message, nimrod
    {
        printw("You can't send a message to a NONE channel!\n");
        refresh();
        return 1;
    }
    sprintf(full_message, "PRIVMSG %s :%s", channel, buf); // PRIVMSG <channel> :Message text
    send(sock_fd, full_message, strlen(full_message), 0);
    return 0;
}