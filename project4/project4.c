#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <pthread.h>
#include <stdint.h>
#include "mailbox.h"

#define DEFAULT_CHUNK_SIZE 1024
#define MAX_THREADS 16
#define DEFAULT_THREADS 4

char *buf;
char *pattern;

void *countPart(void *arg);
void read_mode(int chunkSize, int infile);
void mmap_mode(int infile);



int main(int argc, char *argv[]) {
	size_t chunkSize;
	int infile; // input file descriptor

	// check arguments
	if (argc < 3) {
		// print usage
		printf("Usage:\n\t");
		printf("\033[1m./proj4\033[0m ");
		printf("\033[3msrcfile searchstring\033[23m ");
		printf("[\033[3msize\033[23m|[\033[3mmmap\033[23m ");
		printf("[\033[1mp\033[0m\033[3mnumTheads\033[23m]]]\n");
		exit(1);
	}

	// set chunk size
	if (argc > 3) {
		if (strcmp(argv[3], "mmap") == 0) {
			chunkSize = 0;
		} else if (argv[3][0] == 'p') {
			chunkSize = 0;
		} else {
			chunkSize = atoi(argv[3]);
		}
		if (chunkSize > 8192) { // enforce chunkSize limit
			printf("Chunk size exceeded limit (8192 bytes). ");
			printf("Defaulting to %d bytes\n", DEFAULT_CHUNK_SIZE);
			chunkSize = DEFAULT_CHUNK_SIZE;
		}
       	} else {
		chunkSize = DEFAULT_CHUNK_SIZE;
	}


	// set number of threads
	numThreads = 1;
	if (chunkSize == 0 && argv[3][0] == 'p') {
		if ((numThreads=atoi(++argv[3]))<1 || numThreads>MAX_THREADS) {
			printf("Invalid numThreads, defaulting to %d\n",
				       	DEFAULT_THREADS);
			numThreads = DEFAULT_THREADS;
		}
	} else if (argc > 4 && argv[4][0] == 'p') {
		if ((numThreads=atoi(++argv[4]))<1 || numThreads>MAX_THREADS) {
			printf("Invalid numThreads, defaulting to %d\n",
				       	DEFAULT_THREADS);
			numThreads = DEFAULT_THREADS;
		}
	}

	// try to open the file
	if ((infile = open(argv[1], O_RDONLY)) == -1) {
		printf("Failed to open file: %s\n", argv[1]);
		exit(1);
	}

	// assign the global pattern
	pattern = argv[2];

	// start reading
	if (chunkSize == 0) {
		// MMAP MODE
		mmap_mode(infile);
	}
	else {
		// READ MODE
		read_mode(chunkSize, infile);
	}


	return 0;
}

void read_mode(int chunkSize, int infile) {
	int matches, searchIndex, fileSize, bufIndex, charsRead;

	printf("Entering read mode with chunkSize = %d bytes\n", chunkSize);

	buf = malloc(chunkSize * sizeof(char));
	matches = 0;
	searchIndex = 0;
	fileSize = 0;
	bufIndex = 0;
	while ((charsRead = read(infile, buf, chunkSize))) {
		fileSize += charsRead;
		while (bufIndex < charsRead) {
			if (pattern[searchIndex] == '\0') {
				// matched to the end
				matches++;
				searchIndex = 0;
			} 
			else if (buf[bufIndex] == pattern[searchIndex]){
				// one character matched
				searchIndex++;
			} 
			else {
				searchIndex = 0;
			}

			bufIndex++;
		}
		bufIndex = 0;
	}
	close(infile);
	free(buf);

	printf("File size: %d bytes.\n", fileSize);
	printf("Occurrences of the string \"%s\": %d\n", pattern, matches);
}

