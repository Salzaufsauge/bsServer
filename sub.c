#include "sub.h"

KeyList keys;

void startServer() {
    initList(&keys);
}

void analyze(int *socket, char *buffer) {
    char *cmd = strsep(&buffer, " ");
    if (!strcmp(cmd, "QUIT")) {
        quit(socket);
        return;
    }
    char *key = strsep(&buffer, " ");
    if (!strcmp(cmd, "PUT")) {
        put(key, buffer);
        sendToSocket(socket, cmd, key, buffer);
    } else if (!strcmp(cmd, "GET")) {
        char res[128];
        if(get(key, res) < 0) {
            error("Key not found");
            return;
        }
        sendToSocket(socket, cmd, key, res);
    } else if (!strcmp(cmd, "DEL")) {
        if(del(key) < 0) {
            error("Key not found");
            return;
        }
        sendToSocket(socket, cmd, key,buffer);
    } else {
        error("Error: Invalid command");
        // char *str = strcat("Error: invalid command: ", cmd);
        // write(*socket, str, strlen(str));
    }
}

void quit(int *socket) {
    char str[] = "Exiting from server";
    write(*socket,str,strlen(str));
    close(*socket);
}

int put(char *key, char *val) {
    if (!key[0]) {
        error("Error: No key!");
        return -1;
    }
    if (!val[0]) {
        error("Error: No value");
        return -1;
    }
    Key newKey;
    strcpy(newKey.keyName, key);
    strcpy(newKey.keyVal, val);
    growList(&keys, newKey);
    return 0;
}

int get(char *key, char *res) {
    int keyCapacity = keys.capacity;
    for (int i = 0; i < keyCapacity; i++) {
        if (!strcmp(key, keys.key[i].keyName)) {
            strcpy(res,keys.key[i].keyVal);
            return 0;
        }
    }
    error("Error: Couldn't find key");
    return -1;
}

int del(char *key) {
    int keyCapacity = keys.capacity;
    for (int i = 0; i < keyCapacity; i++) {
        if (!strcmp(key,keys.key[i].keyName)) {
            deleteFromList(&keys, i);
            return 0;
        }
    }
    error("Error: Couldn't find key");
    return -1;
}
