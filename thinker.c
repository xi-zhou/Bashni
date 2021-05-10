#include "global.h"
#include "performConnection.h"
#include "thinker.h"
char localboard[SIZEOFBOARD][SIZEOFBOARD][MAXHEIGHTTOWER];
char client_message[BUFFER];
extern char board[8][8];

char playsMen = '*';
char playsKing = '*';
char myMen = '*';
char myKing = '*';

bool firstMove = true;
bool simpleZug = true;
int lastdirection = 0;
int neudirection = 0;

int neuI = 0; // neu i position vom stein nach capture
int neuJ = 1; // neu j position vom stein nach capture
bool cancapture = false;
bool returncapture = false;
bool easymove = true;

int capture = 0;

int IDSharedMemory;
structSharedMemory *aktAddr;
void think (int fd){
  easymove = true;
 returncapture = false;
 cancapture = false;
 simpleZug = true;
 lastdirection = 0;
 neudirection = 0;
 firstMove = true;
 capture = 0;
    printf("***thinking.. \n");
    memset(client_message,0,strlen(client_message));
  //   snprintf(client_message, BUFFER, "PLAY %s:%s\n", start, dest);

    strcpy(client_message, "PLAY ");
    lastdirection = 0;
    shmat(IDSharedMemory, NULL, 0);

    if (aktAddr->housekeeping.signal_flag==1)
    {

        for (int i = 0; i <= SIZEOFBOARD; i++) {
            for (int j = 0; j <= SIZEOFBOARD; j++) {
                strcpy(localboard[i][j], aktAddr->board[i][j]);
            }
        }


        setcolor();

        for (int i = 8; i > 0; i--) {
          for (int j = 1; j < 9; j++) {

            returncapture = captureMove(i, j);
            if (returncapture == true) {
              printf("final capture move \n");
              writeIntoPipe(fd);
              return;
            }
          }
        }
        if (easymove == true) {
          printf(
              "can't find a captureMove. go to simpleMove\n");
            simpleMove();
              writeIntoPipe(fd);
              return;
              }

      return;
    }
    perror("Error FLAG\n");
      exit(EXIT_FAILURE);
}

void writeIntoPipe(int fd) {
  strcat(client_message, "\n");
  if (write(fd, &client_message, sizeof(client_message)) != sizeof(client_message)) {
      perror("error when write into pipe");
      exit(EXIT_FAILURE);
  } else {
      printf("write Success\n %s",client_message);
      return;
}
}
void setcolor() {
  printf("Your playerNr is %i, 0 is white, 1 is black\n", aktAddr->me.player_number);
  if (aktAddr->me.player_number == 0) {
    playsMen = 'b';
    playsKing = 'B';
    myMen = 'w';
    myKing = 'W';
  } else if (aktAddr->me.player_number == 1) {
    playsMen = 'w';
    playsKing = 'W';
    myMen = 'b';
    myKing = 'B';
  }
}

int changeDirection(int r) {
  //1:linksoben 2:rechtsoben 3:linksunten 4:rechtsunten
  if (r > 0 && r < 5) {
    switch (r) {
    case 1:
      return 4;
    case 2:
      return 3;
    case 3:
      return 2;
    case 4:
      return 1;
    }
  }
  return -50;
}
void simpleMove() {
  for (int i = 9; i >= 0; i--) {
  for (int j = 0; j < 10; j++) {
    int y = getlast(i, j);
    printf("%c ", localboard[j][i][y]);
  }
  printf("\n");
}

  simpleZug = true;
  int i;
  int j;

  printf("myMen: %c\n", myMen);

  while (simpleZug == true) { // 3

    for (i = 9; i >= 0; i--) {   // 1
      for (j = 0; j < 10; j++) { // 2

        int y = getlast(i, j);

        if (localboard[i][j][y] == myMen) {

          int x = 1;
          while (x < 5) { // 5
            // abprüfen, wo "vorne" ist
            if (myMen == 'b') {
              if (x == 1 || x == 2) {
                x = 3;
              }
            } else if (myMen == 'w') {
              if (x == 3 || x == 4) {
                x = 7;
              }
            }

            if (getNext(i, j, x) == '.') { // 6

              char erstes[3];                // mye Position
              erstes[0] = numberToLetter(i); // buchstabe erste Position
              erstes[1] = j + '0';           // zahl erste Position
              erstes[2] = '\0';
              strcat(client_message, erstes); // mye Position


              char zweites[3]; // wohin er gehen will
              zweites[0] = ':';

              int iValue = getNextI(i, x);
              zweites[1] = numberToLetter(iValue); // buchstabe wohin
              int jValue = getNextJ(j, x);         // zahl wohin
              zweites[2] = jValue + '0';

              strcat(client_message, zweites);



              printf("client_message1: %s\n", client_message);
              return;
            }
            x++; // 6
          }
          simpleZug = false;
        } else if (localboard[i][j][0] == myKing) { // 7
          int x = 1;
          while (x < 5) {
            if (getNext(i, j, x) == '.') {

              char erstes[2];
              erstes[0] = numberToLetter(i);
              erstes[1] = j;
              strcat(client_message, erstes);

              char zweites[4];
              zweites[0] = ':';

              int iValue = getNextI(i, x);

              zweites[1] = numberToLetter(iValue);
              int jValue = getNextJ(j, x);

              zweites[2] = jValue;
              zweites[3] = '\0';
              strcat(client_message, zweites);

              printf("client_message: %s\n", client_message);
              x++;
              return;
            }
          }

          simpleZug = false;

        } // 7 zu
      }   // 3 zu
    }     // 2 zu
  }       // 1
}

