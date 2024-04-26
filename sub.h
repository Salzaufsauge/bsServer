#ifndef SUB_H
#define SUB_H

#include <string.h>
#include <unistd.h>
#include "helper.h"
#include "keyValStore.h"

void startServer();

void analyze(int *socket, char *buffer);

void quit(int *socket);

int put(char *key, char *val);

int get(char *key, char *res);

int del(char *key);



#endif //SUB_H
