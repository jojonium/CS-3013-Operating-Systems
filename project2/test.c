#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>

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

	if(cs3013_syscall2(&info)) {
		printf("Error, exiting\n");
		exit(1);
	}

	printf("State: %li\n", info.state);
	printf("PID: %d\n", info.pid);
	printf("Parent PID: %d\n", info.parent_pid);
	printf("UID: %d\n", info.uid);

	return 0;
}

	
