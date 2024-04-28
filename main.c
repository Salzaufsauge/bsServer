#include <unistd.h>
#include "sub.h"

int main(int argc, char *argv[]) {
    //startup server
    int sockfd = startServer();
    mainLoop(&sockfd);
    close(sockfd);
    return 0;
}
