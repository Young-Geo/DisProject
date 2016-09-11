/*
 * echo.c --
 *
 *	Produce a page containing all FastCGI inputs
 *
 *
 * Copyright (c) 1996 Open Market, Inc.
 *
 * See the file "LICENSE.TERMS" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */
#ifndef lint
static const char rcsid[] = "$Id: echo.c,v 1.5 1999/07/28 00:29:37 roberts Exp $";
#endif /* not lint */

#include "fcgi_config.h"

#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef _WIN32
#include <process.h>
#else
extern char **environ;
#endif

#include "fcgi_stdio.h"

#include <string.h>

#include <sys/wait.h>

#include "util_cgi.h"

#include "redis_op.h"

#include "../include/make_log.h"



#include <stdio.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>
#include "cJSON.h"





static void PrintEnv(char *label, char **envp)
{
    printf("%s:<br>\n<pre>\n", label);
    for ( ; *envp != NULL; envp++) {
        printf("%s\n", *envp);
    }
    printf("</pre><p>\n");
}

int     read_str_list(const char *str, char *file_id, char *url, char *filename, char *c_time, char *user, char *type)
{
    //file_id||url||filename||create time||user||type
    int ret = 0;
    char *pstr = str;
    char *estr = NULL;
    int n = 0;
    estr = strstr(pstr, "||");
    if (estr == NULL) {
        ret = -1;
        printf("error \n");
        return ret;
    }
    n = estr-pstr;
    strncpy(file_id, pstr, n);
    file_id[n] = '\0';

    estr += 2;
    pstr = estr;
    estr = strstr(pstr, "||");
    if (estr == NULL) {
        ret = -1;
        printf("error \n");
        return ret;
    }
    n = estr-pstr;
    strncpy(url, pstr, estr-pstr);
    url[n] = '\0';

    estr += 2;
    pstr = estr;
    estr = strstr(pstr, "||");
    if (estr == NULL) {
        ret = -1;
        printf("error \n");
        return ret;
    }
    n = estr-pstr;
    strncpy(filename, pstr, estr-pstr);
    filename[n] = '\0';

    estr += 2;
    pstr = estr;
    estr = strstr(pstr, "||");
    if (estr == NULL) {
        ret = -1;
        printf("error \n");
        return ret;
    }
    n = estr-pstr;
    strncpy(c_time, pstr, estr-pstr);
    c_time[n] = '\0';

    estr += 2;
    pstr = estr;
    estr = strstr(pstr, "||");
    if (estr == NULL) {
        ret = -1;
        printf("error \n");
        return ret;
    }
    n = estr-pstr;
    strncpy(user, pstr, estr-pstr);
    user[n] = '\0';

    estr += 2;
    pstr = estr;
    strcpy(type, pstr);
    return ret;
}


int     en_josn(cJSON *arr, char *file_id, char *url, char *filename, char *c_time, char *user, char *type, int count)
{
    int ret = 0;
    cJSON *ar = NULL;
    ar = cJSON_CreateObject();
    if (ar == NULL) {
        ret = -1;
        printf("root == NULL\n");
        return ret;
    }
    char png[1024] = { 0 };
    /*
     * "id": "group1/M00/00/00/wKgCbFepT0SAOBCtACjizOQy1fU405.rar",
     * "kind": 2,
     * "title_m": "阶段测试_STL_数据结构.rar",
     * "title_s": "文件title_s",
     * "descrip": "2016-08-09 11:34:23",
     * "picurl_m": "http://172.16.0.148/static/file_png/rar.png",
     * "url": "http://192.168.2.108/group1/M00/00/00/wKgCbFepT0SAOBC
     * tACjizOQy1fU405.rar",
     * "pv": 1,
     * "hot": 0
     *
     * */
    cJSON_AddItemToArray(arr, ar);


    sprintf(png, "http://192.168.122.1/static/file_png/%s.png", type);
    cJSON_AddStringToObject(ar, "id", file_id);
    cJSON_AddNumberToObject(ar, "kind", 2);
    cJSON_AddStringToObject(ar, "title_m", filename);
    cJSON_AddStringToObject(ar, "title_s", filename);
    cJSON_AddStringToObject(ar, "descrip", c_time);
    cJSON_AddStringToObject(ar, "picurl_m", png);
    cJSON_AddStringToObject(ar, "url", url);
    cJSON_AddNumberToObject(ar, "pv", count);
    cJSON_AddNumberToObject(ar, "hot", 0);

    return ret;
}

