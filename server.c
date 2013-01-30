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
    char buffer[1024];
    int recv_bytes = recv(sockfd, buffer, sizeof(buffer), 0);
    if(recv_bytes<0){
        printf("recv field");
        close(sockfd);
        return;
    }
    buffer[recv_bytes] = '\0';
    printf("recv data %d bytes\r\n%s", recv_bytes, buffer);
}

void socket_send(int sockfd, const char* msg){
    int ret = send(sockfd, msg, strlen(msg), 0);
    if(ret<0){
        printf("send field");
        close(sockfd);
        return;
    }
}

void setnonblocking(int sock) {
    int opts;
    opts = fcntl(sock, F_GETFL);
    if(opts < 0) {
	perror("fcntl(sock, GETFL)");
	exit(1);
    }
    opts = opts | O_NONBLOCK;
    if(fcntl(sock, F_SETFL, opts) < 0) {
        perror("fcntl(sock, SETFL, opts)");
        exit(1);
    }
}

int main(){
    int server_sockfd, client_sockfd;
    int ret,addr_len;
    struct sockaddr_in client_addr;
    char buffer[1024];
    struct epoll_event ev;
    struct epoll_event events[256];
    //create
    server_sockfd = create();
 
    //bind
    bind2sock(server_sockfd, PORT); 
    
    //listen
    listening(server_sockfd); 
    
    int epollfd = epoll_create(256);
    ev.events = EPOLLIN|EPOLLET;
    ev.data.fd = server_sockfd;
    setnonblocking(server_sockfd);
    epoll_ctl(epollfd, EPOLL_CTL_ADD, server_sockfd, &ev);//register socket to epoll
    
    //accept loop
    while(1){
        int nfds = epoll_wait(epollfd, events, 256, -1);
        printf("epoll triggered\r\n");
        int i=0;
	for(i=0;i<nfds;i++){
            if(events[i].data.fd==server_sockfd){
                addr_len = sizeof(struct sockaddr);
                client_sockfd = accept(server_sockfd, (struct sockaddr*)(&client_addr), &addr_len);
                printf("accept ok!\r\nServer start get connect from %s : %d\r\n",inet_ntoa(client_addr.sin_addr),client_addr.sin_port);
                setnonblocking(client_sockfd); 
		ev.data.fd = client_sockfd;
                ev.events = EPOLLIN|EPOLLOUT|EPOLLET;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, client_sockfd, &ev);//register client socket to epoll
            }else if(events[i].events==EPOLLIN){
                client_sockfd = events[i].data.fd;
                socket_recv(client_sockfd);
                ev.data.fd = client_sockfd;
		ev.events = EPOLLOUT|EPOLLET;
		epoll_ctl(epollfd, EPOLL_CTL_MOD, client_sockfd, &ev);   
            }else if(events[i].events==EPOLLOUT){
                client_sockfd = events[i].data.fd;
                char* msg = "HTTP/1.1 200 OK\r\nContent-Type:text/html;charset=UTF-8\r\n\r\nhello world\r\n";
                socket_send(client_sockfd, msg);
		ev.data.fd = client_sockfd;
		epoll_ctl(epollfd, EPOLL_CTL_DEL, client_sockfd, &ev);
		close(client_sockfd);
	    }
        }
    }
    return 0;
}
