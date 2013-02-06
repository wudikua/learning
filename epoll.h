#define EPOLL_SIZE 128

void epoll_prepare_fd(int sock);
int epoll_init(int size);
int epoll_add(int efd, int fd);
int epoll_set(int efd, int fd, int event_code);
int epoll_del(int efd, int fd);