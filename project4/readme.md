## Project 4

The purpose of this project is to compare the performance of standard file I/O
using the `read()` system call for input with memory-mapped I/O where the
`mmap()` system call allows the contents of a file to be mapped to memory.

The memory-mapped portion of the project is also extended to parallelize the
processing of memory amongst multiple threads.

The program, `proj4`, is similar to the GNU `strings` command, but instead of
finding all printable strings greater than three characters `proj4` searches
through a file byte by byte for a specific input string. It then prints the
number of times that string appears in the given file.

To compile the project, simply use the `make` command. By default `make` also
compiles `doit`, the program from Project 1 used to test `proj4` (see below). If
you don't want this, just use `make proj4`.

To run the program, use the command:

`./proj4 srcfile searchstring [size | mmap [pn]]`

where `srcfile` is the input file, and `searchstring` is the string you want to
find. The third argument tells the program whether to use read or mmap mode
(without it `proj4` will default to read mode with a chunk size of 1024). If the
third argument is a number the program will enter read mode with the input
number as the chunk size (to a maximum of 8192). If the third argument is the
string literal "mmap" the program will use mmap mode. In mmap mode, an optional
fourth argument of the form `p4` will tell the program to use four threads. The
maximum number of threads is 16.

### Performance Analysis

I tested the program with various input files to examine how the different modes
performed.  All tests were performed on an Intel i5 6200U quad-core processor
running Fedora Linux, and searching for the string "a" in the input file. The
test files were:

1. The full text of the 2009 Affordable Health Care for America Act (2,807,571
   bytes)
2. The full text of Moby Dick by Herman Melville (1,276,201 bytes)
3. The grep command-line utility executable (166,328 bytes)
4. The lyrics to “All Star” by Smash Mouth (2014 bytes)
5. The full text of the Gettysburg Address. (1511 bytes)

The graph below shows the wall clock execution times for nine different
configurations (read mode with chunk sizes of 1, 1024, 4096, and 8192 bytes; and
mmap mode with 1, 2, 4, 8, and 16 threads) on each of the input files. Please
note that the Y-axis is on a logarithmic scale.

Note: my operating system did not report any page faults during testing, so this
report only focuses on wall clock time.

Read mode with a chunk size of 1 has by far the worst performance, taking over
one second to execute the longest file. Higher chunk sizes seem to improve the
performance, but have diminishing returns on small files.

In mmap mode with only one thread, the wall clock time is already comparable to
the 8192-byte chunk size read mode results. Increasing the number of threads
increases performance on large files, but actually decreases performance on
smaller file sizes. This is likely due to the overhead of managing threads.

It seems that neither method is better in general, but multithreaded memory
mapping is faster for large file sizes and small chunk sizes for read mode leads
to worse performance.

![Proj4 Graph](https://raw.githubusercontent.com/jojonium/CS3013-Operating-Systems/master/project4/graph.png)
