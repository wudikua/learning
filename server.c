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
static int efd;

void signal_handler(int sig){
    printf("server shutdown\r\n");
    shutdown_flag = 1;
}

void notify_all(struct User* user){
    int i=0;
    for(i=0;i<EPOLL_SIZE;i++){
        if(user[i].in_use){
            socket_send(i, user);
            epoll_del(efd, i);
            close(i);
            free(user[i].callback);
            free(user[i].cmd);
            bzero(&user[i],sizeof(struct User));    
        }
    }
}

int main(){
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    int server_sockfd, client_sockfd;
    int ret,addr_len = sizeof(struct sockaddr);
    struct sockaddr_in client_addr;
    struct epoll_event events[EPOLL_SIZE];
    struct User user[EPOLL_SIZE];
    
    bzero(&user,sizeof(user));
    //create
    server_sockfd = create();
 
    //bind
    bind2sock(server_sockfd, PORT); 
    
    //listen
    listening(server_sockfd); 
    
    efd = epoll_init(EPOLL_SIZE);

    epoll_prepare_fd(server_sockfd);
    epoll_add(efd, server_sockfd);
    int nfds,i=0;
    //accept loop
    while(!shutdown_flag){
        nfds = epoll_wait(efd, events, EPOLL_SIZE, -1);
        for(i=0;i<nfds;i++){
            // printf("triggered %d fds\r\n",nfds);
            if(events[i].data.fd==server_sockfd){
                while(1){
                    client_sockfd = accept(server_sockfd, (struct sockaddr*)(&client_addr), &addr_len);
                    if(client_sockfd<0){
                        break;
                    }
                    epoll_prepare_fd(client_sockfd); 
                    epoll_add(efd, client_sockfd); 
                    user[client_sockfd].sockfd = client_sockfd;
                    user[client_sockfd].port =  client_addr.sin_port;
                    user[client_sockfd].ip =  (char*)inet_ntoa(client_addr.sin_addr);
                    user[client_sockfd].callback = NULL;
                    user[client_sockfd].cmd = NULL;
                    user[client_sockfd].in_use = 1;
                }
            }else if(events[i].events==EPOLLIN){
                // printf("epoll in \r\n");
                client_sockfd = events[i].data.fd;
                if(socket_recv(client_sockfd, user) < 0){
                    printf("close when epoll in\r\n");
                    epoll_del(efd, client_sockfd);
                    close(client_sockfd);
                    bzero(&user[client_sockfd],sizeof(struct User));       
                }else{
                    epoll_set(efd, client_sockfd, EPOLLOUT);  
                }
            }else if(events[i].events==EPOLLOUT){
                // printf("epoll out \r\n");
                client_sockfd = events[i].data.fd;
                if(user[client_sockfd].cmd && strstr(user[client_sockfd].cmd,"notify")){
                    printf("brocast msg\r\n");
                    notify_all(user);
                }else{
                    epoll_set(efd, client_sockfd, EPOLLIN);    
                }
            }else{
                printf("other case \r\n");
                close(events[i].data.fd);
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
