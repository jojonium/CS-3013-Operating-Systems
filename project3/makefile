CFLAGS = -Wall -g

all: addem

addem: addem.o mailbox.o
	gcc $(CFLAGS) addem.o mailbox.o -o addem -lm -lpthread -lrt

addem.o: addem.c mailbox.h
	gcc $(CFLAGS) -c addem.c -o addem.o

mailbox.o: mailbox.c
	gcc $(CFLAGS) -c mailbox.c -o mailbox.o

clean:
	rm -f addem.o mailbox.o addem
