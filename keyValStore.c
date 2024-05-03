#include "keyValStore.h"

void initList(KeyList targetList[]) {

    // targetList->key = (Key *) malloc(1 * sizeof(Key));
    // if (targetList->key == NULL)
    //     perror("Error: Failed initializing KeyList");
    targetList->curSize = 0;
    //targetList->capacity = 1;
}
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
    //targetList->capacity++;
    //targetList->key = (Key *) realloc(targetList->key, targetList->capacity * sizeof(Key));
    // if (targetList->key == NULL)
    //     perror("Error: Failed resizing KeyList");
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
    // const auto temp = (Key *) malloc((targetList->curSize) * sizeof(Key));
    // //if element > 0 copies all values before element into temp
    // if (element)
    //     memcpy(temp, targetList->key, element * sizeof(Key));
    // //copies all values after element if element isn't last
    // if (element != (targetList->curSize - 1))
    //     memcpy(temp + element,
    //            targetList->key + element + 1,
    //            (targetList->curSize - element - 1) * sizeof(Key));
    // //frees old memory
    // //free(targetList->key);
    // memcpy(targetList->key,temp, --targetList->curSize * sizeof(Key));
    // //targetList->capacity--;
    // free(temp);
    Key temp = targetList->key[element];
    targetList->key[element] = targetList->key[targetList->curSize];
    targetList->key[targetList->curSize] = temp;
    targetList->curSize--;
    return 0;
}
