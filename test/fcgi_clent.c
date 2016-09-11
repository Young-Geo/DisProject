#include "fcgi_config.h"

#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/wait.h>

#include "fcgi_stdio.h"
#include "util_cgi.h"

#include "redis_op.h"

int         upload_storage(char *filename, char *file_id, int file_id_len)
{
    int pfd[2];
    pid_t pid;

    if (pipe(pfd) < 0) {
        //LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "[errror], pipe error");
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
        execlp("fdfs_upload_file", "fdfs_upload_file", "./conf/client.conf", filename, NULL);
        //LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "exec fdfs_upload_file error");
    }

    else {
        //parent
        //关闭写端
        close(pfd[1]);

        wait(NULL);

        //从管道中去读数据
        file_id_len = read(pfd[0], file_id, file_id_len);
        file_id[strlen(file_id) - 1] = '\0';
        printf("<h4>%s</h4>", file_id);

        // LOG(FDFS_TEST_MUDULE, FDFS_TEST_PROC, "file_id = [%s]", file_id);
    }
    return 0;
}

int         save_redis(char *ip, char *prot, char *key, char *value)
{
    redisContext* conn = NULL;
    int ret = 0;
    conn = rop_connectdb_nopwd(ip, prot);
    if (conn == NULL) {
        ret = -1;
        printf("error \n");
        return ret;
    }

    ret = rop_redis_setvalue(conn, key, value);
    rop_disconnect(conn);
    return ret;
}


int main ()
{
    char *file_buf = NULL;
    char boundary[256] = {0};
    char content_text[256] = {0};
    char filename[256] = {0};
    char fdfs_file_path[256] = {0};
    char fdfs_file_stat_buf[256] = {0};
    char fdfs_file_host_name[30] = {0};
    char fdfs_file_url[512] = {0};
    //char *redis_value_buf = NULL;
    //time_t now;;
    //char create_time[25];
    //char suffix[10];



    while (FCGI_Accept() >= 0) {
        char *contentLength = getenv("CONTENT_LENGTH");
        int len;

        printf("Content-type: text/html\r\n"
                "\r\n");

        if (contentLength != NULL) {
            len = strtol(contentLength, NULL, 10);
        }
        else {
            len = 0;
        }

        if (len <= 0) {
            printf("No data from standard input\n");
        }
        else {
            int i, ch;
            int fd = 0;
            char *begin = NULL;
            char *end = NULL;
            char *p, *q, *k;
            char file_id[2048] = {0};

            //==========> 开辟存放文件的 内存 <===========

            file_buf = malloc(len);
            if (file_buf == NULL) {
                printf("malloc error! file size is to big!!!!\n");
                return -1;
            }

            begin = file_buf;
            p = begin;
            for (i = 0; i < len; i++) {
                if ((ch = getchar()) < 0) {
                    printf("Error: Not enough bytes received on standard input<p>\n");
                    break;
                }
                //putchar(ch);
                *p = ch;
                p++;
            }

            //===========> 开始处理前端发送过来的post数据格式 <============
            //begin deal
            end = p;

            p = begin;

            //get boundary
            p = strstr(begin, "\r\n");
            if (p == NULL) {
                printf("wrong no boundary!\n");
                goto END;
            }

            strncpy(boundary, begin, p-begin);
            boundary[p-begin] = '\0';
            //printf("boundary: [%s]\n", boundary);

            p+=2;//\r\n
            //已经处理了p-begin的长度
            len -= (p-begin);

            //get content text head
            begin = p;

            p = strstr(begin, "\r\n");
            if(p == NULL) {
                printf("ERROR: get context text error, no filename?\n");
                goto END;
            }
            strncpy(content_text, begin, p-begin);
            content_text[p-begin] = '\0';
            //printf("content_text: [%s]\n", content_text);

            p+=2;//\r\n
            len -= (p-begin);

            //get filename
            // filename="123123.png"
            //           ↑
            q = begin;
            q = strstr(begin, "filename=");

            q+=strlen("filename=");
            q++;

            k = strchr(q, '"');
            strncpy(filename, q, k-q);
            filename[k-q] = '\0';

            trim_space(filename);
            //printf("filename: [%s]\n", filename);

            //get file
            begin = p;     
            p = strstr(begin, "\r\n");
            p+=4;//\r\n\r\n
            len -= (p-begin);

            begin = p;
            // now begin -->file's begin
            //find file's end
            p = memstr(begin, len, boundary);
            if (p == NULL) {
                p = end-2;    //\r\n
            }
            else {
                p = p -2;//\r\n
            }

            //begin---> file_len = (p-begin)

            //=====> 此时begin-->p两个指针的区间就是post的文件二进制数据
            //======>将数据写入文件中,其中文件名也是从post数据解析得来  <===========
            fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
            write(fd, begin, p - begin + 1);
            close(fd);
            upload_storage(filename, file_id, sizeof(file_id));

            unlink(filename);
            save_redis("127.0.0.1", "6379", filename, file_id);



            //===============> 将该文件存入fastDFS中,并得到文件的file_id <============

            //================ > 得到文件所存放storage的host_name <=================
END:

            memset(boundary, 0, 256);
            memset(content_text, 0, 256);
            memset(filename, 0, 256);
            memset(fdfs_file_path, 0, 256);
            memset(fdfs_file_stat_buf, 0, 256);
            memset(fdfs_file_host_name, 0, 30);
            memset(fdfs_file_url, 0, 512);

            free(file_buf);
            //printf("date: %s\r\n", getenv("QUERY_STRING"));
        }
    } /* while */

    return 0;
}
