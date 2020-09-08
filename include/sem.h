#ifndef __SEM_H__
#define __SEM_H__

#include <semaphore.h>

void* named_produce(void *ar);
void* named_consume(void *ar);
void named_sem_test();
void unnamed_sem_test();
int sem_test();

#endif