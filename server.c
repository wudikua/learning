#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

int main(){
    int server_sockfd, client_sockfd;
    int ret,addr_len;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    
    //create
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_sockfd<0){
        printf("socket field\n");
        return -1;
    }
     
    //bind
    bzero(&server_addr, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = 
        htonl(INADDR_ANY);//htonl 把32位值从主机字节序转换成网络字节序
    server_addr.sin_port = htons(8080);
    ret = bind(server_sockfd, (struct sockaddr*)(&server_addr), 
        sizeof(struct sockaddr));//htons 把16位值从主机字节序转换成网络字节序
    if(ret<0){
        printf("bind field\n");
        return -1;
    }
    
    //listen
    ret = listen(server_sockfd, 128);
    if(ret<0){
        printf("listen field\n");
        return -1;
    }

    //accept loop
    while(1){
        addr_len = sizeof(struct sockaddr);
        ret = accept(server_sockfd, (struct sockaddr*)(&client_addr), &addr_len);
        printf("accept ok!\r\nServer start get connect from %s : %s\r\n",ntohl(client_addr.sin_addr.s_addr),ntohs(client_addr.sin_port));
    }
    return 0;
}
