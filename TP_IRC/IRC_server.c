#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define PORT 8000
#define IP_SERVER "192.168.91.17"
#define BUFSIZE 1024
#define max(a, b) ((a > b) ? a : b)
#define CLIENT_MAX 10
#define NTP_TIMESTAMP_DELTA 2208988800u
#define SET_LI(packet, li) (uint8_t)(packet.li_vn_mode |= (li << 6))
#define SET_VN(packet, vn) (uint8_t)(packet.li_vn_mode |= (vn << 3))
#define SET_MODE(packet, mode) (uint8_t)(packet.li_vn_mode |= (mode << 0))

int listenfd;
int socket_client[CLIENT_MAX];
char *pseudo_client[CLIENT_MAX];
clock_t timeMark;

const char *demandMsg = "Please choose a unique nickname: ";
const char *nicknameErrorMsg = "Sorry! This nickname is already been used! Please choose a different one!\n";
const char *welcomeMessage = "Welcome to notDiscord!\n";

typedef struct {
    uint8_t li_vn_mode;  // Eight bits. li, vn, and mode.
                         // li.   Two bits.   Leap indicator.
                         // vn.   Three bits. Version number of the protocol.
                         // mode. Three bits. Client will pick mode 3 for client.

    uint8_t stratum;    // Eight bits. Stratum level of the local clock.
    uint8_t poll;       // Eight bits. Maximum interval between successive messages.
    uint8_t precision;  // Eight bits. Precision of the local clock.

    uint32_t rootDelay;       // 32 bits. Total round trip delay time.
    uint32_t rootDispersion;  // 32 bits. Max error aloud from primary clock source.
    uint32_t refId;           // 32 bits. Reference clock identifier.

    uint32_t refTm_s;  // 32 bits. Reference time-stamp seconds.
    uint32_t refTm_f;  // 32 bits. Reference time-stamp fraction of a second.

    uint32_t origTm_s;  // 32 bits. Originate time-stamp seconds.
    uint32_t origTm_f;  // 32 bits. Originate time-stamp fraction of a second.

    uint32_t rxTm_s;  // 32 bits. Received time-stamp seconds.
    uint32_t rxTm_f;  // 32 bits. Received time-stamp fraction of a second.

    uint32_t txTm_s;  // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
    uint32_t txTm_f;  // 32 bits. Transmit time-stamp fraction of a second.

} ntp_packet;  // Total: 384 bits or 48 bytes.

void error(char *msg) {
    perror(msg);
    _exit(EXIT_FAILURE);
}

void stop(char *errorMsg) {
    perror(errorMsg);
    close(listenfd);
    exit(EXIT_FAILURE);
}

char *getTimestring(int NTPsockfd, ntp_packet *packetNTPin, ntp_packet *packetNTPout) {
    if (write(NTPsockfd, packetNTPout, sizeof(ntp_packet)) < 0) {
        error("Error writing to NTPsocket");
    }
    // printf("Waiting to get time...\n");
    if (read(NTPsockfd, packetNTPin, sizeof(ntp_packet)) < 0) {
        printf("Error reading from NTPsocket\n");
    }
    long int timestamp = ntohl(packetNTPin->txTm_s) - NTP_TIMESTAMP_DELTA;
    // time_t t = time(NULL);
    // struct tm *tm = localtime(&t);
    char *timestring;
    timestring = ctime(&timestamp);
    int pos = 0;
    while (timestring[pos] != '\n') {
        pos++;
    }
    timestring[pos] = '\0';
    return timestring;
}

