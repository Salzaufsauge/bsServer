#ifndef SUB_H
#define SUB_H

#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <sys/sem.h>
#include "defines.h"
#include "helper.h"
#include "keyValStore.h"

void sig_handler(int signum);

int startServer();

void createSemaphores();

int createSocket();

void bindSocket(int serverSocket);

void listenSocket(int serverSocket);

void closeServer(const int serverSocket, const int shmid, const KeyList *keys);

void mainLoop(int serverSocket, KeyList* keys);

void handleClient(int clientSocket, KeyList* keys);

void analyze(int serverSocket,KeyList* keys , char *buffer);

void quit(int clientSocket);

int put(KeyList *keys, char *key, char *val);

int get(const KeyList *keys, char *key, char *res);

int del(KeyList *keys,char *key);

void enterRead();
void exitRead();

void enterWrite();
void exitWrite();

#endif //SUB_H
