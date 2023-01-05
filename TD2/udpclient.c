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
#define MON_MESSAGE "PING"

void stop(char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(){
    int sockfd;
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        stop("Socket Creation Failed\n");
    }
    
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    servaddr.sin_port = htons(1234);

    int n, len;
    char buffer[BUFFER_SIZE];

    printf("Ready to spam...\n");
    while(1){
        if((n = sendto(sockfd, MON_MESSAGE, strlen(MON_MESSAGE), MSG_CONFIRM, 
            (const struct sockaddr *)&servaddr, sizeof(servaddr))) < 0){
            stop("Send Error\n");
        }
        if((n = recvfrom(sockfd, buffer, BUFFER_SIZE, MSG_WAITALL, 
            (struct sockaddr *)&servaddr, &len)) < 0){
            stop("Receive Error\n");
        }
        printf("Message from Server: %s\n\n", buffer);
    }

    return 0;
}