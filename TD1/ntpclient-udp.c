#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/udp.h>
#include <netdb.h>
#include <time.h>

#define NTP_TIMESTAMP_DELTA 2208988800u

# define SET_LI(packet, li)(uint8_t)(packet.li_vn_mode |= (li << 6))
# define SET_VN(packet, vn)(uint8_t)(packet.li_vn_mode |= (vn << 3))
# define SET_MODE(packet, mode)(uint8_t)(packet.li_vn_mode |= (mode << 0))

typedef struct{

    uint8_t li_vn_mode;      // Eight bits. li, vn, and mode.
                             // li.   Two bits.   Leap indicator.
                             // vn.   Three bits. Version number of the protocol.
                             // mode. Three bits. Client will pick mode 3 for client.

    uint8_t stratum;         // Eight bits. Stratum level of the local clock.
    uint8_t poll;            // Eight bits. Maximum interval between successive messages.
    uint8_t precision;       // Eight bits. Precision of the local clock.

    uint32_t rootDelay;      // 32 bits. Total round trip delay time.
    uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
    uint32_t refId;          // 32 bits. Reference clock identifier.

    uint32_t refTm_s;        // 32 bits. Reference time-stamp seconds.
    uint32_t refTm_f;        // 32 bits. Reference time-stamp fraction of a second.

    uint32_t origTm_s;       // 32 bits. Originate time-stamp seconds.
    uint32_t origTm_f;       // 32 bits. Originate time-stamp fraction of a second.

    uint32_t rxTm_s;         // 32 bits. Received time-stamp seconds.
    uint32_t rxTm_f;         // 32 bits. Received time-stamp fraction of a second.

    uint32_t txTm_s;         // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
    uint32_t txTm_f;         // 32 bits. Transmit time-stamp fraction of a second.

} ntp_packet;              // Total: 384 bits or 48 bytes.

void error(char *msg){
    perror(msg);
    _exit(EXIT_FAILURE);
}

int main(int argc, char **argv){
    if(argc < 2){
        error("Missing host name argument");
    }
    char *host_name = argv[1];

    ntp_packet packet;
    bzero(&packet, sizeof(ntp_packet));

    SET_LI(packet, 0);
    SET_VN(packet, 3);
    SET_MODE(packet, 3);
    
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0){
        error("Socket error");
    }

    struct hostent *server;
    server = gethostbyname(host_name);
    if(server == NULL){
        error("Cannot get host by name");
    }

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(struct sockaddr_in));

    serv_addr.sin_family = AF_INET;

    bcopy(server->h_addr, &serv_addr.sin_addr.s_addr, server->h_length);

    int n0_port = 123;
    serv_addr.sin_port = htons(n0_port);

    if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        error("Connect error");
    }

    if(write(sockfd, &packet, sizeof(ntp_packet)) < 0){
        error("Error writing to socket");
    }

    if(read(sockfd, &packet, sizeof(ntp_packet)) < 0){
        error("Error reading from socket");
    }

    long int timestamp = ntohl(packet.txTm_s) - NTP_TIMESTAMP_DELTA;

    printf("Time: %s\n", ctime(&timestamp));

    _exit(EXIT_SUCCESS);
}