#include "sub.h"

volatile sig_atomic_t stop;
struct sembuf up[2], down[2];
int semID, readcount;

void sig_handler(int signum) {
    stop = 1;
}

int startServer() {
    int sockfd = createSocket();
    bindSocket(sockfd);
    listenSocket(sockfd);

    return sockfd;
}

void createSemaphores() {
    semID = semget(IPC_PRIVATE, 2,IPC_CREAT | 0644);

    if (semID < 0)
        error("Error: Failed creating semaphore group!", -1);

    if (semctl(semID, 0,SETVAL, 1) < 0)
        error("Error: Failed setting initial value of semaphore 0", -1);

    if (semctl(semID, 1, SETVAL, 1) < 0)
        error("Error: Failed setting initial value of semaphore 1", -1);

    down[0] = (struct sembuf) {0, -1 ,SEM_UNDO};
    down[1] = (struct sembuf) {1, -1 ,SEM_UNDO};
    up[0] = (struct sembuf) {0, 1 ,SEM_UNDO};
    up[1] = (struct sembuf) {1, 1 ,SEM_UNDO};
    readcount = 0;
}

int createSocket() {
    //create socket
    const int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //set socket options and check if socket creation was successful
    if (sockfd < 0)
        error("Error: Failed opening socket!", -1);
    if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR | SO_REUSEPORT, &(int){1}, sizeof(int)) < 0)
        error("Error: Failed setting options", -2);

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
        error("Error: Failed binding socket!", -3);
}

void listenSocket(const int serverSocket) {
    //listen
    if (listen(serverSocket, 10) < 0)
        error("Error: Failed listening on socket", -4);
}

void closeServer(const int serverSocket, const int shmid, const KeyList *keys) {
    if(semctl(semID, 0,IPC_RMID, 0) < 0)
        error("Error on closeServer: Failed deleting semaphore 0",-1);

    if(semctl(semID, 1,IPC_RMID, 0) < 0)
        error("Error on closeServer: Failed deleting semaphore 1",-1);

    if (shmdt(keys) < 0)
        error("Error on closeServer: Failed detatching shared memory", -1);

    if (shmctl(shmid, IPC_RMID, 0) < 0)
        error("Error on closeServer: Failed deleting shared memory", -1);

    close(serverSocket);
}

void mainLoop(const int serverSocket, KeyList *keys) {
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    while (!stop) {
        //accept client
        int cliSoc = accept(serverSocket, (struct sockaddr *) &cli_addr, &clilen);

        if (cliSoc < 0)
            perror("Error: Failed accepting socket!");
        int pid = fork();
        if (pid < 0)
            error("Error: Failed forking", -1);
        if (pid == 0) {
            close(serverSocket);
            sendToSocket(cliSoc, "Write HELP to receive a list of possible commands!\n");
            handleClient(cliSoc, keys);
            exit(0);
        } else
            close(cliSoc);
    }
}

//Client read and write
void handleClient(int clientSocket, KeyList *keys) {
    char buffer[BUFFER_SIZE];
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
            buffer[n - 2] = '\0';   //Client sends \r\n replaces the \r with \0
            analyze(clientSocket, keys, buffer);
        }
    }
}

//Analyzes the input of the client
void analyze(int serverSocket, KeyList *keys, char *buffer) {
    //seperate command and key from value
    char *cmd = strsep(&buffer, " ");
    char *key = strsep(&buffer, " ");
    if (!strcmp(cmd, "QUIT")) {
        quit(serverSocket);
    } else if (!strcmp(cmd, "PUT")) {
        if (put(keys, key, buffer) < 0) {
            perror("Error: Invalid input");
            sendFormatedSocket(serverSocket, cmd, key, "invalid_input");
            return;
        }
        sendFormatedSocket(serverSocket, cmd, key, buffer);
    } else if (!strcmp(cmd, "GET")) {
        char res[192];
        if (get(keys, key, res) < 0) {
            perror("Error: Key not found");
            sendFormatedSocket(serverSocket, cmd, key, "key_nonexistent");
            return;
        }
        sendFormatedSocket(serverSocket, cmd, key, res);
    } else if (!strcmp(cmd, "DEL")) {
        if (del(keys, key) < 0) {
            perror("Error: Key not found");
            sendFormatedSocket(serverSocket, cmd, key, "key_nonexistent");
            return;
        }
        sendFormatedSocket(serverSocket, cmd, key, "key_deleted");
    } else if (!strcmp(cmd, "HELP")) {
        sendToSocket(serverSocket, commandList);
    } else if (!strcmp(cmd, "BEG")) {

    } else if (!strcmp(cmd, "END")) {

    } else if (!strcmp(cmd, "PUB")) {

    } else if (!strcmp(cmd, "SUB")) {

    } else {
        perror("Error: Invalid command");
        char str[128];
        sprintf(str, "Error: Invalid command %s\n", cmd);
        write(serverSocket, str, strlen(str));
    }
}

//Gracefully quits by stoping the client from reading and writing
void quit(const int clientSocket) {
    constexpr char str[] = "Exiting from server\n";
    write(clientSocket, str, strlen(str));
    shutdown(clientSocket,SHUT_RDWR);
}

int put(KeyList *keys, char *key, char *val) {
    if (isNullOrEmpty(key) || isNullOrEmpty(val) || strpbrk(val,INVALID_CHARACTERS)) {
        perror("Error: invalid data!");
        return -1;
    }

    Key newKey;
    strcpy(newKey.keyName, key);
    strcpy(newKey.keyVal, val);

    enterWrite();

    //If key already exists overwrites old key
    const int keyCapacity = keys->curSize + 1;
    for (int i = 0; i < keyCapacity; i++) {
        if (!strcmp(key, keys->key[i].keyName)) {
            if (overwrite(keys, newKey, i) < 0) {
                exitWrite();
                return -1;
            }
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
    const int keyCapacity = keys->curSize + 1;
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

int del(KeyList *keys, char *key) {
    if (isNullOrEmpty(key)) {
        perror("Error: invalid key");
        return -1;
    }

    enterWrite();

    //If key exists delete it
    const int keyCapacity = keys->curSize + 1;
    for (int i = 0; i < keyCapacity; i++) {
        if (!strcmp(key, keys->key[i].keyName)) {
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
