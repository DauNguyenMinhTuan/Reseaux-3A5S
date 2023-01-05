#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define SERVER_IP "127.0.0.1"
#define PORT 1234
#define BUFFER_SIZE 1025

void stop(char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

void echo(int sockfd){
    printf("Go ahead and send echo...\n");
    char buffer[BUFFER_SIZE];
    int n;
    for(int i = 1; i <= 1000; i++){
        bzero(buffer, BUFFER_SIZE);
        if((n = recv(sockfd, buffer, BUFFER_SIZE, 0)) < 0){
            stop("Receive Error\n");
        }
        buffer[n] = '\0';
        printf("Message from Client: %s\n", buffer);
        if(send(sockfd, buffer, n, MSG_CONFIRM) < 0){
            stop("Send Error\n");
        }
    }
}

int main(){
    int sockfd;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        stop("Socket Creation Failed\n");
    }

    struct sockaddr_in servaddr, cliaddr;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    servaddr.sin_port = htons(PORT);

    if(bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
        stop("Socket Binding Failed\n");
    }

    if(listen(sockfd, 5) < 0){
        stop("Listen Error\n");
    }
    else{
        printf("Server listening...\n");
    }
    int len, connfd;
    len = sizeof(cliaddr);

    connfd = accept(sockfd, (struct sockaddr *)&cliaddr, &len);
    if(connfd < 0){
        stop("Server Acceptance Failed\n");
    }
    else{
        printf("Server accept the client\n");
    }

    echo(connfd);

    close(sockfd);

    return 0;
}