#include "helper.h"

int sendToSocket(int *socket, char *cmd, char *key, char *val) {
    const int del = strcmp(cmd, "DEL");
    if (!key) {
        error("Key missing");
        return -1;
    }
    char str[256]; // Allocate memory for str
    if (!del) {
        sprintf(str, "%s:%s\n", cmd, key); // Format string correctly
    } else {
        if (!val) {
            error("Value missing");
            return -1;
        }
        sprintf(str, "%s:%s:%s\n", cmd, key, val); // Format string correctly
    }
    if (write(*socket, str, strlen(str)) < 0) {
        error("Failed writing to socket!");
        return -1;
    }
    return 0;
}

void error(char *msg) {
    perror(msg);
}