bool captureMove(int i, int j) {
  neudirection = changeDirection(lastdirection);
  int y = getlast(i, j);
  /******************************************
  * men capture
  ********************************************/
 if (localboard[i][j][y] == myMen) {
    int x = 1;

    while(x<5){  // test all 4 direction
      if (x == neudirection) {
           x++;
        }
      // ob diagonal ein gegner steht:
      if (getNext(i, j, x) == playsKing || getNext(i, j, x) == playsMen) {

        int iValue = getNextI(i, x); // VON Gegner
        int jValue = getNextJ(j, x); // von gegner
        char tmp;
        tmp = getNext(iValue, jValue, x); // obs danach leer ist
          if (tmp == '.') {

          cancapture = true; // wichtig. damit er in die pipe schreibt

          capture++;
          if (capture == 1) {


            char my[3];
            my[0] = numberToLetter(i);
            my[1] = j + '0';
            my[2] = '\0';
            strcat(client_message, my);


          }
          int i2;
          int j2;
          char two[3];
          i2 = getNextI(iValue, x);    // x vom leeren feld
          j2 = getNextJ(jValue, x);    // y vom leeren feld

          two[1] = numberToLetter(i2); // leeres feld x in string rein schreiben
          two[2] = j2 + '0';           // y leeres feld in string

          two[0] = ':';

          strcat(client_message, two);


          printf("found a capture %s\n", two); // position vom gegner: ausgabe
          pieceCapture(iValue, jValue);       // gegner, obersten loeschen
          putMen(i, j, i2, j2);
          if (myMen == 'b' && j2 == 8) {
            putKing(i2, j2);
          } else if (myMen == 'w' && j2 == 1) {
            putKing(i2, j2);
          }
          printf("we are going to: %s\n", two); // ausgabe leeres feld
          printf("until now: %s\n", client_message);
          lastdirection = x;

          // REKURSION:
          easymove = false;
          captureMove(i2, j2); // i und j muss das vom leeren feld sein.
        }
      }
      x++;
      }



    /******************************************
    * king capture
    ********************************************/
  } else if (localboard[i][j][y] == myKing) {
    printf("KING\n" );
    neudirection = changeDirection(lastdirection);
    int x = 1;

    if (x == lastdirection) {
      x++;
    } // fuer die directionen

    while (x < 5) {
      int neuI = i;
      int neuJ = j;
      for (int moving = 0; moving < 8; moving++) {
         //printf("HIER IST X IM captureDEN ZUG: %i\n", x);
        char nextChar = getNext(neuI, neuJ, x);
         //printf("HIER IST X IM captureDEN ZUG: %i\n", x);
        if (nextChar == '.') {
          // nächstes Feld
          neuI = getNextI(i, x);
          neuJ = getNextJ(j, x);
        } else if (nextChar == playsMen || nextChar == playsKing) {
          // Variablen fuer Gegnerposition deklarieren und setzen
          int iValue;
          int jValue;
          iValue = getNextI(i, x);

          jValue = getNextJ(j, x);

          if (getNext(iValue, jValue, x) == '.') {
            if (capture == 0) {
              char erster[3];
              erster[0] = numberToLetter(i);
              erster[1] = j + '0';
              erster[2] = '\0';
              strcat(client_message, erster);

            }
            // freies Feld hinter zu capturedem Gegner
            char two[4];
            neuI = getNextI(iValue, x);
            neuJ = getNextJ(jValue, x);
            // speichert die Variablen der danach zu "besuchenden" Position in
            // Array
            two[0] = ':';
            two[1] = numberToLetter(neuI);
            two[2] = neuJ + '0';
            two[3] = '\0';


            strcat(client_message, two);


            pieceCapture(iValue, jValue);
            putMen(i, j, neuI, neuJ);
            cancapture = true; // wichtig. damit er in die pipe schreibt
            capture++;
            lastdirection = x;
            easymove = false;

            // rekursiver Funktionsaufruf
            captureMove(neuI, neuJ);
          }
        } else if (nextChar == 'r') {
          x++;
        }
      }
      x++;
    }
  }

  // geht hier rein, wenn es mindestens einen Zug gibt.
  if (cancapture == true) {

    return true;
  }
  return false; // falls nix zutrifft;
}
int getlast(int x, int y) {
  for (int i = 0; i <= MAXHEIGHTTOWER; i++) {
    if (localboard[x][y][i] == '\0') {
      return i - 1;
    }
  }
  return 1;
}
int getNextI(int i, int direction) {
  if (direction <= 4 && direction >= 1) {
    int next;
    switch (direction) {
    case 1:
      next = i - 1;
      break;
    case 2:
      next = i + 1;
      break;
    case 3:
      next = i - 1;
      break;
    case 4:
      next = i + 1;
      break;
    }
    return next;
  } else {
    printf("Es gibt nur 4 directionen - Fehler. get NextI \n");
    return -50;
  }
  return -50;
}
int getNextJ(int j, int direction) {
  if (direction <= 4 && direction >= 1) {
    int next;
    switch (direction) {
    case 1:
      next = j + 1;
      break;
    case 3:
      next = j - 1;
      break;
    case 2:
      next = j + 1;
      break;
    case 4:
      next = j - 1;
      break;
    }
    return next;
  } else {
    printf("Es gibt nur 4 directionen - Fehler. getNextJ \n");
    return -50;
  }
}

