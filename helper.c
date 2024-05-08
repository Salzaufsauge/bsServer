#include "helper.h"

int isNullOrEmpty(const char *str) {
    return !(int)str || !*str;
}

int sendToSocket(const int socket, const char *msg) {
    const size_t strLen = strlen(msg) + 2;
    char str[strLen];
    sprintf(str,"%s\n",msg);
    if (send(socket, str, strLen,0) < 0) {
        perror("Error: Failed writing to socket!");
        return -1;
    }
    return 0;
}

void error(const char *msg, const int exitcode) {
    perror(msg);
    exit(exitcode);
}
