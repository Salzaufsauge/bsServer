#include "keyValStore.h"

//resizes KeyList and appends 1 Key
int appendList(KeyList targetList[], const Key data) {
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

int overwrite(KeyList targetList[], const Key data, const int element) {
    if(element < 0 || element >= targetList->curSize) {
        perror("Error: invalid element");
        return -1;
    }
    targetList->key[element] = data;
    return 0;
}
//Deletes element n from list and shrinks list
int deleteFromList(KeyList targetList[], const int element) {
    if (element < 0 || element > targetList->curSize) {
        perror("Error: invalid element");
        return -1;
    }
    targetList->key[element] = targetList->key[targetList->curSize];
    targetList->curSize--;
    return 0;
}
