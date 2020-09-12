#ifndef __SEM_H__
#define __SEM_H__

#include <semaphore.h>
#include <sys/sem.h>

void* named_produce(void *ar);
void* named_consume(void *ar);
void named_sem_test();
void unnamed_sem_test();

void* multp_singc_produce(void *arg);
void* multp_singc_consume(void *arg);
void multp_singc_test();

void* multp_multc_produce(void *arg);
void* multp_multc_consume(void *arg);
void multp_multc_test();

void *write_buff(void *arg);
void *read_buff(void *arg);
void read_write_test();

union semun
{
    int val;                  /* value for SETVAL-用于设置信号量集中信号量的计数值*/
    struct semid_ds *buf;     /* buffer for IPC_STAT, IPC_SET */
    unsigned short *array;    /* array for GETALL, SETALL */
    /* Linux specific part: */
    struct seminfo *__buf;    /* buffer for IPC_INFO */
};

int vsem_create(key_t key);
int vsem_open(key_t key);
int vsem_p(int semid);
int vsem_v(int semid);
int vsem_d(int semid);
int vsem_setval(int semid, int val);
int vsem_getval(int semid);
int vsem_getmode(int semid);
int vsem_setmode(int semid, char *mode);
void v_test(int argc, char **argv);

int sem_test(int argc, char **argv);

#endif