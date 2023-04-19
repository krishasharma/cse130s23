EXECBIN = memory
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES: %.c=%.o)
FORMATS = $(SOURCES:%.c=%fmt)

CC = clang
FORMAT = clang-format
CFLAGS = -Wall -Wextra -Wstrict-prototypes -Werror -pedantic

.PHONY: all clean format 

all: $(EXECBIN) 

$(EXECBIN): $(OBJECTS)
	$(CC) -o $@ $^

%.o : %.c 
	$(CC) $(CFLAGS) -c $<

clean: 
	rm -f memory *.o

format: $(FORMATS)

%.fmt: %.c
	$(FORMAT) -i $<
	touch $@
