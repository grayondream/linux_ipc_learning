#ifndef __SEM_H__
#define __SEM_H__

#include <semaphore.h>

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

int sem_test();

#endif