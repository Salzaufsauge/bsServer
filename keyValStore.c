#include "keyValStore.h"

void initList(KeyList targetList[]) {
    targetList->key = (Key *) malloc(1 * sizeof(Key));
    if (targetList->key == NULL)
        error("Failed initializing KeyList");
    targetList->curSize = 0;
    targetList->capacity = 1;
}

void growList(KeyList targetList[], const Key data) {
    targetList->capacity++;
    targetList->key = (Key *) realloc(targetList->key, targetList->capacity * sizeof(Key));
    if (targetList->key == NULL)
        error("Failed resizing KeyList");
    targetList->key[targetList->curSize++] = data;
}

void deleteFromList(KeyList targetList[], const int element) {
    if (element < 0 || element >= targetList->curSize) {
        error("invalid element");
        return;
    }
    Key *temp = (Key *)malloc((targetList->capacity-1) * sizeof(Key));
    if(element)
        memcpy(temp,targetList->key,element * sizeof(Key));
    if(element != (targetList->curSize - 1))
        memcpy(temp+element,targetList->key + element + 1, (targetList->curSize - element - 1) * sizeof(Key));
    for(int i = 0; i < targetList->curSize;i++)
        printf("%s\n",temp[i].keyName);
    free(targetList->key);
    targetList->key = temp;

    // int listSize = targetList->curSize;
    // free(&targetList->key[listSize]);
    // for (int i = element; i < listSize - 1; i++)
    //     targetList->key[i] = targetList->key[i + 1];
    targetList->capacity--;
    targetList->curSize--;
}
