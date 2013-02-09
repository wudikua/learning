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

void socket_recv(int sockfd, struct User* user){
    char buffer[512];
    int flag = 1;
    int recv_bytes = 0;
    printf("recv data\r\n");
    int first = 1;
    while(flag){
        recv_bytes = recv(sockfd, buffer, sizeof(buffer), 0);
        if(recv_bytes<=0){
            printf("recv field\r\n");
            close(sockfd);
            return;
        }else{
            buffer[recv_bytes] = '\0';
            if(first){
                char temp2[64];
                int i,j;
                char* temp;
                temp = strstr(buffer,"callback=");
                if(temp != NULL){
                    bzero(&temp2,sizeof(temp2));
                    i=0;
                    j=0;
                    while(temp[i]!='\n' && temp[i] !='\r' && temp[i] !='\0' && i<64){
                        if(temp[i]=='='){
                            i++;
                            while(temp[i]!='&'&&temp[i]!='\0'&&temp[i]!=' ' && i<64){
                                temp2[j++] = temp[i++];
                            }
                            break;
                        }
                        i++;
                    }
                    temp2[j]='\0';
                    if(user[sockfd].callback != NULL){
                        free(user[sockfd].callback);
                    }
                    user[sockfd].callback = strdup(temp2);    
                }
                
                
                temp = strstr(buffer,"cmd=");
                if(temp != NULL){
                    bzero(&temp2,sizeof(temp2));
                    i=0;
                    j=0;
                    while(temp[i]!='\n' && temp[i] !='\r' && temp[i] !='\0' && i<64){
                        if(temp[i]=='='){
                            i++;
                            while(temp[i]!='&'&&temp[i]!='\0'&&temp[i]!=' ' && i<64){
                                temp2[j++] = temp[i++];
                            }
                            break;
                        }
                        i++;
                    }
                    temp2[j]='\0';
                    if(user[sockfd].cmd != NULL){
                        free(user[sockfd].cmd);
                    }
                    user[sockfd].cmd = strdup(temp2);
                }
                
            }
            
            //printf("%s", buffer);
            bzero(&buffer,sizeof(buffer));
        }
        if(recv_bytes==sizeof(buffer)){
            flag = 1;
        }else{
            flag = 0;
        }
        first=0;
    }
}

void socket_send(int sockfd, struct User* user){
    char resp[1024] = "HTTP/1.1 200 OK\r\nServer: kua\r\nContent-Type:text/html;charset=UTF-8\r\n\r\n";
    int ret = -1;
    char body[1024];
    sprintf(body, "%s(\"IP:%s PORT:%d cmd:%s resp:%s\")", user[sockfd].callback, user[sockfd].ip, user[sockfd].port, user[sockfd].cmd, "helloworld");
    strcat(resp, body);
    ret = send(sockfd, &resp, strlen(resp), 0);
    if(ret<0){
        printf("send field\r\n");
        close(sockfd);
        return;
    }
}