CSE 130 Professor Veenstra
Spring 2023 UCSC 

Assignment 1: Command-line Memory

Goal: 
	This project involves a review of Linux system calls, buffering file I/O, memory management, and c-string parsing. 

Assignment Details: 
	In this assignment, a program, memory, will be written. The program will provide a get/set memory abstraction for files in a Linux directory. The program will take a command in from stdin and carry out the command in the current working directory. 

Constriants: 
	If memory detects any other errors (such as if it cannot write all of the requested content to a file for any reason), the program should produce the text "Operation Failed" to stderr and exit with a return code of 1. 
	memory must be reasonably time efficent: it should buffer the input and output. 
	memory must be reasonably space efficent: it should use at most 1MB of memory regardless of input.
	memory must not leak any memory (i.e., it should be free all of the memory that it allocates)
	memory must not leak any file descriptors (i.e., it should close all teh files that it opens)
	memory should not crash (e.g., it should never segfault)
	memory must be written using the 'C' programming language (not C++)
	memory cannot use the following functions from the 'C' stdio.h library: fwrite, fread, variants of put (i.e., fputc, putc_unlocked, putchar, putchar_unlocked, and putw), and get (i.e., fgetc, getc, getc_unlocked, getchar, getchar_unlocked, getline, and getw).
	memory cannot use functions, like system(3) to execute external programs

Functionality: 
	The get() function works as intended; however, I did not have enough time to complete the set() function to the required specifications. I did my best attempt at set, although it does not work as intended. 
