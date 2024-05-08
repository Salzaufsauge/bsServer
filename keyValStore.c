#include "keyValStore.h"

//resizes KeyList and appends 1 Key
int appendList(KeyList *targetList, Key data) {
    if(isNullOrEmpty(data.keyName)
        || isNullOrEmpty(data.keyVal)
        || strlen(data.keyName) > KEY_NAME_LENGTH - 1
        || strlen(data.keyVal) > KEY_VALUE_LENGTH - 1) {
        perror("Error: invalid data");
        return -1;
    }
    if((targetList->curSize + 1) > MAX_CAPACITY ) {
        perror("Error: Max capacity reached");
        return -2;
    }
    targetList->key[targetList->curSize++] = data;
    return 0;
}

int overwrite(KeyList *targetList, const Key data, const int element) {
    if(element < 0 || element >= targetList->curSize) {
        perror("Error: invalid element");
        return -1;
    }
    strcpy(targetList->key[element].keyName, data.keyName);
    strcpy(targetList->key[element].keyVal, data.keyVal);
    return 0;
}
//Deletes element n from list and shrinks list
int deleteFromList(KeyList *targetList, const int element) {
    if (element < 0 || element > targetList->curSize) {
        perror("Error: invalid element");
        return -1;
    }
    targetList->key[element] = targetList->key[--targetList->curSize];
    //targetList->curSize--;
    return 0;
}

int setTransaction(KeyList *targetList) {
    if(targetList->transactionInProgress != 0) {
        perror("Error in setTransaction: TransactionOwner already exists/ invalid socket");
        return -1;
    }
    targetList->transactionInProgress = 1;
    return 0;
}

int revokeTransaction(KeyList *targetList) {
    if(targetList->transactionInProgress == 0) {
        perror("Error in revokeTransaction: No transaction open");
        return -1;
    }
    targetList->transactionInProgress = 0;
    return 0;
}

int subscribe(int pid, KeyList *targetList, Key* key) {
    const int keyCapacity = targetList->curSize;
    for (int i = 0; i < keyCapacity; i++) {
        if (!strcmp(key->keyName, targetList->key[i].keyName)) {
            strcpy(key->keyVal, targetList->key[i].keyVal);
            appendSubscriber(pid, &targetList->key[i].subs);
            return 0;
        }
    }
    perror("Error in subscribe: Key not found");
    return -1;
}

void appendSubscriber(const int pid, Subscribers *target);
