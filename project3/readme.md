## Project 3

The goal of Project 3 is to learn how to use semaphores and threads as a synchronization mechanism to build a
message passing mechanism that can be used among a set of threads within the same process. The project uses
the `pthreads` library for thread and synchronization routines.

The project can be compiled with the `make` command. This creates two executables named `addem` and `life`.

### Part 1

Part 1 of the project is a trivial program that really serves to introduce the concepts of threads and semaphores.
This program is adds up all the positive integers below an input number, and prints the result to the terminal.
Call the program with the command:

`% ./addem numThreads number`

where `numThreads` is the number of threads you want the program to use, and `number` is the target number. If
`numThreads` is greater than the maximum number of threads (10) then that maximum will be used instead.

The program is more than a trivial loop because it is multithreaded. It initializes a number of threads equal to the
given `inputThreads` argument, along with a "mailbox" for each, consisting of a message pointer and two semaphores.
With these semaphores the mailbox can act like a shared buffer of size 1. The parent thread divides up the work
of the program among each of the child threads, passing them each a message with a range of values to add up. When
the child threads are done adding they pass a new message with the total back to the parent, and the parent adds up
each of these totals for the final result.

### Part 2

Part 2 is a more interesting program - a distributed version of John Conway's Game of Life. Like Part 1, this
program is also multithreaded. It uses the same mailbox message passing system as the first part. The parent thread
divides up the work of calculating a new game board for each of the children, passing them a message with a range
of rows that thread should play the game for. The children send an ALLDONE message back to the parent when they have finished, and the parent synchronizes these results before moving on to the next generation and repeating the
process.

To run the program, use the command:

`% ./life numThreads, filename, generations, [print] [pause]`

where `numThreads` is the number of threads you want to use, `filename` is the name of a text file with the starting
pattern to be used, `generations` is the maximum number of iterations to play, and `print` and `pause` are optional
`y` or `n` arguments indicating whether the program should print the board after each step and whether it should
pause to wait for user input after each step respectively.