int main() {
    char *ntp_host_name = "fr.pool.ntp.org";

    ntp_packet packetNTPout, packetNTPin;
    bzero(&packetNTPout, sizeof(ntp_packet));

    SET_LI(packetNTPout, 0);
    SET_VN(packetNTPout, 3);
    SET_MODE(packetNTPout, 3);

    int NTPsockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (NTPsockfd < 0) {
        error("Socket error");
    }

    struct hostent *NTPserver;
    NTPserver = gethostbyname(ntp_host_name);
    if (NTPserver == NULL) {
        error("Cannot get host by name");
    }

    struct sockaddr_in ntp_serv_addr;
    bzero(&ntp_serv_addr, sizeof(struct sockaddr_in));

    ntp_serv_addr.sin_family = AF_INET;

    bcopy(NTPserver->h_addr, &ntp_serv_addr.sin_addr.s_addr, NTPserver->h_length);

    int n0_port = 123;
    ntp_serv_addr.sin_port = htons(n0_port);

    struct timeval NTPtimeout;
    NTPtimeout.tv_sec = 3;
    NTPtimeout.tv_usec = 0;
    setsockopt(NTPsockfd, SOL_SOCKET, SO_RCVTIMEO, &NTPtimeout, sizeof(NTPtimeout));
    if (connect(NTPsockfd, (struct sockaddr *)&ntp_serv_addr, sizeof(ntp_serv_addr)) < 0) {
        error("Connect error");
    }

    int connfd, nready, maxfd;
    char buffer[BUFSIZE];
    pid_t childpid;
    fd_set rset;
    ssize_t n;
    socklen_t len;
    struct sockaddr_in servaddr;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        stop("Cannot Create Server's Socket\n");
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(IP_SERVER);
    servaddr.sin_port = htons(PORT);

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        stop("Cannot Bind Server's Socket\n");
    }

    if (listen(listenfd, CLIENT_MAX) < 0) {
        stop("Server Listening Problem\n");
    }

    FD_ZERO(&rset);

    for (int index = 0; index < CLIENT_MAX; index++) {
        socket_client[index] = 0;
        pseudo_client[index] = NULL;
    }

    // printf("Listening\n");

    while (1) {
        FD_ZERO(&rset);
		FD_SET(STDIN_FILENO, &rset);
        FD_SET(listenfd, &rset);
        maxfd = listenfd;
		maxfd = max(maxfd, STDIN_FILENO);

        for (int i = 0; i < CLIENT_MAX; i++) {
            if (socket_client[i] > 0) {
                FD_SET(socket_client[i], &rset);
            }
            maxfd = max(maxfd, socket_client[i]);
        }

        if (((nready = select(maxfd + 1, &rset, NULL, NULL, NULL)) < 0) && errno != EINTR) {
            stop("Select Error\n");
        }

        // Someone Just Join notDiscord
        if (FD_ISSET(listenfd, &rset)) {
            if ((connfd = accept(listenfd, (struct sockaddr *)&servaddr, &len)) < 0) {
                write(0, "Cannot connect to this client!", 31);
            }

            int clientID;
            // Add New Member's Socket
            for (int index = 0; index < CLIENT_MAX; index++) {
                if (socket_client[index] == 0) {
                    socket_client[index] = connfd;
                    clientID = index;
                    break;
                }
            }

            // Send them a warm hello
            if (send(socket_client[clientID], welcomeMessage, strlen(welcomeMessage) + 1, MSG_CONFIRM) < 0) {
                close(listenfd);
                stop("Oops! Couldn't send them a welcoming message!");
            }

            // Tell them to choose a unique nickname
            while (1) {
                if (send(socket_client[clientID], demandMsg, strlen(demandMsg) + 1, MSG_CONFIRM) < 0) {
                    stop("Problem demanding nickname from client!");
                }
                int charcnt;
                // Getting that nickname
                if ((charcnt = recv(socket_client[clientID], buffer, BUFSIZE, 0)) < 0) {
                    stop("Cannot read the nickname!");
                }
                buffer[charcnt] = '\0';
                int ok = 1;
                for (int index = 0; index < CLIENT_MAX; index++) {
                    if (pseudo_client[index] != NULL && !strcmp(buffer, pseudo_client[index])) {
                        ok = 0;
                        break;
                    }
                }
                if (!ok) {
                    if (send(socket_client[clientID], nicknameErrorMsg, strlen(nicknameErrorMsg) + 1, MSG_CONFIRM) < 0) {
                        stop("Problem redemanding nickname from client!");
                    }
                    continue;
                }
                pseudo_client[clientID] = (char *)calloc(charcnt, sizeof(char));
                if (pseudo_client[clientID] == NULL) {
                    stop("Cannot initiate memory to store client's nickname!");
                }
                strcpy(pseudo_client[clientID], buffer);
                break;
            }

            const char *successRegisterMsg = "Registration success! Have fun using notDiscord!\n\0";
            if (send(socket_client[clientID], successRegisterMsg, strlen(successRegisterMsg) + 1, MSG_CONFIRM) < 0) {
                stop("Cannot send success notification to client!");
            }
        } else if (FD_ISSET(STDIN_FILENO, &rset)) {
			bzero(buffer, BUFSIZE);
            int charcnt;
            if ((charcnt = read(STDOUT_FILENO, buffer, BUFSIZE - 1)) < 0) {
                stop("Input error!");
            }
			buffer[charcnt - 1] = '\0';
			if(!strcmp(buffer, "/exit")){
				// Kill all client socket before exit
				for(int index = 0; index < CLIENT_MAX; index++){
					if(socket_client[index]){
						const char *shutdownMsg = "Server is shutdown! Sorry for the inconviences!\n";
						if(write(socket_client[index], shutdownMsg, strlen(shutdownMsg)) < 0){
							stop("Can't send shutdown warning!");
						}
						close(socket_client[index]);
						socket_client[index] = 0;
					}
				}
				break;
			}
			if(!strcmp(buffer, "/date")){
				printf("Server's Time: %s\n", getTimestring(NTPsockfd, &packetNTPin, &packetNTPout));
			}
        }
        // I/O operation from client
        else {
            for (int index = 0; index < CLIENT_MAX; index++) {
                if (FD_ISSET(socket_client[index], &rset)) {
                    // printf("Got something\n");
                    int charcnt;
                    bzero(buffer, BUFSIZE);
                    // Disconnected
                    if ((charcnt = read(socket_client[index], buffer, BUFSIZE)) == 0) {
                        // printf("Disconnected\n");
                        close(socket_client[index]);
                        socket_client[index] = 0;
                    } else if (charcnt < 0) {
                        stop("Error Reading Message");
                    }
                    // They sent us something
                    else {
                        // printf("Message to print\n");
                        // if (write(NTPsockfd, &packetNTPout, sizeof(ntp_packet)) < 0) {
                        //     error("Error writing to NTPsocket");
                        // }
                        // // printf("Waiting to get time...\n");
                        // if (read(NTPsockfd, &packetNTPin, sizeof(ntp_packet)) < 0) {
                        //     printf("Error reading from NTPsocket\n");
                        // }
                        // long int timestamp = ntohl(packetNTPin.txTm_s) - NTP_TIMESTAMP_DELTA;
                        // time_t t = time(NULL);
                        // struct tm *tm = localtime(&t);
                        char *timestring;
                        timestring = getTimestring(NTPsockfd, &packetNTPin, &packetNTPout);
                        // printf("%s\n", timestring);
                        // timestring = ctime(&timestamp);
                        // int pos = 0;
                        // while (timestring[pos] != '\n') {
                        //     pos++;
                        // }
                        // timestring[pos] = '\0';
                        char outbuffer[BUFSIZE * 3];
                        sprintf(outbuffer, "[%s][%s]: %s\n", timestring, pseudo_client[index], buffer);
                        // printf("%s", outbuffer);
                        for (int otherIndex = 0; otherIndex < CLIENT_MAX; otherIndex++) {
                            if (socket_client[otherIndex] == 0) {
                                continue;
                            }
                            if (send(socket_client[otherIndex], outbuffer, strlen(outbuffer), MSG_CONFIRM) < 0) {
                                printf("Cannot sent to %s", pseudo_client[otherIndex]);
                                stop("");
                            }
                        }
                    }
                }
            }
        }
    }

    close(listenfd);

    exit(EXIT_SUCCESS);
}