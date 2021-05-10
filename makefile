CC = gcc
CFLAGS = -Wall -Wextra -Werror -g

SRCS = $(wildcard *.c)
OBJ = $(SRCS:.c=.o)

NAME = sysprak-client
# An example of a GAME_ID="0n48pmu8zml2t"

all: link 

link: $(OBJ)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: play
play: link
	./$(NAME) -g $(GAME_ID) -p $(PLAYER)

.PHONY: valgrind
valgrind: link
	valgrind --leak-check=full --trace-children=yes ./$(NAME) -g $(GAME_ID) -p $(PLAYER) -d

clean:
	rm -f $(OBJ) $(NAME)