void mmap_mode(int infile) {
	int i, step, temp, matches;
	struct stat sb;
	struct msg *received;
	struct msg **mailToSend;

	printf("Entering mmap mode with numThreads = %d\n", numThreads);

	// stat file to obtain its size
	if (fstat(infile, &sb) < 0) {
		printf("Failed to stat file\n");
		exit(1);
	}

	// map memory
	if ((buf = (char *)mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, infile,
					0)) == (char *) -1) {
		printf("Failed to mmap file\n");
		exit(1);
	}

	// allocate memory
	postOffice = (struct msg **)malloc((numThreads + 1) * 
			sizeof(struct msg *));
	mailToSend = (struct msg **)malloc(numThreads * sizeof(struct msg *));
	semArray1 = (sem_t **)malloc((numThreads + 1) * sizeof(sem_t *));
	semArray2 = (sem_t **)malloc((numThreads + 1) * sizeof(sem_t *));
	rope = (pthread_t **)malloc(numThreads * sizeof(pthread_t *));
	received = (struct msg *)malloc(sizeof(struct msg));

	// make the mailboxes
	for (i = 0; i <= numThreads; i++) {
		// allocate more memory
		semArray1[i] = (sem_t *)malloc(sizeof(sem_t));
		semArray2[i] = (sem_t *)malloc(sizeof(sem_t));
		sem_init(semArray1[i], 0, 1); // psem
		sem_init(semArray2[i], 0, 0); // csem
	}

	// make the messages to send
	step = sb.st_size / numThreads;
	temp = 0;
	for (i = 0; i < numThreads; i++) {
		mailToSend[i] = (struct msg *)malloc(sizeof(struct msg *));
		mailToSend[i]->iSender = 0;
		mailToSend[i]->type = RANGE;
		mailToSend[i]->value1 = temp + 1;
		temp += step;
		// last message needs to be different to account for trunc error
		mailToSend[i]->value2 = (i == numThreads-1) ? sb.st_size : temp;
	}

	// send 'em
	for (i = 0; i < numThreads; i++) {
		SendMsg(i + 1, mailToSend[i]);
	}

	// make the threads
	for (i = 0; i < numThreads; i++) {
		rope[i] = (pthread_t *)malloc(sizeof(pthread_t *));
		if (pthread_create(rope[i], NULL, &countPart,
				       	(void *)(intptr_t)(i + 1))) {
			printf("error creating thread %d\n", i + 1);
		}
	}

	// receive 'em
	matches = 0;
	for (i = 1; i <= numThreads; i++) {
		RecvMsg(0, received);
		if (received->type != ALLDONE) {
			printf("Child sent wrong type of message\n");
		}
		matches += received->value1;
	}

	// clean up
	if (munmap(buf, sb.st_size) < 0) {
		printf("Failed to unmap memory\n");
		exit(1);
	}
	for (i = 0; i < numThreads; i++) {
		free(mailToSend[i]);
		sem_destroy(semArray1[i]);
		sem_destroy(semArray2[i]);
		free(rope[i]);
	}
	free(received);
	sem_destroy(semArray1[numThreads]);
	sem_destroy(semArray2[numThreads]);
	free(semArray1);
	free(semArray2);
	free(rope);
	free(postOffice);
	free(mailToSend);
	close(infile);

	printf("File size: %ld bytes.\n", sb.st_size);
	printf("Occurrences of the string \"%s\": %d\n", pattern, matches);
}

/* void *countPart(void *arg)
 * Waits for a message from the parent, then searches for matches to pattern in
 * buf starting with buf[message->value1] and ending with buf[message->value2].
 * The function can read after buf[message->value2] if it's still finishing a
 * match.
 */
void *countPart(void *arg) {
	int index, matches, bufIndex, searchIndex;
	struct msg *message;

	index = (int)(intptr_t)arg;
	
	message = (struct msg *)malloc(sizeof(struct msg));

	RecvMsg(index, message); // wait for mail from parent

	if (message->type != RANGE) {
		printf("Thread %d received the wrong type of message\n", index);
	}

	// rewrite the message to send it back
	message->iSender = index;
	message->type = ALLDONE;

	matches = 0;
	bufIndex = message->value1;
	searchIndex = 0;

	while (bufIndex <= message->value2 || searchIndex > 0) {
		if (pattern[searchIndex] == '\0') {
			// matched to the end
			matches++;
			searchIndex = 0;
		} 
		else if (buf[bufIndex] == pattern[searchIndex]){
			// one character matched
			searchIndex++;
		} 
		else {
			searchIndex = 0;
		}

		bufIndex++;
	}
	bufIndex = 0;
	
	// send the matches back
	message->value1 = matches;
	SendMsg(0, message);

	return (void *)0;
}
