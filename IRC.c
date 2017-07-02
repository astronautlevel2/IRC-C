#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"

char *strsub(char *input, int start, int len)
{
	char* substr;
	substr = malloc((len - start) + 1); //Extra byte for null terminator
	memcpy(substr, &input[start], len - start);
	substr[len] = '\0';
	return substr;
}

int irc_connect(config *conf)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock < 0)
	{
		perror("sock");
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
		exit(1);
	}

	return sock;
}

int irc_login(int sock_fd, config *conf)
{
	char NICK_STRING[64];
	char USER_STRING[128];

	sprintf(NICK_STRING, "NICK %s\n", conf->user);
	sprintf(USER_STRING, "USER %s * 0 :%s\n", conf->user, strlen(conf->realname) == 0 ? conf->user : conf->realname);

	send(sock_fd, NICK_STRING, strlen(NICK_STRING), 0);
	send(sock_fd, USER_STRING, strlen(USER_STRING), 0);
}

int irc_pong(int sock_fd, char *buf)
{
	int len = strlen(buf);
	char pong_string[len];
	char *pinged_input = strsub(buf, 5, len-1);
	sprintf(pong_string, "PONG %s\n", pinged_input);
	send(sock_fd,pong_string, len, 0);
}