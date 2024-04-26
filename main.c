#include <stdio.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include "sub.h"
#include "helper.h"

#define PORT 5678
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    //startup server
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
    clilen = sizeof(cli_addr);
    int newSoc = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newSoc < 0)
        error("Failed accepting socket!");
    bzero(buffer,BUFFER_SIZE);
    int n = read(newSoc, buffer,BUFFER_SIZE - 1);
    printf("received following message: %s\n", buffer);
    n = write(newSoc, "Hello,world!\n", 13);
    close(newSoc);
    close(sockfd);
    return 0;
}
