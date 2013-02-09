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
    struct User user[EPOLL_SIZE];
    char* responce_msg = "HTTP/1.1 200 OK\r\nServer: kua\r\nContent-Type:text/html;charset=UTF-8\r\n\r\n{\"name\":\"helloworld\"}";
    bzero(&user,sizeof(user));
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
        int i=0;
        for(i=0;i<nfds;i++){
            if(events[i].data.fd==server_sockfd){
                while(1){
                    client_sockfd = accept(server_sockfd, (struct sockaddr*)(&client_addr), &addr_len);
                    if(client_sockfd<0){
                        break;
                    }
                    // printf("accept ok!\r\nServer start get connect from %s : %d\r\n",(char*)inet_ntoa(client_addr.sin_addr),client_addr.sin_port);
                    epoll_prepare_fd(client_sockfd); 
                    epoll_add(efd, client_sockfd); 
                    user[client_sockfd].port =  client_addr.sin_port;
                    user[client_sockfd].ip =  (char*)inet_ntoa(client_addr.sin_addr);
                    user[client_sockfd].callback = NULL;
                    user[client_sockfd].cmd = NULL;
                }
            }else if(events[i].events==EPOLLIN){
                printf("epoll in \r\n");
                client_sockfd = events[i].data.fd;
                socket_recv(client_sockfd, user);
                epoll_set(efd, client_sockfd, EPOLLOUT);
            }else if(events[i].events==EPOLLOUT){
                printf("epoll out \r\n");
                client_sockfd = events[i].data.fd;
                socket_send(client_sockfd, user);
                // epoll_set(efd, client_sockfd, EPOLLIN);
                epoll_del(efd, client_sockfd);
                close(client_sockfd);
                free(user[client_sockfd].callback);
                free(user[client_sockfd].cmd);
                bzero(&user[client_sockfd],sizeof(struct User));
            }else{
                epoll_del(efd, events[i].data.fd);
                free(user[events[i].data.fd].callback);
                free(user[events[i].data.fd].cmd);
                bzero(&user[events[i].data.fd],sizeof(struct User));
                close(events[i].data.fd);
            }
        }
    }
    close(efd);
    close(server_sockfd);
    return 0;
}
