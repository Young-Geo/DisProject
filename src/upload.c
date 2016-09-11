#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/make_log.h"

int main(int argc, char *argv[])
{
    int fd[2], ret = 0;
    pid_t pid;
    if (argc < 2) {
        printf("./a.out file\n");
        return -1;
    }
    ret = pipe(fd);
    if (ret != 0) {
        perror("pipe");
        return -1;
    }

    pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    } else if (pid == 0) {
        //child
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        execlp("fdfs_upload_file", "fdfs_upload_file", "/etc/fdfs/client.conf", argv[1], NULL);
    } else if (pid > 0) {
        //parent
        char buf[2048];
        int count = 0;
        close(fd[1]);
        count = read(fd[0], buf, sizeof(buf));
        LOG("LOG", "DemoFile", "FILE %s ID:%s", argv[1], buf);
        wait(NULL);
    }
	return 0;
}
