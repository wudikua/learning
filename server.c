#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>

#define PORT 8080
#define MAX_CONNS 128

int create(){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd<0){
        printf("socket field\n");
        exit(-1);
    }
    return sockfd;
}

void bind2sock(int server_sockfd,int port){
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = 
        htonl(INADDR_ANY);//htonl 把32位值从主机字节序转换成网络字节序
    server_addr.sin_port = htons(port);
    int ret = bind(server_sockfd, (struct sockaddr*)(&server_addr), 
        sizeof(struct sockaddr));//htons 把16位值从主机字节序转换成网络字节序
    if(ret<0){
        printf("bind field\n");
        exit(-1);
    }
 
}

void listening(int server_sockfd){
    int ret = listen(server_sockfd, MAX_CONNS);
    if(ret<0){
        printf("listen field\n");
        exit(-1);
    }
}

void socket_recv(int sockfd){
    char buffer[512];
    int flag = 1;
    int recv_bytes = 0;
    printf("recv data\r\n");
    while(flag){
        recv_bytes = recv(sockfd, buffer, sizeof(buffer), 0);
        if(recv_bytes<=0){
            printf("recv field\r\n");
            close(sockfd);
            return;
        }else{
            buffer[recv_bytes] = '\0';
            printf("%s", buffer);
            bzero(&buffer,sizeof(buffer));
        }
        if(recv_bytes==sizeof(buffer)){
            flag = 1;
        }else{
            flag = 0;
        }
    }
    return;
}

void socket_send(int sockfd, const char* msg){
    int ret = send(sockfd, msg, strlen(msg), 0);
    if(ret<0){
        printf("send field\r\n");
        close(sockfd);
        return;
    }
}

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

int main(){
    int server_sockfd, client_sockfd;
    int ret,addr_len;
    struct sockaddr_in client_addr;
    struct epoll_event events[MAX_CONNS];
    char* responce_msg = "HTTP/1.1 200 OK\r\nContent-Type:text/html;charset=UTF-8\r\n\r\nhello world\r\n";
    //create
    server_sockfd = create();
 
    //bind
    bind2sock(server_sockfd, PORT); 
    
    //listen
    listening(server_sockfd); 
    
    int efd = epoll_init(MAX_CONNS);

    epoll_prepare_fd(server_sockfd);
    epoll_add(efd, server_sockfd);
    
    //accept loop
    while(1){
        int nfds = epoll_wait(efd, events, MAX_CONNS, -1);
        printf("epoll triggered\r\n");
        int i=0;
        for(i=0;i<nfds;i++){
            if(events[i].data.fd==server_sockfd){
                addr_len = sizeof(struct sockaddr);
                client_sockfd = accept(server_sockfd, (struct sockaddr*)(&client_addr), &addr_len);
                printf("accept ok!\r\nServer start get connect from %s : %d\r\n",(char*)inet_ntoa(client_addr.sin_addr),client_addr.sin_port);
                epoll_prepare_fd(client_sockfd); 
                epoll_add(efd, client_sockfd);
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
    free(events);
    return 0;
}
