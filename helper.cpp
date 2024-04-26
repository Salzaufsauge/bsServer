#include "helper.h"

int send(int *socket, char *cmd, char *key, char *val) {
    int del = strcmp(cmd, "DEL");
    char *str = del ? "%s:%s\n" : "%s:%s:%s\n";
    del ? sprintf(str, cmd, key) : sprintf(str, cmd, key, val);
    if (write(*socket, str, strlen(str)) < 0) {
        error("Failed writing to socket!");
        return -1;
    }
    return 0;
}

void error(char *msg) {
    perror(msg);
}