void pieceCapture(int iGegner, int jGegner) {
  // Hilfsvariable für Position des lastn stein
  int posLast;
  posLast = getlast(iGegner, jGegner);
  // abgefrage, ob Brett dann leer ist
  if (posLast == 0) {
    localboard[iGegner][jGegner][0] = '.';
    localboard[iGegner][jGegner][1] = '\0';
  } else {
    localboard[iGegner][jGegner][posLast] = '\0';
  }
}
void putMen(int i, int j, int iValue, int jValue) {
  // Kopie unseres Spielbretts
  char zwischenKopie[MAXHEIGHTTOWER];
  // String kopieren
  strcpy(zwischenKopie, localboard[i][j]);
  // urspr. Feld loeschen
  localboard[i][j][0] = '.';
  localboard[i][j][1] = '\0';
  // neus Feld setzen
  strcpy(localboard[iValue][jValue], zwischenKopie);
}
void putKing(int i, int j) {
  if (myMen == 'w') {
    // Hilfsvariable, in die die Stelle des lastn Zeichens gespeichert
    // wird
    int temp;
    // lasts Zeichen finden
    temp = getlast(i, j);
    // Dame setzen
    localboard[i][j][temp] = 'W';
  } else if (myMen == 'b') {
    // Hilfsvariable, in die die Stelle des lastn Zeichens gespeichert
    // wird
    int temp;
    // lasts Zeichen finden
    temp = getlast(i, j);
    // Dame setzen
    localboard[i][j][temp] = 'B';
  }
}
char getNext(int i, int j, int direction) {
// gibt den Wert des nächsten Felds in einer direction aus
  if (direction <= 4 && direction >= 1) {
      char inhalt;
      int y = -50;
      switch (direction) {
      case 1:
        y = getlast(i - 1, j + 1);
        inhalt = localboard[i - 1][j + 1][y];
        break;
      case 2:
        y = getlast(i + 1, j + 1);
        inhalt = localboard[i + 1][j + 1][y];
        break;
      case 3:
        y = getlast(i - 1, j - 1);
        inhalt = localboard[i - 1][j - 1][y];
        break;
      case 4:
        y = getlast(i + 1, j - 1);

        inhalt = localboard[i + 1][j - 1][y];
        break;
      }
    return inhalt;
  } else {
    // printf("Es gibt nur 4 directionen - Fehler. getNext!! \n");
    return 'f';
  }
}
