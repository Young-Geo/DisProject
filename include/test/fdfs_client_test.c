#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

#include "make_log.h"

#define FDFS_TEST_MUDULE "test"
#define FDFS_TEST_PROC   "fdfs_test"

#define FILE_ID_LEN     (256)

int main(int argc, char *argv[])
{
    char *file_name = NULL;
    char file_id[FILE_ID_LEN] = {0};
    pid_t pid;

    if (argc < 2) {
        printf("usage ./a.out file-name");
        exit(0);
    }

    file_name = argv[1];



    int pfd[2];

    if (pipe(pfd) < 0) {
        LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "[errror], pipe error");
        exit(1);
    }

    pid = fork();

    if (pid == 0) {
        //chlid
        //关闭读端
        close(pfd[0]);

        //将标准输出 重定向到管道中
        dup2(pfd[1], STDOUT_FILENO);

        //exec
        execlp("fdfs_upload_file", "fdfs_upload_file", "./conf/client.conf", file_name, NULL);
        LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "exec fdfs_upload_file error");
    }

    else {
        //parent
        //关闭写端
        close(pfd[1]);

        wait(NULL);

        //从管道中去读数据
        read(pfd[0], file_id, FILE_ID_LEN);


        LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "file_id = [%s]", file_id);
    }


    

	return 0;
}
