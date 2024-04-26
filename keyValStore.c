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
    targetList->key[targetList->curSize] = data;
}

void deleteFromList(KeyList targetList[], const int element) {
    for (int i = element; i < targetList->capacity; i++)
        targetList->key[i] = targetList->key[i + 1];
    free(&(targetList->key[targetList->curSize]));
    targetList->capacity--;
    targetList->curSize--;
}
