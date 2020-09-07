#include <pthread.h>

void cond_producer(void *arg);
void cond_comsumer(void *arg);
void pro_cond_test(int argc, char **argv);

void producer(void *arg);
void comsumer(void *arg);
void pro_com_test(int argc, char **argv);

/*
//读写锁测试

void rwlock_thread1(void *arg);
void rwlock_thread2(void *arg);
void rwlock_test();
*/
//记录锁测试
void lock_file(int fd, int flag);
void unlock_file(int fd, int flag);
void fcntl_test();

//记录上锁，锁优先级测试
//当当前文件占有一个写锁时是否允许读锁存在
void rd_wr_test();
//等待中的读锁和写锁哪个优先级高
void rd_wr_test2();

int mutex_test(int argc, char **argv);