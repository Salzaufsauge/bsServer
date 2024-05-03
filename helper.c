#include "helper.h"

int isNullOrEmpty(char *str) {
    return !(int)str || !*str;
}

int sendFormatedSocket(const int socket, char *cmd, char *key, char *val) {
    if (isNullOrEmpty(key) || isNullOrEmpty(val)) {
        perror("Error: Key/Value missing");
        sendToSocket(socket, "Error: Invalid input\n");
        return -1;
    }

    char str[BUFFER_SIZE + 3];
    sprintf(str, "%s:%s:%s\n", cmd, key, val);

    return sendToSocket(socket, str);
}


int sendToSocket(const int socket, const char *msg) {
    if (send(socket, msg, strlen(msg),0) < 0) {
        perror("Error: Failed writing to socket!");
        return -1;
    }
    return 0;
}

void error(const char *msg, const int exitcode) {
    perror(msg);
    exit(exitcode);
}
