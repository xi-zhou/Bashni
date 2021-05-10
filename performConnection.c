#include "global.h"
#include "config.h"
#include "connector.h"
#include "performConnection.h"
#include "thinker.h"

char 	*gameid;
static char server_message[BUFFER];
int		msg_length;
int	 	countSrvMsg=1;
int 	msgCounter=0;

int		socket_desc, pipe_desc;
int		dflag;
bool 	in_loop = false;

//preparing epoll materials
int epoll_desc;
struct epoll_event events[MAX_EVENTS];

char piecesList[2][22];
extern structSharedMemory *aktAddr;


int fd[2];

int
send_message(int socket_desc)
{
	if (dflag == 1) {
		printf("***Sending following message:\n");
		printf("***%s\n", client_message);
	}
	msg_length = send(socket_desc, client_message, strlen(client_message), 0);
	if (msg_length < 0) {
		if (dflag == 1) {
			printf("***Send message failed.\n");
		}
		return -1;
	}
       	else {
		if (dflag == 1) {
			printf("***Send message Success.\n\n");
		}
		return 0;
	}

}

int
recv_all(int socket, char *buffer_ptr)
{
	// char* checkpointer;
	int ret=0;
	int ctrl=0;
	int temp=0;
	char message[BUFFER];

	do{
		epoll_wait(epoll_desc, events, 1, WAIT_TIME);

		//	epoll_wait(epoll_desc, events, MAX_EVENTS, WAIT_TIME);
		temp = recv(socket, message, BUFFER, 0);

		if (temp<0)
		{
			perror("Failure recieving the message \n");
			exit(EXIT_FAILURE);
		}

		ret += temp;

		if (dflag==1)
		{
			printf("***Currently ret is = %d\n", ret);
		}

		strncat(buffer_ptr, message, temp);
		if (buffer_ptr[ret-1] == '\n')
		{
			ctrl = 1;
		}


	} while (ctrl == 0);

	buffer_ptr[ret]='\0';

	return ret;
}

int
receive_message(int socket_desc)
{
/*	epoll_wait(epoll_desc, events, 1, 30000);
		if (events->data.fd == socket_desc){
			printf("monitoring server msg\n");
*/
  	memset(&server_message, 0, sizeof(server_message));
	msg_length = recv_all(socket_desc, server_message);
	if (msg_length < 0) {
		printf("Receive message failed.\n");
		//memset(&server_message, 0, sizeof(server_message));
		return -1;
	}
  	else {
		msgCounter++;
		if (dflag == 1) {
			printf("***Received message from server successfully.\n");
			printf("***Server says:\n");
			printf("***%s\n", server_message);
			printf("***End of the message\n\n");
		}
		check_server_msg(socket_desc, msg_length);
	}
  // memset(&server_message, 0, sizeof(server_message));


return 0;
}

void
check_server_msg(int socket_desc, int msg_length) {
	if ((server_message[msg_length-1] == '\n')) { // if message ends with /n
		if (server_message[0] == '+') { // if the message starts with +
			start_plus(socket_desc);
		} else { // if the message starts with -
			start_Minus();
		}
	} else {
		perror("ERROR: Message was not fully received");
	}
}

void
start_Minus() {
	printf("Server returned failure message\n");
	printf("The received message is : \n%s\n", server_message);

	if (strncmp(server_message, "- Invalid Move: ", strlen("- Invalid Move: ")) == 0) {
		printf("Invalid play\n");
	}
	if(strncmp(server_message, "- TIMEOUT Be faster next time", strlen("- TIMEOUT Be faster next time")) == 0) {
		printf("Client too slow\n");
	}
	if(strncmp(server_message, "- Did not get the expected PLAY command", strlen("- Did not get the expected PLAY command")) == 0) {
		printf("Unexpected message\n");
	}
	if(strncmp(server_message, "- No free player", strlen("- No free player")) == 0) {
		printf("no free player \n");
	}

	printf("Disconnecting..\n");
	printf("Stopping sysprak-client..\n\n");
	exit(EXIT_FAILURE);
}

