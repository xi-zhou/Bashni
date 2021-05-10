/*	global.h */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <err.h>
#include <errno.h>
#include <signal.h>

#define BUFFER 1024
#define SIZEOFBOARD 10
#define MAXHEIGHTTOWER 25


struct piece {

	int colour;

};

// struct field {
	
// 	int height;
// 	struct piece composition[24];

// };

struct player_account {

	int player_number;
	char *player_name;
	int registration_status;
};

struct house_keeping {
	int signal_flag;
	pid_t thinker_pid;
	pid_t connector_pid;

};

typedef struct {
	int player_count;
	struct player_account me;
    struct player_account *players;

	char board[SIZEOFBOARD][SIZEOFBOARD][MAXHEIGHTTOWER];
    char piecesList[2][22];

    struct house_keeping housekeeping;
} structSharedMemory;

extern structSharedMemory *aktAddr;
extern char piecesList[2][22];