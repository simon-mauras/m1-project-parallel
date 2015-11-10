CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2
SRC = types.c main.c sequential.c
OBJ= $(SRC:.c=.o)

all: run create_input display_output

%.o: %.c Makefile
	$(CC) -o $@ -c $< $(CFLAGS)

run: $(OBJ)
	$(CC) -o run $(OBJ)

clean:
	rm run create_input display_output *.o
	
create_input: create_input.o types.o
	gcc -o create_input create_input.o types.o $(CFLAGS)

display_output: display_output.o types.o
	gcc -o display_output display_output.o types.o $(CFLAGS)

