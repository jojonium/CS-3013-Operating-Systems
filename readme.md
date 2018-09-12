CS3013 Operating Systems
========================

Coursework and projects from the WPI computer science class CS3013 - Operating Systems.

## Project 1

The goal of Project 1 is to introduce students to the process manipulation facilities in the Linux
Operating System. For the project I wrote a C program called _doit_, that takes another command
as an argument and executes that command. For example, executing:

`% ./doit wc foo.txt`

would invoke the _wc_ ("word count") command with an argument of _foo.txt_, which will output the
number of lines, words, and bytes in the file "foo.txt." After execution of the specified command
has completed, _doit_ displays statistics that show some of the system resources the comman used.
In particular it shows:

1. the amount of CPU time used (both user and system time (in milliseconds),
2. the elapsed "wall-clock" time for the command to execute (in milliseconds),
3. the number of times the process was preempted involuntarily (e.g. the time slice expired,
preemption by higher priority process),
4. the number of times the process gave up the CPU voluntarily (e.g. waiting for a resource),
5. the number of major page faults, which require disk I/O,
6. the number of minor page faults, which could be satisfied by reclaiming memory,
7. the maximum resident set size used, which is given in kilobytes.

_doit_ also has the ability to act as a shell. Calling _doit_ with no command line arguments will
engage shell mode, and prompt the user with the default prompt string of `==>`. You can enter
comands into this shell just as you would with a regular Linux shell, with the exception that the
_doit_ shell will also print the same usage statistics for every command executed on it.

Shell mode has four built-in commands. These are:

* _exit_ - causes the shell to terminate.
* _cd dir_ - changes the directory to _dir_.
* _set prompt = newprompt_ - changes the prompt to _newprompt_.
* _jobs_ - lists all background tasks

Each line of input may not contain no more than 128 characters or more than 32 distinct arguments.

Shell mode also supports background tasks, indicated by putting an ampersand ('&') character at the
end of an input line. When a task runs in the background the shell does not wait for the task to
complete, but instead immediately prompts the user for another command. Note that any output from the
background command will intermingle with output from the shell and other commands.
