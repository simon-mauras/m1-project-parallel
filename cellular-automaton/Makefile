CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2
SRC = main.c sequential.c
OBJ= $(SRC:.c=.o)

all: run

%.o: %.c Makefile
	$(CC) -o $@ -c $< $(CFLAGS)

run: $(OBJ)
	$(CC) -o run $(OBJ)

clean:
	rm run create_input display_output *.o
	
create_input: create_input.c
	gcc -o create_input create_input.c $(CFLAGS)

display_output: display_output.c
	gcc -o display_output display_output.c $(CFLAGS)

