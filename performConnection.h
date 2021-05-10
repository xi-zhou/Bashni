/*	performConnection.h */
#define WAIT_TIME 30000
#define MAX_EVENTS 30

int receive_message(int socket_desc);

int send_message(int socket_desc);

int performConnection(int socket_desc, int pipe, int player_number);

void check_server_msg(int socket_desc, int msg_length);
 void start_plus(int socket_desc);
 void start_Minus();
 void game(int socket_desc);
 int recv_all(int socket, char *buffer_ptr);
 void checkopt(int socket_desc);

 void reactAfterOKTHINK(int socket_desc);

 void reactAfterMOVE ();
void splitLine();
void rowHandling(char *line);
int letterToNumber(char piece);
void printBoard();
char numberToLetter(int number);
int getlast2(int x, int y);
void ini_board();

char client_message[BUFFER];

extern structSharedMemory *aktAddr;
extern char piecesList[2][22];
