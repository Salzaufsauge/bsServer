#ifndef SUB_H
#define SUB_H

#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include "defines.h"
#include "helper.h"
#include "keyValStore.h"

int startServer();

void mainLoop(const int *serverSocket);

void handleClient(int *clientSocket);

void analyze(int *socket, char *buffer);

void quit(const int *socket);

int put(char *key, char *val);

int get(char *key, char *res);

int del(char *key);


#endif //SUB_H
