#ifndef CONNECT_INFO_H
#define CONNECT_INFO_H

#define IP_SERVER "192.168.1.45"
#define PORT 8000

const char *RED = "\033[0;31m";
const char *BLUE = "\033[0;34m";
const char *GREEN = "\033[0;32m";
const char *RESET = "\033[0;0m";

typedef struct MYFILE_{
	int fd;
	void *buffer_read;
	void *buffer_write;
	int ind_read;
	int ind_write;
} MYFILE;

#endif