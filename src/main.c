#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include "config.h"
#include "IRC.h"
#include "stringse.h"

#define BUF_SIZE 512

int main(int argc, char **argv)
{
    char *config_path;
    if (argc == 2)
    {
        config_path = argv[1];
    } else {
        config_path = "config";
    }
    config *conf;
    conf = malloc(sizeof(*conf));
    read_config(conf, config_path);

    fd_set master;
    fd_set read_fds;
    struct timeval tv;
    int status;

    tv.tv_sec = 1;
    tv.tv_usec = 0;

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    initscr(); //Curses!
    cbreak();
    noecho();
    scrollok(stdscr, true);

    int sock_fd = irc_connect(conf);
    irc_login(sock_fd, conf);

    FD_SET(sock_fd, &master);
    FD_SET(0, &master);

    char input_buffer[BUF_SIZE];
    memset(&input_buffer, 0, BUF_SIZE);
    int bytes_input = 0;

    char channel[30];
    strcpy(channel, "NONE");

    for (;;)
    {
        read_fds = master;
        status = select(sock_fd+1,&read_fds,NULL,NULL,&tv);
        if (status == -1) 
        {
            perror("select");
            endwin();
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
                    endwin();
                    exit(1);
                }
            }

            buf[bytes_received + 1] = '\0';
            if(strcmp(strsub(buf, 0, 4), "PING") == 0)
            {
                irc_pong(sock_fd, buf);
            } else {
                // delete_line(input_buffer);
                printw("\r%s", buf);
                printw("%s> %s", channel, input_buffer);
                refresh();
            }
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds))
        {
            char c = getch();
            if (c == 127)
            {
                bytes_input--;
                int x = 0;
                int y = 0;
                getyx(stdscr, y, x);
                move(y, x-1);
                delch();
                refresh();
            } else {
                input_buffer[bytes_input] = c;
                printw("%c", input_buffer[bytes_input]);
                refresh();
                bytes_input++;
            }

            if (input_buffer[bytes_input - 1] ==  '\n')
            {
                refresh();
                input_buffer[bytes_input - 1] = '\r';
                input_buffer[bytes_input] = '\n';
                if (input_buffer[0] == '/')
                {
                    if (strcmp(strsub(input_buffer, 0, 5), "/JOIN") == 0)
                    {
                        strcpy(channel, irc_switch(sock_fd, input_buffer, channel));
                    } else {
                        send(sock_fd, strsub(input_buffer, 1, strlen(input_buffer)), strlen(input_buffer) - 1, 0);
                    }
                } else {
                    irc_message(sock_fd, channel, input_buffer);
                }
                printw("%s> ", channel);
                refresh();
                memset(&input_buffer, 0, bytes_input);
                bytes_input = 0;
            }
        }
    }
}