void
start_plus(int socket_desc) {

	if (in_loop == false){

		if (countSrvMsg == 1) { // first Server Message

			countSrvMsg++;

			int major, minor; //+Server Version

			sscanf(server_message, "+ MNM Gameserver v%d.%d %*s", &major, &minor);
			printf("Server version: %d.%d\n", major, minor);

			if(dflag==1){
				printf("***1st message recieved\n\n");
			}

			return;

		} else if (countSrvMsg == 2) { // second Server Message

			countSrvMsg++;

			printf("Client version accepted proceeding to send game id\n");

			if(dflag==1){
				printf("***2nd message recieved\n\n");
			}

			return;

		} else if (countSrvMsg == 3) { // third Server Message

			countSrvMsg++;

			char gamekind[BUFFER], gamename[BUFFER]; //+Gamekind-Name and Game-Name

			sscanf(server_message, "+ PLAYING %s\n+ %[^\t\n]", gamekind, gamename);
			printf("Gamekind: %s\nGamename: %s\n", gamekind, gamename);

			if(dflag==1){
				printf("***3rd message recieved\n\n");
			}

			return;

		} else if (countSrvMsg == 4) { // fourth Server Message

			countSrvMsg++;

			int myPlayerNr, playersCount, otherPlayerNr;
			char myPlayerName[BUFFER], rest[BUFFER], temp[BUFFER];


			sscanf (server_message, "+ YOU %d %[^\t\n]\n+ TOTAL %d\n%[^\t]", &myPlayerNr, myPlayerName, &playersCount, rest);

			printf("Your name is %s and you player number is %d. The players count is %d \n", myPlayerName, myPlayerNr, playersCount);


			struct player_account players[playersCount];

			players[myPlayerNr].player_number = myPlayerNr;
			players[myPlayerNr].registration_status = 0;
			players[myPlayerNr].player_name = malloc(sizeof(char) * (strlen(myPlayerName) + 1));
			memcpy(players[myPlayerNr].player_name, myPlayerName, strlen(myPlayerName));
			players[myPlayerNr].player_name[strlen(myPlayerName)]='\0';


			for (int i = 1; i < playersCount; i++){

				sscanf (rest, "+ %d %[^\t+] %[^\t]", &otherPlayerNr, temp, rest);

				players[otherPlayerNr].player_number = otherPlayerNr;

				players[otherPlayerNr].player_name = malloc(sizeof(char) * (strlen(temp)-2));
				memcpy(players[otherPlayerNr].player_name, temp, strlen(temp)-3);
				players[otherPlayerNr].player_name[strlen(temp)-3]='\0';

				players[otherPlayerNr].registration_status = temp[strlen(temp)];


				if(dflag==1){
				printf("***Test:temp is=%s and rest is=%s\n", temp, rest);
				}


				printf ("Player number %d is called %s. His ready-status is: %d \n", players[i].player_number, players[i].player_name, players[i].registration_status);
			}

			//All the infos regarding the players are getting saved in the shm
			aktAddr->player_count = playersCount;
			aktAddr->me.player_number = myPlayerNr;
			aktAddr->me.registration_status = 0;



			for (int i = 0; i < playersCount; i++){
        		free(players[i].player_name);
    		}

			checkopt(socket_desc);
			// go directly into game, until in_loop=false



		} else if(countSrvMsg == 5){

			countSrvMsg++;

			printf("5th message\n" );
		if(dflag==1)	printf("%s\n",server_message );
			//printf("%s\n",server_message );
			checkopt(socket_desc);
		}

	} else {
		game(socket_desc);
		return;
	}

}

void
game(int socket_desc) {
	//int nrPiecs;
	//int movetime;
	if (strncmp(server_message, "+ WAIT", strlen("+ WAIT")) == 0) {
		printf(":\n");
		snprintf(client_message, BUFFER, "OKWAIT\n");
		send_message(socket_desc);
		receive_message(socket_desc);
 	} else if (strstr(server_message, "+ PIECESLIST") != NULL) {
		printf("now move...\n");
		reactAfterMOVE(server_message);


		aktAddr->housekeeping.signal_flag=1;

		kill(getppid(), SIGUSR1);
		printf("signal geschickt..\n");

		snprintf(client_message, BUFFER-1, "THINKING\n");
		send_message(socket_desc);
		receive_message(socket_desc); //OK thinking,goto 207

 	} else if (strncmp(server_message, "+ GAMEOVER", strlen("+ GAMEOVER")) == 0) {
		printf("now gameover...\n");
		//gameOverMethod(msg);*/
	} else if (strncmp(server_message, "+ OKTHINK", strlen("+ OKTHINK")) == 0) {
		printf("ok thinking, be fast, server waits only for a short time  \n");
		epoll_wait(epoll_desc, events, 1, 30000);
			if (events->data.fd == pipe_desc){
			read(fd[0], client_message, BUFFER);
			printf("received from pipe: %s\n", client_message);
			send_message(socket_desc);
			receive_message(socket_desc);
			}else{
		 	printf("monitoring server msg\n");
		 	receive_message(socket_desc);
		}
	}else if (strncmp(server_message, "+ MOVEOK", strlen("+ MOVEOK")) == 0) {
		printf("move ok, move successful\n");
		receive_message(socket_desc);
	}else {
		receive_message(socket_desc);
	}

}


