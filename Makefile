CC = gcc 

CFLAGS= -Wall -g -O2

.PHONY:		all clean spotless

all:		tunner

clean:
		rm -f tunner

spotless:	clean
