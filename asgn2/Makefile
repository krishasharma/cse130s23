CC = clang
CFLAGS = -Wall -Wpedantic -Werror -Wextra
REMOVE = rm -f

ALL_SOURCE = $(wildcard *.c)
COMMON_SOURCE = ${shell grep -L "int main(" ${ALL_SOURCE}}
COMMON_OBJECT = $(COMMON_SOURCE:%.c=%.o)
EXECS = httpserver

.PHONY: all clean debug GDB

all: $(EXECS)

clean:
	$(REMOVE) $(EXECS) *.o

GDB: CFLAGS += -g
GDB: clean all

$(EXECS): %: %.o $(COMMON_OBJECT)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

cleanScript:
	$(REMOVE) *.txt 
       
