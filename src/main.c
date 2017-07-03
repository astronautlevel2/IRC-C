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
    // Get user-specified config path, otherwise use default
    char *config_path;
    if (argc == 2)
    {
        config_path = argv[1];
    } else {
        config_path = "config";
    }

    // Create conf structure and read in conf structure from config_path
    config *conf;
    conf = malloc(sizeof(*conf));
    read_config(conf, config_path);

    // Create a couple variables we need
    fd_set master;
    fd_set read_fds;
    struct timeval tv;
    int status;

    // Make select() time out after 1 second
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    // Initialize the fd sets
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    
    // Curses!
    initscr(); 
    cbreak();
    noecho();
    scrollok(stdscr, true);

    // Get sock_fd, connect to IRC server specified in conf
    int sock_fd = irc_connect(conf);
    irc_login(sock_fd, conf);

    // Add our sock_fd and stdin to master fd set 
    FD_SET(sock_fd, &master);
    FD_SET(0, &master);

    // Create a few more variables and initialize them
    char input_buffer[BUF_SIZE];
    memset(&input_buffer, 0, BUF_SIZE);
    int bytes_input = 0;

    char channel[30];
    strcpy(channel, "NONE");

    // Ugly infinite loop
    for (;;)
    {
        // Select modifies our set, so we copy it at the last second
        read_fds = master;
        status = select(sock_fd+1,&read_fds,NULL,NULL,&tv);
        if (status == -1) // If select returned -1, something seriously went wrong
        {
            perror("select");
            endwin();
            exit(1);
        } 
        else if (status == 0) // Timeout, reset our timeval and try again
        {
            tv.tv_sec = 1;
            tv.tv_usec = 0;
        }
        
        if (FD_ISSET(sock_fd, &read_fds)) // Oh boy, our sock_fd is ready for reading!
        {
            // Create an input buf
            char buf[BUF_SIZE];
            int bytes_received = 0;

            for (;;) // I hate this and should probably rewrite it to be more like stdin at some point
            {
                int status = recv(sock_fd, &buf[bytes_received], 1, 0); // Receive bytes, 1 byte at a time
                if ((buf[bytes_received] == '\n' || buf[bytes_received] == '\r') && bytes_received == 0)    // If got a newline as the first character
                {                                                                                           // it's a stray message, ignore it
                    buf[bytes_received] = '\0';
                    break;
                } 
                else if (buf[bytes_received] == '\n' || buf[bytes_received] == '\r')  // If we get a newline and it's not the first character, break, we got a message
                {
                    buf[bytes_received] = '\n';
                    break;
                } else { // Increment the number of bytes received
                    bytes_received++;
                }
                if (status == 0) { // Server closed connection with us
                    puts("Remote host closed connection");
                    endwin();
                    exit(1);
                }
            }

            buf[bytes_received + 1] = '\0'; // Make sure we have a null terminator. Shouldn't be an issue, but you never know.
            if(strcmp(strsub(buf, 0, 4), "PING") == 0) // We got a ping message. Silently handle it.
            {
                irc_pong(sock_fd, buf);
            } else {
                printw("\r%s", buf); // Return to beginning of line. May need to call delete_line here.
                printw("%s> %s", channel, input_buffer); // Print out the neat channel thinger
                refresh();
            }
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) // We have a character input!
        {
            char c = getch();
            if (c == 127 && bytes_input != 0) // If it's a backspace and we have actual values, we curse curses and then delete the previous character
            {
                bytes_input--; // Gotta decrement this
                int x = 0;
                int y = 0;
                getyx(stdscr, y, x);
                move(y, x-1);
                delch();
                refresh();
            } else if (c == 127) {
                // Do nothing if it's a backspace and we have nothing in the input buffer
            } else { // Not a backspace, so we add it to the input_buffer buffer and be on our way
                input_buffer[bytes_input] = c;
                printw("%c", input_buffer[bytes_input]); // Gotta manually echo because curses
                refresh();
                bytes_input++;
            }

            if (input_buffer[bytes_input - 1] ==  '\n') // Newline, handle this command
            {
                input_buffer[bytes_input - 1] = '\r'; // Change to \r\n, just because some IRC servers require it, but none disallow it
                input_buffer[bytes_input] = '\n';
                if (input_buffer[0] == '/') // We got a command boys!
                {
                    if (strcmp(strsub(input_buffer, 0, 5), "/JOIN") == 0) // Do the join thing
                    {
                        strcpy(channel, irc_switch(sock_fd, input_buffer, channel)); // This both sends the IRC command to JOIN and updates the channel header
                    } else {
                        send(sock_fd, strsub(input_buffer, 1, strlen(input_buffer)), strlen(input_buffer) - 1, 0); // No implementation, pass it on to the server
                    }
                } else {
                    irc_message(sock_fd, channel, input_buffer); // No /, so it's just a normal message. Send it.
                }
                printw("%s> ", channel); // Print the cool channel thing
                refresh();
                memset(&input_buffer, 0, bytes_input); // Reset for future use
                bytes_input = 0;
            }
        }
    }
}