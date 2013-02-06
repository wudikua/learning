#include "sock.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

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