int     load_file(int start, int count)
{
    int ret = 0;
    int num = 0, i;
    char value[10][1024];
    redisContext* conn = NULL;
    char file_id[512], url[512], filename[512], c_time[128], user[128], type[128];
    char temp[512];
    cJSON *root = NULL, *arr = NULL;
    char *root_str = NULL;
    int cnt = 0;
    root=cJSON_CreateObject();
    if (root == NULL) {
        ret = -1;
        printf("root == NULL\n");
        return ret;
    }

    arr = cJSON_CreateArray();
    if (arr == NULL) {
        ret = -1;
        printf("arr == NULL\n");
        return ret;
    }
    cJSON_AddItemToObject(root, "games", arr);

    conn = rop_connectdb_nopwd("127.0.0.1", "6379");
    if (conn == NULL) {
        ret = -1;
        printf("error \n");
        return ret;
    }
    ret = rop_range_list(conn, "FILE_INFO_LIST", start, count, value, &num);
    if (ret != 0) {
        return ret;
    }
    for (i = 0; i < num; ++i) {
        ret = read_str_list(value[i], file_id, url, filename, c_time, user, type);
        strcpy(temp, file_id);
        str_replace(temp, "/", "%2F");
        cnt = rop_zset_get_score(conn, "sort", temp);
        if (cnt == -1)
            cnt = 0;
        ret = en_josn(arr, file_id, url, filename, c_time, user, type, cnt);
    }
    root_str = cJSON_Print(root);
    printf("%s\n", root_str);


    cJSON_Delete(root);
    free(root_str);
    rop_disconnect(conn);
    return ret;
}

int down_count(char *file_id)
{
    int ret = 0;
    redisContext* conn = NULL;

    conn = rop_connectdb_nopwd("127.0.0.1", "6379");
    if (conn == NULL) {
        ret = -1;
        printf("error \n");
        return ret;
    }

    ret = rop_zset_increment(conn, "sort", file_id);
    if (ret < 0) {
        printf("error \n");
        return ret;
    }
    return ret;
}

int main ()
{
    char **initialEnv = environ;
    int count = 0;

    while (FCGI_Accept() >= 0) {

        printf("Content-type: text/html\r\n");
        printf("\r\n");
                    /*
        printf("Content-type: text/html\r\n"
                "\r\n"
                "<title>FastCGI echo</title>"
                "<h1>FastCGI echo</h1>\n"
                "Request number %d,  Process ID: %d<p>\n", ++count, getpid());
                */


        /*char *contentLength = getenv("CONTENT_LENGTH");
          int len;
          LOG("DATA", "data", "you");

          printf("Content-type: text/html\r\n"
          "\r\n"
          "<title>FastCGI echo</title>"
          "<h1>FastCGI echo</h1>\n"
          "Request number %d,  Process ID: %d<p>\n", ++count, getpid());

          if (contentLength != NULL) {
          len = strtol(contentLength, NULL, 10);
          }
          else {
          len = 0;
          }

          if (len <= 0) {
          printf("No data from standard input.<p>\n");
          }
          else {



          }
          */

        ///data?cmd=newFile&fromId=0&count=8&user=
        char *str = getenv("QUERY_STRING");
        char *cmd = NULL;
        char *end = NULL;
        cmd = strstr(str, "cmd=");
        if (cmd == NULL)
            continue;
        cmd += 4;
        if (strncmp(cmd, "newFile", 7) == 0) {
            char *str_id = NULL;
            char *str_count = NULL;
            int id = 0;
            int count = 0;
            str_id = strstr(cmd, "fromId=");
            if (str_id == NULL)
                continue;
            str_id += 7;
            id = atoi(str_id);
            str_count = strstr(cmd, "count=");
            if (str_count == NULL)
                continue;
            str_count += 6;
            count = atoi(str_count);
            load_file(id, count);
        } else if (strncmp(cmd, "increase", 8) == 0) {
            //op_zset_increment(redisContext *conn, char* key, char* member);
            //int rop_zset_get_score(redisContext *conn, char *key, char *member);
            //fileId=group1/M00/00/00/wKh6AVfToaGAVE0wAAAMvP0NmIA60006.c
            char file_id[1024] = { 0 };
            char *str = strstr(cmd, "fileId=");
            if (str == NULL)
                continue;
            str += 7;
            strcpy(file_id, str);
            down_count(file_id);
        }
    }
    return 0;
}
