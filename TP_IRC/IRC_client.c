#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <ncurses.h>

#define BUFSIZE 1024
#define max(a, b) ((a > b) ? a : b)

WINDOW *my_window;

void stop(char *errorMsg) {
    perror(errorMsg);
	// endwin();
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {

	// Create a chat window here - Done
	// Global WINDOW - Done
	// my_window = initscr();

    if (argc < 3) {
        stop("Missing server's IP or port\n");
    }

    char *ip_server = argv[1];
    // printf("%s %s\n", argv[1], argv[2]);
    uint16_t port = (u_int16_t)atoi(argv[2]);

    int sockfd;
    char buffer[BUFSIZE];
    int n, len;
    struct sockaddr_in servaddr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        stop("Socket Creation Failed\n");
    }

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip_server);
    servaddr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        stop("Connection Error\n");
    }

    // printf("ready to go\n");
    int maxfd;
    fd_set rset, wset;

	// Initiate inpBuffer and outBuffer - Done
	char inpBuffer[BUFSIZE], outBuffer[BUFSIZE];

	// Nonblocking, noecho mode - Done
	// cbreak();
	// noecho();

	// Get size of the window - Done
	int window_lines, window_columns;
	// getmaxyx(my_window, window_lines, window_columns);

	// Devide window in to 4 part:
	// 1. Last interactive line for message input - Done
	// Last line for chat will be at (win_lines - 1)th line
	int chat_divider_pos = window_lines - 2;

	// 2. 1/4 of the rest on left side for channel names - Done
	int channel_panel_pos = window_columns / 4;

	// 3. 1/3 of the rest on right side for member of channel names - Done
	int member_panel_pos = window_columns - window_columns / 4;

	// 4. The rest for chat display - Done by 2 & 3
	// Remember to kill window whenever exit - Done

    while (1) {
        FD_ZERO(&rset);
        FD_ZERO(&wset);
        maxfd = max(STDIN_FILENO, max(STDOUT_FILENO, sockfd)) + 1;

        FD_SET(STDIN_FILENO, &rset);
        FD_SET(sockfd, &rset);
        FD_SET(STDOUT_FILENO, &wset);
        FD_SET(sockfd, &wset);

        if (select(maxfd, &rset, &wset, NULL, NULL) < 0) {
            close(sockfd);
            stop("Select error!");
        }

		// This only happens when enter is hit
		// So move this inside the loop of getchar
		// Do this if Enter is hit, change read into a string copy
        if (FD_ISSET(STDIN_FILENO, &rset)) {
            bzero(buffer, BUFSIZE);
            int charcnt;
            if ((charcnt = read(STDOUT_FILENO, buffer, BUFSIZE - 1)) < 0) {
                close(sockfd);
                stop("Input error!");
            }
			buffer[charcnt - 1] = '\0';
			if(!strcmp(buffer, "/exit")){
				break;
			}
            if (!FD_ISSET(sockfd, &wset)) {
                fd_set wset_sock;
                FD_SET(sockfd, &wset_sock);
                if (select(sockfd + 1, NULL, &wset_sock, NULL, NULL) < 0) {
                    close(sockfd);
                    stop("Internal select error!");
                }
            }
            if (send(sockfd, buffer, charcnt, 0) < 0) {
                close(sockfd);
                stop("Error sending message");
            }
			char *rst = "\33[A\33[2K";
			write(STDIN_FILENO, rst, strlen(rst));
			// printf("Your message \"%s\" is sent!\n", buffer);
        }
        if (FD_ISSET(sockfd, &rset)) {
            bzero(outBuffer, BUFSIZE);
			int charcnt;
            if ((charcnt = read(sockfd, outBuffer, BUFSIZE - 1)) < 0) {
                close(sockfd);
                stop("Error receiving message!");
            }
            if (!FD_ISSET(STDOUT_FILENO, &wset)) {
                fd_set wset_stdout;
                FD_SET(STDOUT_FILENO, &wset_stdout);
                if (select(STDOUT_FILENO + 1, NULL, &wset_stdout, NULL, NULL) < 0) {
                    close(sockfd);
                    stop("2nd Internal select!");
                }
            }
			outBuffer[charcnt] = '\0';
			// mvprintw(0, channel_panel_pos + 1, outBuffer);
			// printf("%s\n", outBuffer);
            if(write(STDOUT_FILENO, outBuffer, charcnt) < 0){
				close(sockfd);
				stop("Error printing message!");
			}
        }
		// else{
		// 	timeout(10);
		// 	char c = getchar();
		// 	send(sockfd, &c, 1, MSG_CONFIRM);
		// }
    }

	// getchar();

    close(sockfd);
	// endwin();
    exit(EXIT_SUCCESS);
}