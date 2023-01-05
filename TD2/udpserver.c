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
#define MON_MESSAGE "PONG"

void stop(char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(){
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        stop("Socket Creation Failed\n");
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    servaddr.sin_port = htons(PORT);

    if(bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
        stop("Socket Binding Failed\n");
    }

    int len = sizeof(cliaddr);
    int n;
    char buffer[BUFFER_SIZE];

    printf("Ready to receive spam...\n");
    while(1){
        if((n = recvfrom(sockfd, buffer, BUFFER_SIZE, MSG_WAITALL,
                (struct sockaddr *)&cliaddr, &len)) < 0){
            stop("Receive Error\n");
        }
        buffer[n] = '\0';
        printf("Message from Client: %s\n", buffer);
        printf("Client's IP: %s\n", inet_ntoa(cliaddr.sin_addr));
        printf("Client's Port: %d\n", ntohs(cliaddr.sin_port));
        sleep(1);
        if((n = sendto(sockfd, MON_MESSAGE, strlen(MON_MESSAGE), MSG_CONFIRM,
                (struct sockaddr *)&cliaddr, len)) < 0){
            stop("Send Error\n");
        }
        printf("\n");
    }

    return 0;
}