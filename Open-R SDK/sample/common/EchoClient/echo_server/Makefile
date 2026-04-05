#
# Copyright 2002 Sony Corporation 
#
# Permission to use, copy, modify, and redistribute this software for
# non-commercial use is hereby granted.
#
# This software is provided "as is" without warranty of any kind,
# either expressed or implied, including but not limited to the
# implied warranties of fitness for a particular purpose.
#

CC=gcc
CFLAGS=
LIBS=

all: echo_server

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

echo_server: echo_server.o
	$(CC) -o $@ echo_server.o $(LIBS)

clean:
	rm -f echo_server.exe echo_server *.o
