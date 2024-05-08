#ifndef KEYVALSTORE_H
#define KEYVALSTORE_H

#include <stddef.h>
#include <stdlib.h>
#include "defines.h"
#include "helper.h"

//might replace with hashtable + list
typedef struct {
    int pid[MAX_CLIENT_COUNT];
    int curSize;
} Subscribers;

typedef struct {
    char keyName[KEY_NAME_LENGTH];
    char keyVal[KEY_VALUE_LENGTH];
    Subscribers subs;
} Key;

typedef struct {
    unsigned int curSize;
    Key key[MAX_CAPACITY];
    int transactionInProgress;
} KeyList;

int appendList(KeyList *targetList, const Key data);

int overwrite(KeyList *targetList, const Key data, int element);

int deleteFromList(KeyList *targetList, int element);

int setTransaction(KeyList *targetList);

int revokeTransaction(KeyList *targetList);

int subscribe(int pid,KeyList *targetList,Key* key);

inline void appendSubscriber(const int pid, Subscribers *target) {
    target->pid[target->curSize++] = pid;
}

#endif //KEYVALSTORE_H
