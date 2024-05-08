#include <signal.h>
#include <unistd.h>
#include "sub.h"

int main(int argc, char *argv[]) {
    setupSignals();

    createSemaphores();

    const int shmid = initSharedMemory();
    KeyList *keys = shmat(shmid,NULL, 0);
    if (keys == (void *) -1)
        error("Error attatching shared memory", 1);
    //init value storage
    keys->curSize = 0;
    keys->transactionInProgress = 0;

    const int msgid = initMsgQueue();

    //startup server
    const int sockfd = startServer();
    mainLoop(sockfd, msgid, keys);
    closeServer(sockfd, shmid, keys);
    return 0;
}
