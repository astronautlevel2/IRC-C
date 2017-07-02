#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"
#include "IRC.h"

#define BUF_SIZE 512

int login_sent = 0;

int main(int argc, char **argv)
{
	config *conf;
	conf = malloc(sizeof(*conf));
	read_config(conf);

	fd_set master;
	fd_set read_fds;
	struct timeval tv;
	int status;

	tv.tv_sec = 1;
	tv.tv_usec = 0;

	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	int sock_fd = irc_connect(conf);
	irc_login(sock_fd, conf);

	FD_SET(sock_fd, &master);
	FD_SET(0, &master);

	for (;;)
	{
		read_fds = master;
		status = select(sock_fd+1,&read_fds,NULL,NULL,&tv);
		if (status == -1) 
		{
			perror("select");
			exit(1);
		} 
		else if (status == 0)
		{
			tv.tv_sec = 1;
			tv.tv_usec = 0;
		}
		
		if (FD_ISSET(sock_fd, &read_fds))
		{
			char buf[BUF_SIZE];
			int bytes_received = 0;

			for (;;)
			{
				int status = recv(sock_fd, &buf[bytes_received], 1, 0);
				if ((buf[bytes_received] == '\n' || buf[bytes_received] == '\r') && bytes_received == 0)
				{
					buf[bytes_received] = '\0';
					break;
				} 
				else if (buf[bytes_received] == '\n' || buf[bytes_received] == '\r') 
				{
					buf[bytes_received] = '\n';
					break;
				} else {
					bytes_received++;
				}
				if (status == 0) {
					puts("Remote host closed connection");
					exit(1);
				}
			}

			buf[bytes_received + 1] = '\0';
			if(strcmp(strsub(buf, 0, 4), "PING") == 0)
			{
				printf("%s", buf);
				irc_pong(sock_fd, buf);
			} else {
				printf("%s", buf);
			}
		}

		if (FD_ISSET(STDIN_FILENO, &read_fds))
		{
			ssize_t r;
			char buf[BUF_SIZE + 2];
			memset(&buf, 0, BUF_SIZE);

			r = read(STDIN_FILENO, buf, BUF_SIZE);

			if (buf[r-1] ==  '\n')
			{
				buf[r-1] = '\r';
				buf[r] = '\n';
			}

			r = send(sock_fd, buf, r + 1, 0);
		}
	}
}