#include "sub.h"

volatile sig_atomic_t stop;
struct sembuf up[2], down[2];
int semID, readcount;
int transactionOwner = 0;

void sig_handler(int signum) {
    stop = 1;
}

void setupSignals() {
    signal(SIGCHLD, SIG_IGN);
    struct sigaction sa;
    sa.sa_handler = sig_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if(sigaction(SIGTERM, &sa, NULL) < 0)
        error("Error on setupSignals: Failed setting sigaction",1);
}

int initSharedMemory() {
    const int shmid = shmget(IPC_PRIVATE,sizeof(KeyList),0644 | IPC_CREAT);
    if(shmid < 0)
        error("Error: Failed initializing shared memory",1);
    return shmid;
}

int initMsgQueue() {
    const int msgid = msgget(IPC_PRIVATE, 0644 | IPC_CREAT);
    if(msgid < 0)
        error("Error: Failed initializing message queue",2);
    return msgid;
}

int startServer() {
    const int sockfd = createSocket();
    bindSocket(sockfd);
    listenSocket(sockfd);
    return sockfd;
}

void createSemaphores() {
    semID = semget(IPC_PRIVATE, 2,IPC_CREAT | 0644);

    if (semID < 0)
        error("Error: Failed creating semaphore group!", 1);

    if (semctl(semID, 0,SETVAL, 1) < 0)
        error("Error: Failed setting initial value of semaphore 0", 2);

    if (semctl(semID, 1, SETVAL, 1) < 0)
        error("Error: Failed setting initial value of semaphore 1", 3);

    down[0] = (struct sembuf){0, -1,SEM_UNDO};
    down[1] = (struct sembuf){1, -1,SEM_UNDO};
    up[0] = (struct sembuf){0, 1,SEM_UNDO};
    up[1] = (struct sembuf){1, 1,SEM_UNDO};
    readcount = 0;
}

int createSocket() {
    //create socket
    const int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //set socket options and check if socket creation was successful
    if (sockfd < 0)
        error("Error: Failed opening socket!", 11);
    if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR | SO_REUSEPORT, &(int){1}, sizeof(int)) < 0)
        error("Error: Failed setting options", 12);

    return sockfd;
}

