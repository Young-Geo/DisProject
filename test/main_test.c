#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "redis_op.h"

int main(int argc, char *argv[])
{

    redisContext* conn = NULL;
    char *ip = "127.0.0.1";
    char *prot = "6379";
    int ret = 0;
    int i = 0;
    char buf[1024];
    char value[1024][128];
    int len = 0;
    conn = rop_connectdb_nopwd(ip, prot);
    if (conn == NULL) {
        printf("error \n");
        return -1;
    }

    ret = rop_redis_setvalue(conn, "wode", "nide");
    ret = rop_redis_getvalue(conn, "wode", buf);
    printf("%s\n", buf);
    ret = rop_hash_setvalue(conn, "myhash", "key3", "value3");
    ret = rop_hash_getvalue(conn, "myhash", "key3", buf);
    printf("%s\n", buf);
    ret = rop_hash_getall(conn, "myhash",value, &len);
    for (i = 0; i < len; ++i) {
        printf("%s\n", value[i]);
    }
    rop_disconnect(conn);
	return ret;
}
