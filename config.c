#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include "config.h"

void read_config(config *conf)
{
    FILE *fp;
    if (fp = fopen("./config", "r"))
    {
        deserialize(conf, fp);
    } else {
        printf("Address to connect to: ");
        fgets(conf->addr, 20, stdin);
        conf->addr[strlen(conf->addr)-1] = '\0';
        printf("Port to connect to: ");
        char temp[20];
        fgets(temp, 20, stdin);
        conf->port = atoi(temp);
        printf("Username to use: ");
        fgets(conf->user, 20, stdin);
        conf->user[strlen(conf->user)-1] = '\0';
        printf("Real name (optional): ");
        fgets(conf->realname, 20, stdin);
        conf->realname[strlen(conf->realname)-1] = '\0';
        printf("Password to use (optional): ");
        fgets(conf->password, 20, stdin);
        conf->password[strlen(conf->password)-1] = '\0';
        serialize(conf);
    }
}

int serialize(config *conf)
{
    FILE *fp = fopen("./config", "w");
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