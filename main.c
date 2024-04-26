#include <stdio.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
// #include <signal.h>
#include "sub.h"
#include "helper.h"

#define PORT 5678
#define BUFFER_SIZE 256

// int terminateServer = 0;
//
// void signalTermination(int signal) {
//     printf("Terminating Server");
//     terminateServer = 1;
// }

int main(int argc, char *argv[]) {
    //startup server
    // signal(SIGINT,signalTermination);
    startServer();
    int sockfd;
    socklen_t clilen;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in serv_addr, cli_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("Failed opening socket!");
    if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        error("Failed option reuse addr");
    if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEPORT, &(int){1}, sizeof(int)) < 0)
        error("Failed option reuse port");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    //bind socket
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("Failed binding socket!");

    //listen
    listen(sockfd, 5);
    while(1) {
        clilen = sizeof(cli_addr);
        int cliSoc = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (cliSoc < 0)
            error("Failed accepting socket!");
        while(1) {
            bzero(buffer,BUFFER_SIZE);
            int n = read(cliSoc, buffer,BUFFER_SIZE - 1);

            if(n < 0) {
                error("Error: read failed");
                break;
            } else if(n == 0) {
                printf("Connection closed");
                close(cliSoc);

                break;
            } else {
                buffer[n-2] = '\0';

                //printf("received following message: %s", buffer);
                analyze(&cliSoc,buffer);
                //n = write(cliSoc, buffer, sizeof(buffer));
            }

        }
        // if (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        //     if (buffer[0] == 'quit') {
        //         printf("Quitting server.\n");
        //         close(sockfd);
        //         return 0;
        //     }
        // }
    }
    //printf("Quitting server.\n");
    close(sockfd);
    return 0;
}
