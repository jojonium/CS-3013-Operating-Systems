#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>

#define DEFAULT_CHUNK_SIZE 1024

int main(int argc, char *argv[]) {
	size_t chunkSize;
	int infile; // input file descriptor
	int charsRead, bufIndex, searchIndex, matches, fileSize;
	struct stat sb;
	char *buf;

	// check arguments
	if (argc < 3) {
		printf("Usage:\n\t./proj4 srcfile searchstring [size|mmap]\n");
		exit(1);
	}

	// set chunk size
	if (argc > 3) {
		if (strcmp(argv[3], "mmap") == 0) {
			printf("Starting in memory mapping mode...\n");
			chunkSize = 0;
		} else {
			printf("Starting in read mode...\n");
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
	bufIndex = 0;
	if (chunkSize == 0) {
		// MMAP MODE

		// stat file to obtain its size
		if (fstat(infile, &sb) < 0) {
			printf("Failed to stat file\n");
			exit(1);
		}

		// map memory
		if ((buf = (char *)mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED,
						infile, 0)) == (char *) -1) {
			printf("Failed to mmap file\n");
			exit(1);
		}

		while (buf[bufIndex] != '\0') {
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

		fileSize = sb.st_size;

		// clean up
		if (munmap(buf, sb.st_size) < 0) {
			printf("Failed to unmap memory\n");
			exit(1);
		}
	}
	else {
		// READ MODE
		while ((charsRead = read(infile, buf, chunkSize))) {
			fileSize += charsRead;
			while (bufIndex < charsRead) {
				if (argv[2][searchIndex] == '\0') {
					// matched to the end
					matches++;
					searchIndex = 0;
				} 
				else if (buf[bufIndex] == argv[2][searchIndex]){
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
	}

	close(infile);

	printf("File size: %d bytes.\n", fileSize);
	printf("Occurrences of the string \"%s\": %d\n", argv[2], matches);

	return 0;
}