int
performConnection(int socket_desc, int pipe, int player_number)
{
	pipe_desc=pipe;

	//preparing epole
	epoll_desc = epoll_create1(0);
	if (epoll_desc == -1){
		perror("Failure during the creation of the epoll file descriptor.\n");
	}

	//adding the socket event to the epoll instance
	struct epoll_event socket_event;
	socket_event.data.ptr = NULL;
	socket_event.data.fd = socket_desc;
	socket_event.events = EPOLLIN;

	if (epoll_ctl(epoll_desc, EPOLL_CTL_ADD, socket_desc, &socket_event)){
		fprintf(stderr, "Failed to add the socket file descriptor to the epoll instance\n");
			close(epoll_desc);
	}

	//adding the pipe event to the epoll instance
	struct epoll_event pipe_event;
	pipe_event.data.ptr = NULL;
	pipe_event.data.fd = pipe_desc;
	pipe_event.events = EPOLLIN;
	if (epoll_ctl(epoll_desc, EPOLL_CTL_ADD, pipe_desc, &pipe_event)){
		fprintf(stderr, "Failed to add the pipe file descriptor to the epoll instance\n");
			close(epoll_desc);
	}


	//NOTE: use <epoll_wait(epoll_desc, events, MAX_EVENTS, WAIT_TIME);> before reading something from a the pipe or from the socket

	//Start Handshake (Prolog Phase)
	receive_message(socket_desc);

	snprintf(client_message, BUFFER-1, "VERSION 2.3\n");
	send_message(socket_desc);

	receive_message(socket_desc);

	snprintf(client_message, BUFFER-1, "ID %s\n", gameid);

	send_message(socket_desc);

	receive_message(socket_desc);

	snprintf(client_message, BUFFER-1, "PLAYER %d\n", player_number);
	printf("***Client message plnr is :%s***\n", client_message);
	send_message(socket_desc);

	receive_message(socket_desc); //endplayers;



	return 0;
}



void
checkopt(int socket_desc){
	//+move +gameover +wait	in_loop = true;

	// char cpyopt [BUFFER];

	// strcpy(cpyopt, copyMessage);

	if (strstr(server_message, "+ PIECESLIST") != NULL) {
		printf("\n\n----------spielzug---------\n\n");
		reactAfterMOVE(server_message);
		in_loop = true;

		printf("we are going to move\n" );

		aktAddr->housekeeping.signal_flag=1;

		kill(getppid(), SIGUSR1);
		printf("signal geschickt.\n");

		snprintf(client_message, BUFFER, "THINKING\n");
		send_message(socket_desc);
		receive_message(socket_desc);  //recv:OKTHINKING ,goto OKthing in game


	}else if(strstr (server_message,"+ GAMEOVER")!= NULL){
		receive_message(socket_desc); //spielbrett
		printf("game over now\n" );

 	}else if(strstr (server_message,"+ WAIT")!= NULL){
		in_loop = true;
		snprintf(client_message, BUFFER, "OKWAIT\n");
		send_message(socket_desc);
		receive_message(socket_desc);
 	//	}else{ //(strcmp (server_message),"QUIT") {
	//	return -1;
	}else{
		printf("\nmove not in 4th message\n" );

		receive_message(socket_desc);
	}
	}


void
reactAfterMOVE (){
	//ini board
	char hboard[SIZEOFBOARD][SIZEOFBOARD][MAXHEIGHTTOWER] = {
      {"r", "r", "r", "r", "r", "r", "r", "r", "r", "r"},
      {"r", ".", "_", ".", "_", ".", "_", ".", "_", "r"},
      {"r", "_", ".", "_", ".", "_", ".", "_", ".", "r"},
      {"r", ".", "_", ".", "_", ".", "_", ".", "_", "r"},
      {"r", "_", ".", "_", ".", "_", ".", "_", ".", "r"},
      {"r", ".", "_", ".", "_", ".", "_", ".", "_", "r"},
      {"r", "_", ".", "_", ".", "_", ".", "_", ".", "r"},
      {"r", ".", "_", ".", "_", ".", "_", ".", "_", "r"},
      {"r", "_", ".", "_", ".", "_", ".", "_", ".", "r"},
      {"r", "r", "r", "r", "r", "r", "r", "r", "r", "r"}};
  memcpy(aktAddr->board, hboard, sizeof(aktAddr->board));

	splitLine(server_message);

	printBoard();
}


void
splitLine() {
  //printf("nachricht ist folgende: %s\n", moveStringMessage);
  char delimeter[] = "\n";

  char *ptr;
  // int zeile = 1;

  ptr = strtok(server_message, delimeter);
  while (ptr != NULL) {
    char copy[30];
    strcpy(copy, ptr);
    // printf("zeile %i: %s\n", zeile++, copy);
    rowHandling(copy);
    ptr = strtok(NULL, delimeter);
  }
}

