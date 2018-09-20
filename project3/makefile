CFLAGS = -Wall -g
LIB = -lpthread
CC = gcc

all: addem life

life: life.o board.o
	$(CC) $(CFLAGS) life.o board.o -o life $(LIB)

life.o: life.c board.h
	$(CC) $(CFLAGS) -c life.c -o life.o

board.o: board.c
	$(CC) $(CFLAGS) -c board.c -o board.o

addem: addem.o mailbox.o
	$(CC) $(CFLAGS) addem.o mailbox.o -o addem $(LIB)

addem.o: addem.c mailbox.h
	$(CC) $(CFLAGS) -c addem.c -o addem.o

mailbox.o: mailbox.c
	$(CC) $(CFLAGS) -c mailbox.c -o mailbox.o

clean:
	rm -f addem.o mailbox.o addem life.o board.o life
