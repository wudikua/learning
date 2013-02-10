#define PORT 8080
#define MAX_CONNS 128

struct User{
    int sockfd;
    int port;
    char* ip;
    char* callback;
    char* cmd;
    int in_use;
};

int create();
void bind2sock(int server_sockfd,int port);
void listening(int server_sockfd);
int socket_recv(int sockfd, struct User* user);
void socket_send(int sockfd, struct User* user);