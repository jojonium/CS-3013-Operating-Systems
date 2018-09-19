CFLAGS = -Wall -g

all: project3
	make project3

project3: project3.o mailbox.o
	gcc $(CFLAGS) project3.o mailbox.o -o addem -lm -lpthread -lrt

project3.o: project3.c mailbox.h
	gcc $(CFLAGS) -c project3.c -o project3.o

mailbox.o: mailbox.c
	gcc $(CFLAGS) -c mailbox.c -o mailbox.o

clean:
	rm -f project3.o mailbox.o project3
