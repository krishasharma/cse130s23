
CC = clang 
CFLAGS = -Wall -Wextra -Wstrict-prototypes -Werror -pedantic -g
HERA = memory.o 
ZEUS = --leak-check=full --show-leak-kinds=all

.PHONY: all scan-build valgrind 

all: memory

memory: $(HERA) memory.o
	$(CC) $(CFLAGS) -c memory $(HERA)

memory.o: memory.c
	$(CC) $(CFLAGS) -c memory.c

clean: 
	rm -f *.o memory 

format: 
	clang-format -i -style=file *.[ch]
