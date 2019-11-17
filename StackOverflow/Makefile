build ?= release

srcdir		= .

CC		= gcc
CPP		= g++

CFLAGS		= -c -fno-stack-protector -z execstack -D_FORTIFY_SOURCE=0
ifeq "$(build)" "debug"
	CFLAGS += -g 
endif

ifeq "$(build)" "debug"
	LDFLAGS		= 
else
	LDFLAGS		= 
endif
LIBS		= -lpthread


StackOverflow: StackOverflow.o
	$(CC) -o $@  $<

StackOverflow.o: StackOverflow.c
	$(CC) -o $@  $(CFLAGS) $(SHARED_CFLAGS) $<


clean:
	rm -rf ./StackOverflow ./StackOverflow.o
