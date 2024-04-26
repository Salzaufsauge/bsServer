//
// Created by leon on 4/25/24.
//

#ifndef HELPER_H
#define HELPER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int send(int *socket, char *cmd, char *key, char *val = "");

void error(char *msg);

#endif //HELPER_H
