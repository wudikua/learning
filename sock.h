#define PORT 8080
#define MAX_CONNS 128

int create();
void bind2sock(int server_sockfd,int port);
void listening(int server_sockfd);
void socket_recv(int sockfd);
void socket_send(int sockfd, const char* msg);