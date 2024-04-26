//
// Created by leon on 4/25/24.
//

#ifndef KEYVALSTORE_H
#define KEYVALSTORE_H

#include <stddef.h>
#include <stdlib.h>
#include "helper.h"

typedef struct {
    char keyName[32];
    char keyVal[128];
} Key;

typedef struct {
    unsigned int curSize;
    unsigned int capacity;
    Key *key;
} KeyList;

void initList(KeyList targetList[]);

void growList(KeyList targetList[], const Key data);

void deleteFromList(KeyList targetList[], int element);

#endif //KEYVALSTORE_H
