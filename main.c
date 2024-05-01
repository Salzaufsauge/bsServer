#include <signal.h>
#include <unistd.h>
#include "sub.h"

int main(int argc, char *argv[]) {
    //startup server
    signal(SIGCHLD, SIG_IGN);
    int sockfd = startServer();
    mainLoop(&sockfd);
    close(sockfd);
    return 0;
}
