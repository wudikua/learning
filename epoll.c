#include "epoll.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <fcntl.h>

void epoll_prepare_fd(int sock) {
    int opts = fcntl(sock, F_GETFL, 0);
    opts |= O_NONBLOCK;
    if(fcntl(sock, F_SETFL, opts) < 0) {
        printf("set nonblocking error\r\n");
    }
}

int epoll_init(int size){
    return epoll_create(size);
}

int epoll_add(int efd, int fd){
    struct epoll_event ev;
    ev.events = EPOLLIN|EPOLLET|EPOLLERR|EPOLLHUP;
    ev.data.fd = fd;
    if(epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev)<0){
        printf("epoll add error\r\n");
        return -1;
    }
    return 0;
}

int epoll_set(int efd, int fd, int event_code){
    struct epoll_event ev;
    ev.events = event_code|EPOLLERR|EPOLLHUP;
    ev.data.fd = fd;
    if(epoll_ctl(efd, EPOLL_CTL_MOD, fd, &ev)<0){
        printf("epoll set error\r\n");
        return -1;
    }
    return 0;
}

int epoll_del(int efd, int fd){
    if(fd<0)
        return -1;
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = -1;
    if(epoll_ctl(efd, EPOLL_CTL_DEL, fd, &ev)<0){
        printf("epoll del error\r\n");
        return -1;
    }
    return 0;
}