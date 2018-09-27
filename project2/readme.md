## Project 2

The goal of Project 2 is to learn how to use Loadable Kernel Modules (LKMs) to change the operation
of the base operating system arbitrarily. In Pre-Project 2 we compiled a modified version of the Linux
Kernel with three new system calls:

* `cs3013_syscall1`
* `cs3013_syscall2`
* `cs3013_syscall3`

In project 2, I wrote a C program, _mymodule.c_ that intercepts several system calls and replaces
them with new system calls. In particular, it does the following:

* Intercepts the `sys_open` call, replacing it with a modified function that prints something like
"User 1000 is opening the file: /etc/motd" when it is called by a normal user, then calls the old
`sys_open`.
* Intercepts the `sys_close` call, replacing it with a modified function that prints something like
"User 1000 is closing file descriptor: 2" when it is called by a normal user, then calls the old
`sys_close`.
* Intercepts the `cs3013_syscall2` call, which originally did nothing, and replaces it with a new
custom function (detailed below).

The new `cs3013_syscall2` takes the following form:

`long cs3013_syscall2(struct processinfo *info);`

where `*info` is a pointer to a data structure in user space where the system call will put
information about the process. It returns 0 if successful or an error indication if not successful.

The `processinfo` struct is defined as follows:

```c
struct processinfo {
	long state;             // current state of process
	pid_t pid;              // process ID of this process
	pid_t parent_pid;       // process ID of parent
	pid_t youngest_child;   // process ID of youngest child
	pid_t younger_sibling;  // pid of the next younger sibling
	pid_t older_sibling;    // pid of next older sibling
	uid_t uid;              // used ID of process owner
	long long start_time;   // process start time in nanoseconds since boot time
	long long user_time;    // CPU time in user mode (microseconds)
	long long sys_time;     // CPU time in system mode (microseconds)
	long long cutime;       // user time of children (microseconds)
	long long cstime;       // system time of children (microseconds)
}; // struct processinfo
```

`test.c` is a simple C program to test the system call modifications. After compiling the Loadable
Kernel Module from `mymodule.c` and the test executable from test.c with the `make` command, insert
it into the kernel using:

`sudo insmod mymodule.ko`

Then compile the test program with:

`gcc test.c -o test`

and run it with `./test`. It should call the modified `cs3013_syscall2` and print out all the
information stored in a new user-level `struct processinfo`.

