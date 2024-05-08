#ifndef SUB_H
#define SUB_H

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/msg.h>

#include "defines.h"
#include "helper.h"
#include "keyValStore.h"

struct message {
    long mtype;
    char mtext[BUFFER_SIZE];
};

void sig_handler(int signum);

void setupSignals();

int initSharedMemory();

int initMsgQueue();

int startServer();

void createSemaphores();

int createSocket();

void bindSocket(int serverSocket);

void listenSocket(int serverSocket);

void closeServer(const int serverSocket, const int shmid, const KeyList *keys);

void mainLoop(int serverSocket, int msgid, KeyList* keys);

void handleClient(int pid, int clientSocket, int msgid, KeyList* keys);

void handleMessage(int pid, int clientSocket, int msgid);

void analyze(int pid, int msgid, int clientSocket, KeyList* keys, char *buffer);

void quit(int clientSocket, KeyList* keys);

int put(KeyList *keys, Subscribers *subs, char *key, char *val);

int get(const KeyList *keys, char *key, char *res);

int del(KeyList *keys, Subscribers *subs, char *key);

void beg(int clientSocket, KeyList* keys);

void end(int clientSocket, KeyList* keys);

int sub(int pid, int clientSocket, KeyList* keys, Key *key);

void pub(int msgid, Subscribers *subs, char *str);

//int sendFormatedSocket(int socket, char *cmd, char *key, char *val);

void formatStr(char *msg,char *cmd, char *key, char *val);

int checkTransaction(KeyList* keys);

void closeTransaction(KeyList* keys);

void enterRead();
void exitRead();

void enterWrite();
void exitWrite();

#endif //SUB_H
