#include "sub.h"

KeyList keys;

int startServer() {
    initList(&keys);
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
        error("Error: Failed opening socket!", -1);
    if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR | SO_REUSEPORT, &(int){1}, sizeof(int)) < 0)
        error("Error: Failed setting options", -2);

    struct sockaddr_in serv_addr = {0};
    //bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    //bind socket
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("Error: Failed binding socket!", -3);

    //listen
    if(listen(sockfd, 10) < 0)
        error("Error: Failed listening on socket",-4);

    return sockfd;
}

void mainLoop(const int *serverSocket) {
    struct sockaddr_in cli_addr;
    socklen_t clilen;
    while (1) {
        clilen = sizeof(cli_addr);
        int cliSoc = accept(*serverSocket, (struct sockaddr *) &cli_addr, &clilen);
        if (cliSoc < 0)
            perror("Failed accepting socket!");
        handleClient(&cliSoc);
    }
}

void handleClient(int *clientSocket) {
    char buffer[BUFFER_SIZE];
    while (1) {
        bzero(buffer,BUFFER_SIZE);
        int n = read(*clientSocket, buffer,BUFFER_SIZE - 1);

        if (n < 0) {
            perror("Error: read failed");
            close(*clientSocket);
            break;
        } else if (n == 0) {
            printf("Connection closed");
            close(*clientSocket);
            break;
        } else {
            buffer[n - 2] = '\0';
            analyze(clientSocket, buffer);
        }
    }
}

void analyze(int *socket, char *buffer) {
    char *cmd = strsep(&buffer, " ");
    char *key = strsep(&buffer, " ");
    if (!strcmp(cmd, "QUIT")) {
        quit(socket);
    } else if (!strcmp(cmd, "PUT")) {
        if (put(key, buffer) < 0) {
            perror("Error: Invalid input");
            sendFormatedSocket(socket, cmd, key, "invalid_input");
            return;
        }
        sendFormatedSocket(socket, cmd, key, buffer);
    } else if (!strcmp(cmd, "GET")) {
        char res[192];
        if (get(key, res) < 0) {
            perror("Error: Key not found");
            sendFormatedSocket(socket, cmd, key, "key_nonexistent");
            return;
        }
        sendFormatedSocket(socket, cmd, key, res);
    } else if (!strcmp(cmd, "DEL")) {
        if (del(key) < 0) {
            perror("Error: Key not found");
            sendFormatedSocket(socket, cmd, key, "key_nonexistent");
            return;
        }
        sendFormatedSocket(socket, cmd, key, "key_deleted");
    } else {
        perror("Error: Invalid command");
        char str[128];
        sprintf(str, "Error: Invalid command %s\n", cmd);
        write(*socket, str, strlen(str));
    }
}

void quit(const int *socket) {
    constexpr char str[] = "Exiting from server\n";
    write(*socket, str, strlen(str));
    shutdown(*socket,SHUT_RDWR);
}

int put(char *key, char *val) {
    if (isNullOrEmpty(key) || isNullOrEmpty(val) || strpbrk(val,INVALID_CHARACTERS)) {
        perror("Error: invalid data!");
        return -1;
    }
    Key newKey;
    strcpy(newKey.keyName, key);
    strcpy(newKey.keyVal, val);
    const int keyCapacity = keys.capacity;
    for (int i = 0; i < keyCapacity; i++) {
        if (!strcmp(key, keys.key[i].keyName)) {
            if(overwrite(&keys, newKey, i) < 0)
                return -1;
            return 0;
        }
    }
    if(appendList(&keys, newKey) < 0)
        return -1;
    return 0;
}

int get(char *key, char *res) {
    if(isNullOrEmpty(key)) {
        perror("Error: invalid key");
        return -1;
    }
    const int keyCapacity = keys.capacity;
    for (int i = 0; i < keyCapacity; i++) {
        if (!strcmp(key, keys.key[i].keyName)) {
            strcpy(res, keys.key[i].keyVal);
            return 0;
        }
    }
    perror("Error: Couldn't find key");
    return -1;
}

int del(char *key) {
    if(isNullOrEmpty(key)) {
        perror("Error: invalid key");
        return -1;
    }
    const int keyCapacity = keys.capacity;
    for (int i = 0; i < keyCapacity; i++) {
        if (!strcmp(key, keys.key[i].keyName)) {
            if(deleteFromList(&keys, i) < 0)
                return -1;
            return 0;
        }
    }
    perror("Error: Couldn't find key");
    return -1;
}
