#include <signal.h>
#include <unistd.h>
#include "sub.h"

int main(int argc, char *argv[]) {
    //startup server
    //signal(SIGTERM, sig_handler);
    signal(SIGCHLD, SIG_IGN);
    struct sigaction sa;
    sa.sa_handler = sig_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGTERM, &sa, NULL);
    int shmid= shmget(IPC_PRIVATE,sizeof(KeyList),0644|IPC_CREAT);
    if(shmid < 0)
        error("Error: Failed initializing shared memory",-1);
    KeyList *keys = shmat(shmid,NULL,0);
    //init value storage
    initList(keys);
    int sockfd = startServer(&shmid,keys);
    mainLoop(&sockfd, keys);
    closeServer(&sockfd, &shmid, keys);
    return 0;
}
