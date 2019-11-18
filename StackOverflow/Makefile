build ?= stackoverflownx

CC		= gcc
CPP		= g++

CFLAGS		= 
LDFLAGS = 
ifeq "$(build)" "stackoverflow"
	CFLAGS += -g
else ifeq "$(build)" "stackoverflownx"
	CFLAGS += -g
endif

LIBS		= -lpthread

all: StackOverflow StackOverflowNX StackOverflowSP

StackOverflow: StackOverflow.c
	$(CC) -o $@  $(CFLAGS) -fno-stack-protector -z execstack -D_FORTIFY_SOURCE=0 $<

StackOverflowNX: StackOverflow.c
	$(CC) -o $@  $(CFLAGS) -fno-stack-protector -D_FORTIFY_SOURCE=0 $<

StackOverflowSP: StackOverflow.c
	$(CC) -o $@  $(CFLAGS) -D_FORTIFY_SOURCE=0 $<

clean:
	rm -rf ./*.o

cleanall:
	rm -rf ./*.o ./StackOverflow ./StackOverflowNX ./StackOverflowSP

