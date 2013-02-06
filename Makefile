all: clean install
install: server
server: server.o sock.o epoll.o 
	gcc server.o sock.o epoll.o -o server
sock.o: sock.c sock.h 
	gcc -c sock.c
epoll.o: epoll.c epoll.h 
	gcc -c epoll.c 
clean:
	rm *.o server