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

all: echo_client

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

echo_client: echo_client.o
	$(CC) -o $@ echo_client.o $(LIBS)

clean:
	rm -f echo_client.exe echo_client *.o
