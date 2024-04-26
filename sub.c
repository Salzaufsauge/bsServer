#include "sub.h"

KeyList keys;

void startServer() {
    initList(&keys);
}

void analyze(int *socket, char *buffer) {
    char *cmd = strsep(&buffer, " ");
    if (!strcmp(cmd, "QUIT"))
        quit(socket);
    char *key = strsep(&buffer, " ");
    if (!strcmp(cmd, "PUT")) {
        put(key, buffer);
        send(socket, cmd, key, buffer);
    } else if (!strcmp(cmd, "GET")) {
        char res[];
        get(key, res);
        send(socket, cmd, key, res);
    } else if (!strcmp(cmd, "DEL")) {
        del(key);
        send(socket, cmd, key);
    } else {
        error("Error: Invalid command");
        char *str = strcat("Error: invalid command: ", buffer);
        write(*socket, str, strlen(str));
    }
}

int put(char *key, char *val) {
    if (!key[0]) {
        error("Error: No key!");
        return -1;
    }
    if (!val[0]) {
        error("Error: No value");
        return -2;
    }
    Key newKey;
    strcpy(newKey.keyName, key);
    strcpy(newKey.keyVal, val);
    growList(&keys, newKey);
    return 1;
}

int get(char *key, char *res) {
    for (int i = 0; keys.capacity; i++) {
        if (key == keys.key[i].keyName) {
            *res = keys.key[i].keyVal;
            return 1;
        }
    }
    error("Error: Couldn't find key");
    return -1;
}

int del(char *key) {
    for (int i = 0; keys.capacity; i++) {
        if (key == keys.key[i].keyName) {
            deleteFromList(&keys, i);
        }
    }
    error("Error: Couldn't find key");
    return -1;
}
