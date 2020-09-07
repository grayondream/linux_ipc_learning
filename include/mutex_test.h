#include <pthread.h>

void cond_producer(void *arg);
void cond_comsumer(void *arg);
void pro_cond_test(int argc, char **argv);

void producer(void *arg);
void comsumer(void *arg);
void pro_com_test(int argc, char **argv);

//读写锁测试

void rwlock_thread1(void *arg);
void rwlock_thread2(void *arg);
void rwlock_test();

int mutex_test(int argc, char **argv);