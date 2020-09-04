#include <pthread.h>

void cond_producer(void *arg);
void cond_comsumer(void *arg);
void pro_cond_test(int argc, char **argv);

void producer(void *arg);
void comsumer(void *arg);
void pro_com_test(int argc, char **argv);
int mutex_test(int argc, char **argv);