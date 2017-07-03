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

void read_config(config *conf, char *config_path)
{
    int dns_magic_needed = 0;
    FILE *fp;
    if ((fp = fopen(config_path, "r")))
    {
        deserialize(conf, fp);
    } else {
        printf("Address to connect to: ");
        char temp[20];
        fgets(temp, 20, stdin);
        temp[strlen(temp)-1] = '\0';
        if (validate_ip(temp))
        {
            strcpy(conf->addr, temp);
        } else {
            dns_magic_needed = 1;
        }
        printf("Port to connect to: ");
        char port_temp[20];
        fgets(port_temp, 20, stdin);
        conf->port = atoi(port_temp);
        printf("Username to use: ");
        fgets(conf->user, 20, stdin);
        conf->user[strlen(conf->user)-1] = '\0';
        printf("Real name (optional): ");
        fgets(conf->realname, 20, stdin);
        conf->realname[strlen(conf->realname)-1] = '\0';
        printf("Password to use (optional): ");
        fgets(conf->password, 20, stdin);
        conf->password[strlen(conf->password)-1] = '\0';
        if (dns_magic_needed)
        {
            struct addrinfo hints, *res;
            memset(&hints, 0, sizeof hints);
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            res = dns_magic(temp, conf->port, &hints);
            long ip = ((struct sockaddr_in *)res->ai_addr)->sin_addr.s_addr;
            sprintf(temp, "%ld.%ld.%ld.%ld", (ip & 0xFF), ((ip >> 8) & 0xFF), ((ip >> 16) & 0xFF), ((ip >> 24) & 0xFF));
            strcpy(conf->addr, temp);
        }
        serialize(conf, config_path);
    }
}

int serialize(config *conf, char *config_path)
{
    FILE *fp = fopen(config_path, "w");
    fwrite(conf, sizeof(*conf), 1, fp);
    fclose(fp);
    return 0;
}

int deserialize(config *conf, FILE *fp)
{
    fread(conf, sizeof(*conf), 1, fp);
    fclose(fp);
    return 0;
}

int setup_config(config *conf)
{
    memset(conf->addr, '\0', sizeof(conf->addr));
    memset(conf->user, '\0', sizeof(conf->user));
    memset(conf->password, '\0', sizeof(conf->password));
    memset(conf->realname, '\0', sizeof(conf->realname));
    return 0;
}