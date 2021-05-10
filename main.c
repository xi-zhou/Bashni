/*	main.c */
#define _GNU_SOURCE
#include "global.h"

#include "userguide.h"
#include "thinker.h"
#include "performConnection.h"
#include "connector.h"
#include "config.h"

int fd[2];
int 	socket_desc;
char	*gameid;
int	player_number;
int	dflag; // debug flag sets verbose mode
//static struct field (*board)[63]; // board state shared mem var
//struct game_data game;
int IDSharedMemory;
structSharedMemory *aktAddr;

void deleteSharedMemory(int SHMID) {
	free (aktAddr->me.player_name);
  if (shmctl(SHMID, IPC_RMID, NULL) < 0) {
    perror("\n\ncan't delete SHM\n\n");
  } else {
    shmctl(SHMID, IPC_RMID, 0);
  }
}


void signal_handler(int signal)
{

  if (signal == SIGUSR1) {
    printf("signal arrived, going to call think()\n");
    think(fd[1]);
  }
}


int
main(int argc, char *argv[])
{

    int    ret;
    while ((ret=getopt(argc, argv, "g:p:f:d")) != -1) {
        switch (ret) {

        case 'g':
            //gameid = atoi(optarg);
            gameid = optarg;
            if (strlen(gameid) != 13) {
                perror ("Wrong Game-ID format: length has to be 13.");
                exit(EXIT_FAILURE);
            }
            break;

        case 'p':
            player_number = atoi(optarg)-1;
            break;

        case 'd':
            dflag =1;
            break;

        case 'f':
            setFileName(optarg);
            break;

        default:
            return -1;
            break;
        }
    }

	/*
		if (argc == 5 ){
		printf("Game ID ist: %s\n", gameid);
		printf("Anzahl der Spieler: %d\n", number_of_players);
	*/
		if (dflag == 1) {printf("***dflag is set\n");}


		// Setup board var as shared mem space
	/*	board = mmap(NULL, sizeof *board,
				PROT_READ | PROT_WRITE,
				MAP_SHARED | MAP_ANON, -1, 0);
  */
  IDSharedMemory = shmget(IPC_PRIVATE, 4000, IPC_CREAT | SHM_R | SHM_W);

  if (IDSharedMemory < 0) {
    perror("shmat fail");
  } else {
    aktAddr = shmat(IDSharedMemory, NULL, 0);
    printf("store board in SHM successful\n");
  }

		pid_t connector_pid;

		// Preparing the pipe
		//int fd [2];
   		if (pipe (fd) < 0) {
      		perror ("pipe error.");
      		exit (EXIT_FAILURE);
   		}



		if ((connector_pid = fork()) < 0) {
			perror ("fork error.");
			exit(EXIT_FAILURE);
		}
		else if (connector_pid == 0) {
			//child process: connector

			aktAddr->housekeeping.connector_pid=getpid();

			close (fd[1]);   /*Closing the writing-site of the pipe*/

			initiateConnection();

			performConnection(socket_desc, fd[0],player_number);

			//close (fd[0]);

			// Write_board_state_to_shared_memory_space
			// Proof of concept:
		  //(*board)[1].height = 3;
			//
			//

			exit(EXIT_SUCCESS);
		}
		else {
			//parent process: thinker

			aktAddr->housekeeping.thinker_pid=getpid();


			close (fd[0]);   /*Closing the reading-site in pipe */

			// char* send = "Hallo";
			// int a = write (fd[1], send, strlen(send));
			// printf("In pipe wurde geschrieben: %s, %d\n", send, a);


			signal(SIGUSR1, signal_handler);

      aktAddr = shmat(IDSharedMemory, NULL, 0);
      if (aktAddr == NULL) {
        perror("store in SHM failed");
      }
      if (shmdt(aktAddr) != 0) {
        fprintf(stderr, "error detaching SHM \n");
        exit(EXIT_FAILURE);
      }



			//close(fd[1]);

			if ((waitpid(connector_pid, NULL, 0)) < 0) {
				perror ("error waitting for child process.");
				exit(EXIT_FAILURE);
			}
		//sleep(3);

		// POC for shared mem space continued
		/*printf("Height of tower at adress \'1\' = %d\n", (*board)[1].height);
		int mumrc;
		mumrc = munmap(board, sizeof *board);
		if (dflag == 1) {printf("mumap return code = %d\n", mumrc);}
		}
    */

	/*
		} else userguide();
	*/
        return 0;

}

	deleteSharedMemory(IDSharedMemory);

}
