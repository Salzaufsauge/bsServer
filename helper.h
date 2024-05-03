#ifndef HELPER_H
#define HELPER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include "defines.h"

int isNullOrEmpty(char *str);

int sendToSocket(const int socket, const char *msg);

void error(const char *msg,int exitcode);

#endif //HELPER_H
