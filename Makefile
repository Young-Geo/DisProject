
CC=gcc
CPPFLAGS= -I./include -I/usr/local/include/hiredis/
CFLAGS=-Wall 
LIBS=-lhiredis -lpthread  -lfcgi -lm

#找到当前目录下所有的.c文件
src = $(wildcard ./test/*.c) $(wildcard ./src/*.c)

#将当前目录下所有的.c  转换成.o给obj
obj = $(patsubst %.c, %.o, $(src))

upload=test/upload
redis_test=test/redis_test
fdfs_test=test/fdfs_test
main_test=test/main_test
data=test/data

target=$(fdfs_test) $(redis_test) $(main_test) $(upload) $(data)


ALL:$(target)


#生成所有的.o文件
$(obj):%.o:%.c
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS) 


#fdfs_test程序
$(fdfs_test): test/fdfs_client_test.o src/make_log.o
	$(CC) $^ -o $@ $(LIBS)

#redis_test程序
$(redis_test): test/test_redis_api.o src/make_log.o
	$(CC) $^ -o $@ $(LIBS)

$(main_test): test/main_test.o src/make_log.o test/redis_op.o
	$(CC) $^ -o $@ $(LIBS)

$(upload): test/upload.o src/make_log.o test/util_cgi.o test/redis_op.o
	$(CC) $^ -o $@ $(LIBS)

$(data): test/data.o src/make_log.o test/util_cgi.o test/redis_op.o test/cJSON.o
	$(CC) $^ -o $@ $(LIBS)

#clean指令

clean:
	-rm -rf $(obj) $(target)

distclean:
	-rm -rf $(obj) $(target)

#将clean目标 改成一个虚拟符号
.PHONY: clean ALL distclean