void
rowHandling(char *line) {
	//  board zuorden und in SHM schreiben
  	if (strncmp(line, "+ b@", 4) == 0) {

		int file = letterToNumber(line[4]);
    	int rang;
    	sscanf(line, "+ b@%*c%d\n", &rang);

		char *ptr;
    	ptr = &aktAddr->board[file][rang][0];

		if (ptr[0] == '.') {
     		ptr[0] = 'b';
    	} else {
      	strcat(ptr, "b");
  		}
  	}

  	if (strncmp(line, "+ B@", 4) == 0) {

		int file = letterToNumber(line[4]);
		int rang;
		sscanf(line, "+ B@%*c%d\n", &rang);

		char *ptr;
		ptr = &aktAddr->board[file][rang][0];

		if (ptr[0] == '.') {
		ptr[0] = 'B';
		} else {
		strcat(ptr, "B");
		}
  	}

	if (strncmp(line, "+ w@", 4) == 0) {
		int file = letterToNumber(line[4]);
		int rang;
		sscanf(line, "+ w@%*c%d\n", &rang);

		char *ptr;
		ptr = &aktAddr->board[file][rang][0];

		if (ptr[0] == '.') {
		ptr[0] = 'w';
		} else {
		strcat(ptr, "w");
		}

	}
	if (strncmp(line, "+ W@", 4) == 0) {

		int file = letterToNumber(line[4]);
		int rang;
		sscanf(line, "+ W@%*c%d\n", &rang);

		char *ptr;
		ptr = &aktAddr->board[file][rang][0];

		if (ptr[0] == '.') {
		ptr[0] = 'W';
		} else {
		strcat(ptr, "W");
		}
	}
}


void
printBoard() {
//lese board SHM and print
  int z = 8;

  printf("\n\n\n");
  printf("   A B C D E F G H   \n");
  printf(" +-----------------+ \n");

  for (int i = 8; i > 0; i--) {
    printf("%i| ", z);
    for (int j = 1; j < 9; j++) {
      int y = getlast2(i, j);
      printf("%c ", aktAddr->board[j][i][y]);
    }
    printf("|%i", z);
    printf("\n");
    z--;
  }
  printf(" +-----------------+ \n");
  printf("   A B C D E F G H   \n");
  printf("\n");

  printf("White Towers\n\n");
  printf("============\n\n");

  // printf("%s\n", ptr);

  for (int i = 1; i < 9; i++) {
    for (int j = 1; j < 9; j++) {

      char *ptr;
      ptr = &aktAddr->board[i][j][0];

      if (ptr[0] == 'w') {
        printf("%c%i: %s\n", numberToLetter(i), j, ptr);

      } else if (ptr[0] == 'W') {
        printf("%c%i: %s\n", numberToLetter(i), j, ptr);
      }
    }
  }

  printf("\n");

  printf("Black Towers\n\n");
  printf("============\n\n");

  for (int i = 1; i < 9; i++) {
    for (int j = 1; j < 9; j++) {

      char *ptr;
      ptr = &aktAddr->board[i][j][0];

      if (ptr[0] == 'b') {
        printf("%c%i: %s\n", numberToLetter(i), j, ptr);

      } else if (ptr[0] == 'B') {
        printf("%c%i: %s\n", numberToLetter(i), j, ptr);
      }
    }
  }
}

int
letterToNumber(char piece) {
    // board position
  int first;

  switch (piece) {
  case 'A':
    first = 1;
    break;
  case 'B':
    first = 2;
    break;
  case 'C':
    first = 3;
    break;
  case 'D':
    first = 4;
    break;
  case 'E':
    first = 5;
    break;
  case 'F':
    first = 6;
    break;
  case 'G':
    first = 7;
    break;
  case 'H':
    first = 8;
    break;
  }
  return first;
}

char
numberToLetter(int number) {

  char letter;

  switch (number) {
  case 1:
    letter = 'A';
    break;
  case 2:
    letter = 'B';
    break;
  case 3:
    letter = 'C';
    break;
  case 4:
    letter = 'D';
    break;
  case 5:
    letter = 'E';
    break;
  case 6:
    letter = 'F';
    break;
  case 7:
    letter = 'G';
    break;
  case 8:
    letter = 'H';
    break;
  default:
    return 'Z';
  }

  return letter;
}


int
getlast2(int x, int y) {
  for (int i = 0; i <= MAXHEIGHTTOWER; i++) {
    if (aktAddr->board[x][y][i] == '\0') {
      return i - 1;
    }
  }
  return 1;
}
