#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

int main() {
	char buf[1024];
	pid_t pid;
	int status = 0;
	printf("%% ");
	while (fgets(buf, 1024, stdin) != 0) {
		if (buf[strlen(buf)-1] == '\n')
			buf[strlen(buf)-1] = 0;
		if ((pid = fork()) == 0) {
			execlp(buf, buf, (char*)0);
			printf("couldn't execute: %s \n",buf);
			exit(127);
		}
		if ((pid=waitpid(pid, &status, 0)) < 0)
			printf("waitpid error\n");
		printf("%% ");
	}
	return 0;
}