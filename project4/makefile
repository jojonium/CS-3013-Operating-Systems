CFLAGS = -g -Wall
LIB = -lpthread
CC = gcc

all: proj4

proj4: project4.o mailbox.o
	$(CC) $(CFLAGS) project4.o mailbox.o -o proj4 $(LIB)

project4.o: project4.c mailbox.h
	$(CC) $(CFLAGS) -c project4.c -o project4.o

mailbox.o: mailbox.c
	$(CC) $(CFLAGS) -c mailbox.c -o mailbox.o

clean:
	rm -f proj4 mailbox.o project4.o
