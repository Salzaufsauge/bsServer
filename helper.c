#include "helper.h"

int isNullOrEmpty(char *str) {
    return !(int)str || !*str;
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
