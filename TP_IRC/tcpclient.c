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
#define MON_MESSAGE "ECHO"

void stop(char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

void echo(int sockfd){
    char buffer[BUFFER_SIZE];
    int n;
    for(int i = 1; i <= 1000; i++){
        bzero(buffer, BUFFER_SIZE);
        if(send(sockfd, MON_MESSAGE, sizeof(MON_MESSAGE), MSG_CONFIRM) < 0){
            stop("Send Error\n");
        }
        else{
            // printf("ECHO sent\n");
        }
        if(recv(sockfd, buffer, BUFFER_SIZE, 0) < 0){
            stop("Receive Error\n");
        }
        printf("Message from Server: %s\n", buffer);
    }
}

int main(){
    int sockfd;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        stop("Socket Creation Failed\n");
    }

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    servaddr.sin_port = htons(PORT);

    if(connect(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
        stop("Connection Error\n");
    }
    else{
        printf("Ready to echo to infinity...\n");
    }
    
    echo(sockfd);

    close(sockfd);

    return 0;
}