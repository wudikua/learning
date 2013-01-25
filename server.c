#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

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

int main(){
    int server_sockfd, client_sockfd;
    int ret,addr_len;
    struct sockaddr_in client_addr;
    char buffer[1024];
    //create
    server_sockfd = create();
 
    //bind
    bind2sock(server_sockfd, PORT); 
    
    //listen
    listening(server_sockfd); 

    //accept loop
    while(1){
        addr_len = sizeof(struct sockaddr);
        client_sockfd = accept(server_sockfd, (struct sockaddr*)(&client_addr), &addr_len);
        printf("accept ok!\r\nServer start get connect from %s : %d\r\n",inet_ntoa(client_addr.sin_addr),client_addr.sin_port);
        socket_recv(client_sockfd); 
        char* msg = "HTTP/1.1 200 OK\r\nContent-Type:text/html;charset=UTF-8\r\n\r\nhello world\r\n";
        socket_send(client_sockfd, msg);
        close(client_sockfd);
    }
    return 0;
}