void bindSocket(const int serverSocket) {
    //set address settings
    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    //bind socket
    if (bind(serverSocket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("Error: Failed binding socket!", 21);
}

void listenSocket(const int serverSocket) {
    //listen
    if (listen(serverSocket, 10) < 0)
        error("Error: Failed listening on socket", 31);
}

void closeServer(const int serverSocket, const int shmid, const KeyList *keys) {
    if (semctl(semID, 0,IPC_RMID, 0) < 0)
        error("Error on closeServer: Failed deleting semaphore 0", 42);

    if (shmdt(keys) < 0)
        error("Error on closeServer: Failed detatching shared memory", 43);

    if (shmctl(shmid, IPC_RMID, 0) < 0)
        error("Error on closeServer: Failed deleting shared memory", 44);

    close(serverSocket);
}

void mainLoop(const int serverSocket, int msgid, KeyList *keys) {
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    while (!stop) {
        //accept client
        int cliSoc = accept(serverSocket, (struct sockaddr *) &cli_addr, &clilen);

        if (cliSoc < 0) {
            perror("Error: Failed accepting socket");
            continue;
        }
        int pid = fork();
        if (pid < 0) {
            perror("Error: Failed forking");
            close(cliSoc);
            continue;
        }
        if (pid == 0) {
            close(serverSocket);
            sendToSocket(cliSoc, "Write HELP to receive a list of possible commands!");
            handleClient(getpid(),cliSoc, msgid, keys);
            closeTransaction(keys);
            exit(0);
        } else
            close(cliSoc);
    }
}

//Client read and write
void handleClient(int pid, int clientSocket, int msgid, KeyList *keys) {
    char buffer[BUFFER_SIZE];
    int subPid = fork();
    if (subPid < 0) {
        error("Error in handleClient: Failed forking",1);
    } else if(subPid == 0) {
        handleMessage(pid, clientSocket, msgid);
        exit(0);
    } else {
        while (!stop) {
            //Blocks untill receiving data from client
            int n = recv(clientSocket, buffer,BUFFER_SIZE - 1, 0);

            if (n < 0) {
                perror("Error: read failed");
                close(clientSocket);
                break;
            } else if (n == 0) {
                printf("Connection closed\n");
                close(clientSocket);
                break;
            } else {
                buffer[n - 2] = '\0'; //Client sends \r\n replaces the \r with \0
                analyze(pid, msgid, clientSocket, keys, buffer);
            }
        }
    }
}

void handleMessage(int pid, int clientSocket, int msgid) {
    struct message msg;
    while(!stop) {
        bzero(&msg,sizeof(msg));
        int n = msgrcv(msgid,&msg,BUFFER_SIZE + 3,pid,0);
        if(n > 0)
            sendToSocket(clientSocket,msg.mtext);
    }
}

//Analyzes the input of the client
void analyze(int pid, int msgid, int clientSocket, KeyList *keys, char *buffer) {
    //seperate command and key from value
    char *cmd = strsep(&buffer, " ");
    char *key = strsep(&buffer, " ");
    Subscribers subs = {0};
    char msg[BUFFER_SIZE + 3];
    if (!strcmp(cmd, "QUIT")) {
        quit(clientSocket, keys);
    } else if (!strcmp(cmd, "PUT")) {
        if (checkTransaction(keys) && !transactionOwner) {
            sendToSocket(clientSocket, "Transaction in progress");
            return;
        }
        if (put(keys, &subs, key, buffer) < 0) {
            perror("Error: Invalid input");
            formatStr(msg, cmd, key,"invalid_input");
            sendToSocket(clientSocket,msg);
            return;
        }
        formatStr(msg, cmd, key, buffer);
        pub(msgid,&subs,msg);
        sendToSocket(clientSocket,msg);
    } else if (!strcmp(cmd, "GET")) {
        char res[192];
        if (get(keys, key, res) < 0) {
            perror("Error: Key not found");
            formatStr(msg,cmd,key,"key_nonexistent");
            sendToSocket(clientSocket,msg);
            return;
        }
        formatStr(msg, cmd, key, res);
        sendToSocket(clientSocket, msg);
    } else if (!strcmp(cmd, "DEL")) {
        if (checkTransaction(keys) && !transactionOwner) {
            sendToSocket(clientSocket, "Transaction in progress");
            return;
        }
        if (del(keys, &subs, key) < 0) {
            perror("Error: Key not found");
            formatStr(msg,cmd,key,"key_nonexistent");
            sendToSocket(clientSocket,msg);
            return;
        }
        formatStr(msg, cmd, key, "key_deleted");
        pub(msgid,&subs,msg);
        sendToSocket(clientSocket,msg);
    } else if (!strcmp(cmd, "HELP")) {
        sendToSocket(clientSocket, commandList);
    } else if (!strcmp(cmd, "BEG")) {
        beg(clientSocket, keys);
    } else if (!strcmp(cmd, "END")) {
        end(clientSocket, keys);
    } else if (!strcmp(cmd, "SUB")) {
        Key newKey;
        strcpy(newKey.keyName,key);
        if(sub(pid,clientSocket,keys,&newKey)) {
            perror("Error: Failed subscribing");
            formatStr(msg, cmd,key,"key_nonexistent");
            sendToSocket(clientSocket,msg);
            return;
        }
        formatStr(msg, cmd, key, newKey.keyVal);
        sendToSocket(clientSocket,msg);
    } else {
        perror("Error: Invalid command");
        char str[128];
        sprintf(str, "Error: Invalid command %s", cmd);
        sendToSocket(clientSocket,str);
    }
}

//Gracefully quits by stoping the client from reading and writing
void quit(const int clientSocket, KeyList* keys) {
    sendToSocket(clientSocket, "Exiting from server");
    shutdown(clientSocket,SHUT_RDWR);
}

int put(KeyList *keys, Subscribers *subs, char *key, char *val) {
    if (isNullOrEmpty(key) || isNullOrEmpty(val) || strpbrk(val,INVALID_CHARACTERS)) {
        perror("Error: invalid data!");
        return -1;
    }

    Key newKey;
    strcpy(newKey.keyName, key);
    strcpy(newKey.keyVal, val);
    bzero(&newKey.subs,sizeof(Subscribers));

    enterWrite();

    //If key already exists overwrites old key
    const int keyCapacity = keys->curSize;
    for (int i = 0; i < keyCapacity; i++) {
        if (!strcmp(key, keys->key[i].keyName)) {
            if (overwrite(keys, newKey, i) < 0) {
                exitWrite();
                return -1;
            }
            *subs = keys->key[i].subs;
            exitWrite();
            return 0;
        }
    }
    if (appendList(keys, newKey) < 0) {
        exitWrite();
        return -1;
    }

    exitWrite();
    return 0;
}

int get(const KeyList *keys, char *key, char *res) {
    if (isNullOrEmpty(key)) {
        perror("Error: invalid key");
        return -1;
    }

    enterRead();

    //look if key exists if yes copy string into res
    const int keyCapacity = keys->curSize;
    for (int i = 0; i < keyCapacity; i++) {
        if (!strcmp(key, keys->key[i].keyName)) {
            strcpy(res, keys->key[i].keyVal);
            exitRead();
            return 0;
        }
    }

    exitRead();

    perror("Error: Couldn't find key");
    return -1;
}

int del(KeyList *keys, Subscribers *subs, char *key) {
    if (isNullOrEmpty(key)) {
        perror("Error: invalid key");
        return -1;
    }

    enterWrite();

    //If key exists delete it
    const int keyCapacity = keys->curSize;
    for (int i = 0; i < keyCapacity; i++) {
        if (!strcmp(key, keys->key[i].keyName)) {
            *subs = keys->key[i].subs;
            if (deleteFromList(keys, i) < 0) {
                exitWrite();
                return -1;
            }
            exitWrite();
            return 0;
        }
    }

    exitWrite();

    perror("Error: Couldn't find key");
    return -1;
}

void beg(const int clientSocket, KeyList *keys) {
    enterWrite();
    if (setTransaction(keys) < 0) {
        exitWrite();
        sendToSocket(clientSocket, "Transaction already in progress");
        return;
    }
    exitWrite();
    transactionOwner = 1;
    sendToSocket(clientSocket,"Beginning transaction");
}

void end(const int clientSocket, KeyList *keys) {
    if(checkTransaction(keys) && transactionOwner != 1) {
        sendToSocket(clientSocket, "Not the transaction owner");
        return;
    }
    enterWrite();
    if (revokeTransaction(keys) < 0) {
        exitWrite();
        sendToSocket(clientSocket, "No transaction in progress");
        return;
    }
    exitWrite();
    transactionOwner = 0;
    sendToSocket(clientSocket,"Ending transaction");
}

int sub(int pid, int clientSocket, KeyList *keys, Key *key) {
    enterWrite();
    if(subscribe( pid, keys, key) < 0) {
        exitWrite();
        return -1;
    }
    exitWrite();
    return 0;
}

void pub(int msgid, Subscribers *subs, char *str) {
    struct message msg = {0};
    strcpy(msg.mtext,str);
    if(subs->curSize > 0) {
        int subCount = subs->curSize;
        for(int i = 0; i < subCount; ++i) {
            msg.mtype = subs->pid[i];
            msgsnd(msgid, &msg,strlen(str),0);
        }
    }
}

void formatStr(char *str, char *cmd, char *key, char *val) {
    if (isNullOrEmpty(key) || isNullOrEmpty(val)) {
        perror("Error: Key/Value missing");
    }
    sprintf(str, "%s:%s:%s", cmd, key, val);
}

int checkTransaction(KeyList* keys) {
    int transaction = 0;
    enterRead();
    transaction = keys->transactionInProgress;
    exitRead();
    return transaction;
}

void closeTransaction(KeyList *keys) {
    if(checkTransaction(keys) && transactionOwner) {
        enterWrite();
        revokeTransaction(keys);
        exitWrite();
    }
}

void enterRead() {
    if (semop(semID, &down[1], 1) < 0)
        perror("Error on enterRead: Failed blocking reader");
    readcount++;
    if (readcount == 1)
        if (semop(semID, &down[0], 1) < 0)
            perror("Error on enterRead: Failed blocking writer");
    if (semop(semID, &up[1], 1) < 0)
        perror("Error on enterRead: Failed unblocking reader");
}

void exitRead() {
    if (semop(semID, &down[1], 1) < 0)
        perror("Error on exitRead: Failed blocking reader");
    readcount--;
    if (readcount == 0)
        if (semop(semID, &up[0], 1) < 0)
            perror("Error on exitRead: Failed unblocking writer");
    if (semop(semID, &up[1], 1) < 0)
        perror("Error on exitRead: Failed unblocking reader");
}

void enterWrite() {
    if (semop(semID, &down[0], 1) < 0)
        perror("Error on enterWrite: Failed blocking writer");
}

void exitWrite() {
    if (semop(semID, &up[0], 1) < 0)
        perror("Error on exitWrite: Failed unblocking writer");
}
