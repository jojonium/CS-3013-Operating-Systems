/* doit.C */
/* Joseph Petitti - CS 3013 Project 1 */
#include <iostream>
using namespace std;
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#define MAX_CHARS 128
#define MAX_ARGS 32
char *prompt = (char *)malloc(16 * sizeof(char));

int execute(char ** newArgs) {
	int pid;
	struct rusage usage;
	struct timeval ut_end, st_end, wc_start, wc_end;

	gettimeofday(&wc_start, NULL);

	/* fork it */
	pid = fork();
		
	if (pid < 0) {
		cerr << "Fork error\n";
		exit(1);
	} else if (pid == 0) {
		/* child process */
		/* execute the command */
		if (execvp(newArgs[0], newArgs) < 0) {
			cerr << "execvp error\n";
			exit(1);
		}
		return 0;
	} else {
		/* parent */
		/* wait for child to finish */
		wait(0);

		/* print usage info */
		getrusage(RUSAGE_CHILDREN, &usage);
		gettimeofday(&wc_end, NULL);
		ut_end = usage.ru_utime;
		st_end = usage.ru_stime;
		double userTime = ((ut_end.tv_sec * 1000) + (ut_end.tv_usec / 1000));
		double systemTime = ((st_end.tv_sec * 1000) + (st_end.tv_usec / 1000));
		double wallClockTime = ((wc_end.tv_sec * 1000) + (wc_end.tv_usec / 1000)) - ((wc_start.tv_sec * 1000) + (wc_start.tv_usec / 1000));
		cout << "USAGE STATISTICS:\n";
		cout << "*** system time = " << systemTime << " milliseconds\n";
		cout << "*** user time = " << userTime << " milliseconds\n";
		cout << "*** wall clock time = " << wallClockTime << " milliseconds\n";
		cout << "*** involuntary context switches = " << usage.ru_nivcsw << endl;
		cout << "*** voluntary context switches = " << usage.ru_nvcsw << endl;
		cout << "*** page faults requiring I/O = " << usage.ru_majflt << endl;
		cout << "*** page faults serviced without I/O = " << usage.ru_minflt << endl;
		cout << "*** maximum resident set size = " << usage.ru_maxrss << " kilobytes\n";
		return 0;
	}
}

/* grabs a line from cin and executes it. */
/* returns 1 if the execution should stop (because of "exit" or error), 0 otherwise */
int doLine(char **out) {
	char line[MAX_CHARS];
	char *token;
	int position = 0;

	cin.getline(line, MAX_CHARS);
	token = strtok(line, " ");
	while (token != NULL) {
		out[position] = token;
		token = strtok(NULL, " ");	
		position++;
	}
	out[position] = NULL;
	
	if (strcmp(out[0], "exit") == 0) { // exit command
		return 1;
	} else if (strcmp(out[0], "cd") == 0 && out[1] != NULL)  { // cd command
		if (chdir(out[1]) != 0)
			cerr << "chdir error\n";
	} else if (strcmp(out[0], "set") == 0 && strcmp(out[1], "prompt") == 0 && strcmp(out[2], "=") == 0 && out[3] != NULL) { // set prompt command
		strcpy(prompt, out[3]);
	} else {
		execute(out);
	}

	return 0;
}
		
	

int main(int argc, char *argv[]) {
	char **newArgs = (char **)malloc(MAX_ARGS * sizeof(char *));
	int i;
	int status = 0;
	prompt[0] = '=';
	prompt[1] = '=';
	prompt[2] = '>';
	prompt[3] = '\0';

	/* if there are command line arguments */
	if (argc > 1) {
		for (i = 1; i < argc; i++) {
			newArgs[i - 1] = argv[i];
		}
		newArgs[argc - 1] = NULL;
		execute(newArgs);
	}
	else {
		while (!status) {
			cout << prompt << " ";
			status = doLine(newArgs);
		} 
	}

}

