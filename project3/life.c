#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include "board.h"

int main(int argc, char *argv[]) {
	int temp, i, j, done, step;
	struct msg **mailToSend;
	struct msg *received;
	FILE *fp;
	char c;
	char **A; // board 1
	char **B; // board 2
	char **C; // board 3
	char *line = NULL;
	size_t len = 0;
	int doPrint = 0;
	int doPause = 0;

	/*
	int i, target, step, temp, result;
	struct msg *received;
	struct msg **mailToSend;
	*/

	// check arguments
	if (argc < 4) {
		printf("ERROR: too few arguments\n");
		printf("Expected the form:\n\t./life numThreads filename generations [print] [pause]\n");
		exit(1);
	}

	inputThreads = atoi(argv[1]);

	if (inputThreads < 1) {
		printf("Input number of threads too small, defaulting to 1\n");
		inputThreads = 1;
	} else if (inputThreads > MAXTHREAD) {
		printf("Input number of threads too high, defaulting to %d\n", MAXTHREAD);
		inputThreads = MAXTHREAD;
	}

	gens = atol(argv[3]);
	if (argc > 4)
		doPrint = (argv[4][0] == 'y');
	if (argc > 5)
		doPause = (argv[5][0] == 'y');

	// open file
	if ((fp = fopen(argv[2], "r")) == NULL) {
		printf("File open error\n");
		exit(1);
	}

	// count lines and columns
	rows = 0;
	cols = 0;
	while ((temp = getline(&line, &len, fp)) != -1) {
		temp = temp / 2;
		if (temp > cols) cols = temp;
		rows++;
	}

	// check grid size
	if (cols > MAXGRID) {
		printf("Input file too wide, defaulting to %d columns\n", MAXGRID);
		cols = MAXGRID;
	}
	if (rows > MAXGRID) {
		printf("Input file too tall, defaulting to %d rows\n", MAXGRID);
		rows = MAXGRID;
	}
	if (rows < MAXTHREAD && rows > inputThreads) {
		printf("too few rows for this many threads, decreasing thread count to %d\n", rows);
		inputThreads = rows;
	}

	// allocate memory
	A = (char **)malloc(cols * sizeof(char *));
	B = (char **)malloc(cols * sizeof(char *));
	C = (char **)malloc(cols * sizeof(char *));
	postOffice = (struct msg **)malloc((inputThreads + 1) * sizeof(struct msg *));
	mailToSend = (struct msg **)malloc(inputThreads * sizeof(struct msg *));
	semArray1 = (sem_t **)malloc((inputThreads + 1) * sizeof(sem_t *));
	semArray2 = (sem_t **)malloc((inputThreads + 1) * sizeof(sem_t *));
	rope = (pthread_t **)malloc(inputThreads * sizeof(pthread_t *));
	received = (struct msg *)malloc(sizeof(struct msg));
	for (i = 0; i < cols; i++) {
		A[i] = (char *)malloc(rows * sizeof(char));
		B[i] = (char *)malloc(rows * sizeof(char));
		C[i] = (char *)malloc(rows * sizeof(char));
	}

	// make the mailboxes
	for (i = 0; i <= inputThreads; i++) {
		// allocate more memory
		semArray1[i] = (sem_t *)malloc(sizeof(sem_t));
		semArray2[i] = (sem_t *)malloc(sizeof(sem_t));
		sem_init(semArray1[i], 0, 1); //psem
		sem_init(semArray2[i], 0, 0); //csem
	}

	// make the messages to send
	step = rows / inputThreads;
	temp = 0;
	for (i = 0; i < inputThreads; i++) {
		mailToSend[i] = (struct msg *)malloc(sizeof(struct msg *));
		mailToSend[i]->iSender = 0;
		mailToSend[i]->type = RANGE;
		mailToSend[i]->value1 = temp;
		temp += step;
		// lasat message needs to be different to account for rounding errors
		mailToSend[i]->value2 = (i == inputThreads - 1) ? rows - 1 : temp;
	}

	// send it
	for (i = 0; i < inputThreads; i++) {
		SendMsg(i + 1, mailToSend[i]);
	}

	// fill grids with dead cells
	for (j = 0; j < rows; j++) {
		for (i = 0; i < cols; i++) {
			A[i][j] = OFF;
			B[i][j] = OFF;
			C[i][j] = OFF;
		}
	}

	// write file into grid
	rewind(fp);
	i = 0;
	j = 0;
	while ((c = fgetc(fp)) != EOF) {
		if (i >= cols) {
			i = 0;
			j++;
			while (c != '\n') c = fgetc(fp); // zoom to end of line
		} else if (c == '1') {
			A[i][j] = ON;
			i++;
		} else if (c == '0') {
			i++;
		} else if (c == '\n') {
			i = 0;
			j++;
		}
	}

	// prepare to play
	iteration = 0;
	boards[0] = A;
	boards[1] = B;
	boards[2] = C;

	// create threads
	for (i = 0; i < inputThreads; i++) {
		rope[i] = (pthread_t *)malloc(sizeof(pthread_t *));
		if (pthread_create(rope[i], NULL, &playT, (void *)(intptr_t)(i + 1))) {
			printf("error creating thread %d\n", i + 1);
		}
	}

	// print gen 0
	printGrid(A, iteration); 

	// begin play loop
	while (iteration < gens) {
		// send the go for the next generation
		for (i = 1; i <= inputThreads; i++) {
			mailToSend[i - 1]->type = GO;
			mailToSend[i - 1]->iSender = 0;
			SendMsg(i, mailToSend[i - 1]);
		}

		// wait for messages back from child threads
		for (i = 1; i <= inputThreads; i++) {
			RecvMsg(0, received);
			if (received->type != GENDONE) {
				printf("Parent expected GENDONE\n");
			}
		}
		
		if (doPrint && iteration > 0) printGrid(boards[iteration % 3], iteration);

		if ((done = checkDone(boards[iteration % 3],
				boards[(iteration + 1) % 3], boards[(iteration + 2) % 3])) == 1) {
			printf("Repeating, exiting.\n");
			break;
		} else if (done == 2) {
			printf("Oscillating, exiting.\n");
			break;
		}

		if (doPause) {
			printf("press enter to continue...\n");
			getchar();
		}

		iteration ++;
	}

	// print final grid
	printf("=== Final Board ===\n");
	printGrid(boards[iteration % 3], iteration);

	// clean up 
	for (i = 0; i < cols; i++) {
		free(A[i]);
		free(B[i]);
		free(C[i]);
	}
	for (i = 0; i < inputThreads; i++) {
		free(mailToSend[i]);
		sem_destroy(semArray1[i]);
		sem_destroy(semArray2[i]);
		free(rope[i]);
	}
	free(A);
	free(B);
	free(C);
	free(postOffice);
	free(mailToSend);
	sem_destroy(semArray1[inputThreads]);
	sem_destroy(semArray2[inputThreads]);
	free(semArray1);
	free(semArray2);
	free(rope);
	free(received);

	return 0;
}
