#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include "mailbox.h"

#define MAXTHREAD 10

void *addit(void *arg);

int main(int argc, char *argv[]) {
	int i, target, step, temp, result;
	struct msg *received;
	struct msg **mailToSend;

	// check arguments
	if (argc < 3) {
		printf("ERROR: too few arguments\n");
		printf("Expected the form;\n\t./addem numThreads number\n");
		exit(1);
	}

	inputThreads = atoi(argv[1]);
	target = atoi(argv[2]);

	if (inputThreads < 1) {
		printf("Input number of threads too small, defaulting to 1\n");
		inputThreads = 1;
	} else if (inputThreads > MAXTHREAD) {
		printf("Input number of threads too high, defaulting to %d\n", MAXTHREAD);
		inputThreads = MAXTHREAD;
	}

	// allocate memory
	postOffice = (struct msg **)malloc((inputThreads + 1) * sizeof(struct msg *));
	mailToSend = (struct msg **)malloc(inputThreads * sizeof(struct msg *));
	semArray = (sem_t *)malloc((inputThreads + 1) * sizeof(sem_t));
	rope = (pthread_t **)malloc(inputThreads * sizeof(pthread_t *));
	received = (struct msg *)malloc(sizeof(struct msg));

	printf("allocated memory\n");

	// make the mailboxes
	for (i = 0; i <= inputThreads; i++) {
		sem_init((semArray + i), 0, 1);
	}

	printf("made mailboxes\n");

	// make the threads
	for (i = 0; i < inputThreads - 1; i++) {
		rope[i] = (pthread_t *)malloc(sizeof(pthread_t *));
		if (pthread_create(rope[i], NULL, addit, (void *)(intptr_t)(i + 1))) {
			printf("error creating thread %d\n", i + 1);
		}
		printf("made thread #%d\n", i);
	}

	printf("made threads\n");

	// make messages to send
	step = target / inputThreads;
	temp = 1;
	for (i = 0; i < inputThreads; i++) {
		mailToSend[i] = (struct msg *)malloc(sizeof(struct msg *));
		mailToSend[i]->iSender = 0;
		mailToSend[i]->type = RANGE;
		mailToSend[i]->value1 = temp;
		temp += step;
		// last message needs to be different to account for rounding errors
		mailToSend[i]->value2 = (i == inputThreads - 1) ? target : temp;
	}
	
	printf("made messages\n");

	// send 'em
	for (i = 0; i < inputThreads; i++) {
		printf("sending message with value1: %d and value2: %d\n", mailToSend[i]->value1, mailToSend[i]->value2);
		SendMsg(i + 1, mailToSend[i]);
	}

	// receive 'em
	for (i = 1; i <= inputThreads; i++) {
		RecvMsg(i, received);
		if (received->type != ALLDONE) {
			printf("Child sent wrong type message");
		}
		result += received->value1;
	}
	
	printf("The total for 1 to %d using %d threads is %d.\n", target, inputThreads, result);

	return 0;

}

// waits for a message from the parent, then adds the sum of integers 
// between message->value1 and message->value2 inclusive
void *addit(void *arg) {
	printf("in addit\n");
	int index, out;
	struct msg *message, *response;

	index = *((int *)arg);
	printf("thread number %d\n", index);
	message = (struct msg *)malloc(sizeof(struct msg));
	response = (struct msg *)malloc(sizeof(struct msg));
	RecvMsg(0, message); // wait for mail from parent

	if (message->type != RANGE || message->value1 > message->value2) {
		printf("Something went horribly wrong\n");
		pthread_exit((void *)1);
	}

	response->iSender = index;
	response->type = ALLDONE;

	for (index = message->value1; index <= message->value2; index++) {
		out += index;
	}

	response->value1 = out;

	SendMsg(message->iSender, response); // send the sum back

	return (void *)0;
}
