/* Joseph Petitti and Justin Cheng */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>

#define __NR_cs3013_syscall1 377
#define __NR_cs3013_syscall2 378

struct processinfo {
	long state;
	pid_t pid;
	pid_t parent_pid;
	pid_t youngest_child;
	pid_t younger_sibling;
	pid_t older_sibling;
	uid_t uid;
	long long start_time;
	long long user_time;
	long long sys_time;
	long long cutime;
	long long cstime;
};

long cs3013_syscall2(struct processinfo *info) {
	return (long) syscall(__NR_cs3013_syscall2, info);
}

int main(int argc, char *argv[]) {
	struct processinfo info;

	printf("syscall1 returned value: %ld\n\n", syscall(__NR_cs3013_syscall1));

	if (fork()) {
		if (fork()) {
			if(cs3013_syscall2(&info)) {
				printf("Error, exiting\n");
				exit(1);
			}
		

			printf("State: %li\n", info.state);
			printf("PID: %d\n", info.pid);
			printf("Parent PID: %d\n", info.parent_pid);
			printf("Younger sibling PID: %d\n", info.younger_sibling);
			printf("Older sibling PID: %d\n", info.older_sibling);
			printf("Youngest child PID: %d\n", info.youngest_child);
			printf("UID: %d\n", info.uid);
			printf("Process start time: %lli ns\n", info.start_time);
			printf("CPU time in user mode: %lli μs\n", info.user_time);
			printf("CPU time in system mode: %lli μs\n", info.sys_time);
			printf("User time of children: %lli μs\n", info.cutime);
			printf("System time of children: %lli μs\n", info.cstime);
		}
	}

	return 0;
}


	
