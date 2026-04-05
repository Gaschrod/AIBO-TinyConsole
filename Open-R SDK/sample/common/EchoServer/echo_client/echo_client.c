/*
 * Copyright 2002 Sony Corporation 
 *
 * Permission to use, copy, modify, and redistribute this software for
 * non-commercial use is hereby granted.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT     54321
#define BUFSIZE  512

main(int argc, char **argv)
{
    struct sockaddr_in  addr;
    struct hostent      *hp;
    int    fd;
    int    len;
    char   buf[BUFSIZE];
    int    ret;

    if (argc != 2){
        printf("Usage: echo_client hostname\n");
        exit(1);
    }

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    if ((hp = gethostbyname(argv[1])) == NULL) {
        perror("gethostbyname");
        exit(1);
    }

    memset((void*)&addr, 0, sizeof(addr));
    memcpy(&addr.sin_addr, hp->h_addr, hp->h_length);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        perror("connect");
        exit(1);
    }

    while (fgets(buf, BUFSIZE, stdin) != NULL) {

        int len = strlen(buf) + 1;
        
        ret = write(fd, buf, len);
        if (ret != len) {
            perror("write");
            close(fd);
            exit(1);
        }

        ret = read(fd, buf, len);
        if (ret != len) {
            perror("read");
            close(fd);
            exit(1);
        }

        buf[ret] = '\0';
        printf("%s", buf);
    }

    close(fd);
    exit(0);
}
