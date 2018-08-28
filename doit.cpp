/* doit.C */
/* Joseph Petitti - CS 3013 Project 1 */
#include <iostream>
using namespace std;
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#define BUFSIZE 64

int execute(char ** newArgs) {
	int i = 0;
	int pid;

	/* print out the arguments */
	cout << "newArgs: ";
	while (newArgs[i] != NULL) {	
		cout << newArgs[i] << " ";
		i++;
	}
	cout << "\n";

	/* fork it */
	if ((pid = fork()) < 0) {
		cerr << "Fork error\n";
		exit(1);
	}
	else if (pid == 0) {
		/* child process */
		cout << "child (" << getpid() << ")\n";
		/* execute the command */
		if (execvp(newArgs[0], newArgs) < 0) {
			cerr << "execvp error\n";
			exit(1);
		}
		return 0;
	}
	else {
		/* parent */
		wait(0);
		cout << "parent (" << getpid() << ")\n";
		return 0;
	}
}

int doLine(char **out) {
	char line[256];
	char *token;
	int position = 0;

	cin.getline(line, 256);
	token = strtok(line, " ");
	while (token != NULL) {
		out[position] = token;
		token = strtok(NULL, " ");	
		position++;
	}
	out[position] = NULL;
	
	int status = strcmp(out[0], "exit");
	
	if (status) {
		execute(out);
		return 1;
	} else {
		return 0;
	}
}
		
	

int main(int argc, char *argv[]) {
	char **newArgs = (char **)malloc(64 * sizeof(char *));
	int i;
	auto prompt = "==>";
	int status = 1;

	/* if there are command line arguments */
	if (argc > 1) {
		for (i = 1; i < argc; i++) {
			newArgs[i - 1] = argv[i];
		}
		newArgs[argc - 1] = NULL;
		execute(newArgs);
	}
	else {
		while (status) {
			cout << prompt << " ";
			status = doLine(newArgs);
		} 
	}

}

