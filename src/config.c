#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <ncurses.h>
#include <netinet/in.h>
#include <netdb.h>
#include "config.h"
#include "IRC.h"

void read_config(config *conf, char *config_path) // This method got out of control, but that's okay. Just a happy accident.
{
    int dns_magic_needed = 0; // Flag to determine if we need to do DNS resolution
    FILE *fp;
    if ((fp = fopen(config_path, "r"))) // If we can open the config file specific, just read from that
    {
        deserialize(conf, fp);
    } else { // If not, get the user input
        printf("Address to connect to: ");
        char temp[20];
        fgets(temp, 20, stdin);
        temp[strlen(temp)-1] = '\0';
        if (validate_ip(temp)) // Valid IP? Just toss it in the config file
        {
            strcpy(conf->addr, temp);
        } else {
            dns_magic_needed = 1; // We need DNS resoltuion
        }
        printf("Port to connect to: ");
        char port_temp[20];
        fgets(port_temp, 20, stdin);
        conf->port = atoi(port_temp); // Convert the string to an int
        printf("Username to use: ");
        fgets(conf->user, 20, stdin);
        conf->user[strlen(conf->user)-1] = '\0';
        printf("Real name (optional): ");
        fgets(conf->realname, 20, stdin);
        conf->realname[strlen(conf->realname)-1] = '\0';
        printf("Password to use (optional): ");
        fgets(conf->password, 20, stdin);
        conf->password[strlen(conf->password)-1] = '\0';
        if (dns_magic_needed) // Turns out we need DNS resolution
        {
            struct addrinfo hints, *res;
            memset(&hints, 0, sizeof hints);
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            res = dns_magic(temp, conf->port, &hints); // res is a linked list (or more accurately, the first element of a linked list) containing all the DNS results
            long ip = ((struct sockaddr_in *)res->ai_addr)->sin_addr.s_addr; // Take the first result, cast it to a sockaddr_in, and get the long value representing IP
            sprintf(temp, "%ld.%ld.%ld.%ld", (ip & 0xFF), ((ip >> 8) & 0xFF), ((ip >> 16) & 0xFF), ((ip >> 24) & 0xFF)); // Perform black magic to get it into a string
            strcpy(conf->addr, temp); // Copy that into the config structure
        }
        serialize(conf, config_path); // Write out the new config to file
    }
}

int serialize(config *conf, char *config_path) // Write struct into file
{
    FILE *fp = fopen(config_path, "w");
    fwrite(conf, sizeof(*conf), 1, fp);
    fclose(fp);
    return 0;
}

int deserialize(config *conf, FILE *fp) // Read file into struct
{
    fread(conf, sizeof(*conf), 1, fp);
    fclose(fp);
    return 0;
}

int setup_config(config *conf) // Just null everything out so weird stuff doesn't happen
{
    memset(conf->addr, '\0', sizeof(conf->addr));
    memset(conf->user, '\0', sizeof(conf->user));
    memset(conf->password, '\0', sizeof(conf->password));
    memset(conf->realname, '\0', sizeof(conf->realname));
    return 0;
}