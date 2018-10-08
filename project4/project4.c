#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DEFAULT_CHUNK_SIZE 1024

int main(int argc, char *argv[]) {
	size_t chunkSize;
	int infile; // input file descriptor
	int charsRead, bufIndex, searchIndex, matches, fileSize;
	char *buf;

	// check arguments
	if (argc < 3) {
		printf("Usage:\n\t./proj4 srcfile searchstring [size|mmap]\n");
		exit(1);
	}

	// set chunk size
	if (argc > 3) {
		chunkSize = atoi(argv[3]);
		if (chunkSize > 8192) { // enforce chunkSize limit
			printf("Err: chunk size exceeded limit (8192 bytes). ");
			printf("Defaulting to %d bytes\n", DEFAULT_CHUNK_SIZE);
			chunkSize = DEFAULT_CHUNK_SIZE;
		}
       	} else {
		chunkSize = DEFAULT_CHUNK_SIZE;
	}

	buf = malloc(chunkSize * sizeof(char));

	// try to open the file
	if ((infile = open(argv[1], O_RDONLY)) == -1) {
		printf("Failed to open file: %s\n", argv[1]);
		exit(1);
	}

	// start reading
	matches = 0;
	searchIndex = 0;
	fileSize = 0;
	while ((charsRead = read(infile, buf, chunkSize))) {
		bufIndex = 0;
		fileSize += charsRead;
		while (bufIndex < charsRead) {
			if (argv[2][searchIndex] == '\0') {
				// matched to the end
				matches++;
				searchIndex = 0;
			} else if (buf[bufIndex] == argv[2][searchIndex]) {
				// one character matched
				searchIndex++;
			} else {
				searchIndex = 0;
			}

			bufIndex++;
		}
	}

	printf("File size: %d bytes.\n", fileSize);
	printf("Occurrences of the string \"%s\": %d\n", argv[2], matches);

	return 0;
}
