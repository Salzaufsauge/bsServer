#ifndef KEYVALSTORE_H
#define KEYVALSTORE_H

#include <stddef.h>
#include <stdlib.h>
#include "defines.h"
#include "helper.h"

//might replace with hashtable + list

typedef struct {
    char keyName[KEY_NAME_LENGTH];
    char keyVal[KEY_VALUE_LENGTH];
} Key;

typedef struct {
    unsigned int curSize;
    //unsigned int capacity;
    Key key[MAX_CAPACITY];
} KeyList;

void initList(KeyList targetList[]);

int appendList(KeyList targetList[], const Key data);

int overwrite(KeyList targetList[], const Key data, int element);

int deleteFromList(KeyList targetList[], int element);

#endif //KEYVALSTORE_H
