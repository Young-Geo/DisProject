#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include <libmemcached/memcached.h> 

void *do_search(void *arg) 
{

    char *key = "key2";
    uint32_t flags = 0;
    size_t value_length = 0;

    memcached_return rc;

    memcached_st *memc = (memcached_st *)arg;

    sleep(2); 

    while (1) {
        char *value = memcached_get(memc, key, strlen(key),\
                &value_length, &flags, &rc); 

        if (rc == MEMCACHED_SUCCESS) {
            printf("key:%s\tvalue:%s\tvalue_lenth:%ld\n", \
                    key, value, value_length); 
        }
        else
            printf("unserach or error\n");

        printf("search rc: %s\n", memcached_strerror(memc, rc));
        sleep(5); 
    }
}

int main(int argc, char *argv[]) 
{
    //表示与memcached通讯的句柄
    memcached_st *memc; 
    //表示调用API返回的类型
    memcached_return rc; 

    //memcached client 需要连接的server句柄
    memcached_server_st *servers;

    //创建一个memcached句柄
    memc = memcached_create(NULL);

    //由于是分布式的 要创建多个server
    //创建一个server1
    servers = memcached_server_list_append(NULL, (char *)"127.0.0.1", 11322, &rc);

    //创建一个server2
    servers = memcached_server_list_append(servers, (char *)"127.0.0.1", 11222, &rc);

    //将创建好的server列表 添加到memcache句柄中
    rc = memcached_server_push(memc, servers); 

    //server就不需要了
    memcached_server_free(servers);

    // 分布 一致性
    //保证两个server的数据是互相备份的 一致的。
    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_DISTRIBUTION, MEMCACHED_DISTRIBUTION_CONSISTENT);
    //设置一个服务器 最长多长时间没有 返回数据
    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_RETRY_TIMEOUT, 20);
    // 设置如果一个server出现问题，就将此server从mem句柄中拆除
    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_REMOVE_FAILED_SERVERS, 1);
    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_SERVER_FAILURE_LIMIT, 5); 
    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_AUTO_EJECT_HOSTS, true);

    pthread_t tid;
    pthread_create(&tid, NULL, do_search, (void *)memc);

    int time_s1 = 0;
    int times = 0;
    const char *keys[] = {"key1", "key2", "key3", "key4"}; 
    const size_t key_length[] = {4, 4, 4, 4};
    char *values[] = {"first", "second", "third", "forth"}; 
    size_t val_length[] = {5, 6, 5, 5};
    int i;

    while (times++ < 100000) {
        for (i = 0; i < 4; i++) {
            rc = memcached_set(memc, keys[i], key_length[i], 
                    values[i], val_length[i], 
                    (time_t)180, (uint32_t)0);

            printf("key: %s rc: %s\n", keys[i], memcached_strerror(memc, rc)); 
        }

        printf("time : %d\n", time_s1++);
        sleep(1); 
    }

    memcached_free(memc);

    return 0; 
}
