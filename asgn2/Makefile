CC = clang 
CFLAGS = -Wall -Wpedantic -Werror -Wextra -g  
.PHONY: all valgrind 

all: httpserver 

httpserver: httpserver.o
	$(CC) -o httpserver httpserver.o 

httpserver.o: httpserver.c
	$(CC) $(CFLAGS) -c httpserver.c

clean: 
	rm -f httpserver *.o 

format: 
	clang-format -i -style=file httpserver.c

