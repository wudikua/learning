#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "epoll.h"
#include "sock.h"

static volatile sig_atomic_t shutdown_flag = 0;

void signal_handler(int sig){
    printf("server shutdown\r\n");
    shutdown_flag = 1;
}

int main(){
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    int server_sockfd, client_sockfd;
    int ret,addr_len = sizeof(struct sockaddr);
    struct sockaddr_in client_addr;
    struct epoll_event events[EPOLL_SIZE];
    char* responce_msg = "HTTP/1.1 200 OK\r\nContent-Type:text/html;charset=UTF-8\r\n\r\nhello world\r\n";
    //create
    server_sockfd = create();
 
    //bind
    bind2sock(server_sockfd, PORT); 
    
    //listen
    listening(server_sockfd); 
    
    int efd = epoll_init(EPOLL_SIZE);

    epoll_prepare_fd(server_sockfd);
    epoll_add(efd, server_sockfd);
    
    //accept loop
    while(!shutdown_flag){
        int nfds = epoll_wait(efd, events, EPOLL_SIZE, -1);
        printf("epoll triggered %d fds\r\n",nfds);
        int i=0;
        for(i=0;i<nfds;i++){
            if(events[i].data.fd==server_sockfd){
                while(1){
                    client_sockfd = accept(server_sockfd, (struct sockaddr*)(&client_addr), &addr_len);
                    if(client_sockfd<0){
                        break;
                    }
                    printf("accept ok!\r\nServer start get connect from %s : %d\r\n",(char*)inet_ntoa(client_addr.sin_addr),client_addr.sin_port);
                    epoll_prepare_fd(client_sockfd); 
                    epoll_add(efd, client_sockfd);    
                }
            }else if(events[i].events==EPOLLIN){
                printf("epoll in \r\n");
                client_sockfd = events[i].data.fd;
                socket_recv(client_sockfd);
                epoll_set(efd, client_sockfd, EPOLLOUT);
            }else if(events[i].events==EPOLLOUT){
                printf("epoll out \r\n");
                client_sockfd = events[i].data.fd;
                socket_send(client_sockfd, responce_msg);
                epoll_del(efd, client_sockfd);
                close(client_sockfd);
            }else{
                epoll_del(efd, events[i].data.fd);
                close(events[i].data.fd);
            }
        }
    }
    close(efd);
    close(server_sockfd);
    return 0;
}
