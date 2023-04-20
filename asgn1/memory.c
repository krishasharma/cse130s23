#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> // for open()
#include <unistd.h> // for read() and write()
#include <errno.h> 

#define BUFFER_SIZE 1024 
#define MAX_FILENAME_SIZE 255


int get(char *filename) {
	// opening the file for reading
	int filedes = open(filename, O_RDONLY);
	if (filedes == -1) {
		// error opening the file
		return 1;
	}

	// read the contents of the file into the buffer
	char buffer[BUFFER_SIZE];
	int nread = read(filedes, buffer, BUFFER_SIZE);
	if (nread == -1) {
		// error reading from the file 
		close(filedes);
		return 1;
	}

	// write the contents of the file to stdout 
	int nwritten = write(STDOUT_FILENO, buffer, nread);
	if (nwritten == -1) {
		// error writing to stdout 
		close(filedes);
		return 1;
	}

	// close the file 
	close(filedes);

	return 0;
}


int set(char *filename, char *content) {
	// open the file for writing 
	int filedes = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 644);
	if (filedes == -1) {
		// error opening the file 
		fprintf(stderr, "Operation Failed\n");
		return 1;
	}

	// write the contents to the file 
	int nwritten = write(filedes, content, strlen(content));
	if (nwritten == -1) {
		// error writing to the file 
		close(filedes);
		return 1;
	}

	// close the file 
	close(filedes);

	return 0;
}


int main(int argc, char **argv) {
	if (argc < 3 || argc > 4) {
		fprintf(stderr, "Invalid Command\n");
		printf("test1");
		return 1;
	}
	 
	if (strcmp(argv[1], "get") != 0) {
		fprintf(stderr, "Ivalid Command\n");
		printf("test2");
		return 1;
	} 
	else if (strcmp(argv[1], "get") == 0) {
		get(argv[2]);
		return 0; 
	}
		
	if (strcmp(argv[1], "set") != 0) {
		fprintf(stderr, "Invalid Command\n");
		printf("test3");
		return 1;
	}
	else if (strcmp(argv[1], "set") == 0) {
		set(argv[2], argv[3]);
		return 0;
	}

	return 0;
}

