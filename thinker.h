void think();

#include <stdbool.h>

#define SIZEOFBOARD 10
#define MAXHEIGHTTOWER 25

char getNext( int i, int j, int direction);
int getlast( int x, int y);
char numberToLetter(int nbr);
int NumberToNumber(char *piece);

void simpleMove();
bool captureMove( int i, int j);
int getNextI(int j, int direction);
int getNextJ(int j, int direction);

void setcolor();
void putKing( int i, int j);
void pieceCapture( int iGegner, int jGegner);
int changeDirection(int r);
void putMen(int i, int j, int iValue, int jValue);
void writeIntoPipe(int fd);
