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
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT     54321
#define BUFSIZE  512

int sock = -1;
int fd = -1;

void sigint_handler()
{
    fprintf(stderr, "sigint_handler()\n");

    if (sock >= 0) close(sock);
    if (fd >= 0) close(fd);
    exit(1);
}

void accept_and_echo()
{
    struct sockaddr_in  caddr;
    int  len, ret, wsize;
    char buf[BUFSIZE];

    len = sizeof(caddr);
    fd = accept(sock, (struct sockaddr *)&caddr, &len);
    if (fd < 0) {
        perror("accept");
        close(sock);
        exit(1);
    }

    ret = read(fd, buf, BUFSIZE);
    if (ret == -1) {
        perror("read");
        close(fd); fd = -1;
        return;
    }
    wsize = ret;
    
    while (1) {

        ret = write(fd, buf, wsize);
        if (ret != wsize) {
            perror("write");
            close(fd); fd = -1;
            return;
        }

        ret = read(fd, buf, BUFSIZE);
        if (ret == -1) {
            perror("read");
            close(fd); fd = -1;
            return;
        }
        wsize = ret;
    }
}

main()
{
    int    i;
    struct sockaddr_in  saddr;

    signal(SIGINT, sigint_handler);
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    memset((void*)&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(PORT);

    if (bind(sock, (struct sockaddr *)&saddr, sizeof(saddr)) < 0){
        perror("bind");
        close(sock);
        exit(1);
    }

    if (listen(sock, 1) < 0) {
        perror("listen");
        close(sock);
        exit(1);
    }

    while (1) {
        accept_and_echo();
    }
}
