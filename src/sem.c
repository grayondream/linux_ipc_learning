#include "sem.h"
#include <time.h>
#include <math.h>
#include "lsafe.h"
#include <stdlib.h>
#include <pthread.h>

#define BUFF_SIZE 5
typedef struct named_share
{
    char buff[BUFF_SIZE];
    sem_t *lock;
    sem_t *nempty;
    sem_t *nstored;
    int items;
}named_share;

void* named_produce(void *arg)
{
    named_share *ptr = arg;
    for(int i = 0;i < ptr->items;i++)
    {
        lsem_wait(ptr->nempty);
        lsem_wait(ptr->lock);   //锁

        //生产物品
        ptr->buff[i % BUFF_SIZE] = rand() % 1000;
        printf("produce %04d into %2d\n", ptr->buff[i % BUFF_SIZE], i % BUFF_SIZE);
        sleep(rand()%2);
        lsem_post(ptr->lock);   //解锁
        lsem_post(ptr->nstored);
    }

    return NULL;
}

void* named_consume(void *arg)
{
    named_share *ptr = arg;
    for(int i = 0;i < ptr->items;i++)
    {
        lsem_wait(ptr->nstored);
        lsem_wait(ptr->lock);   //锁

        //生产物品
        printf("consume %04d at   %2d\n", ptr->buff[i % BUFF_SIZE], i % BUFF_SIZE);
        ptr->buff[i % BUFF_SIZE] = -1;
        sleep(rand()%2);
        lsem_post(ptr->lock);   //解锁
        lsem_post(ptr->nempty);
        //iterator
    }

    return NULL;
}

void named_sem_test()
{
    char *nempty_name = "nempty6";
    char *nstored_name = "nstored6";
    char *lock_name = "lock6";
    
    int items = 10;
    int flag = O_CREAT | O_EXCL;

    named_share arg;
    srand(time(NULL));
    arg.items = items;
    memset(arg.buff, -1, sizeof(int) * BUFF_SIZE);
    arg.nempty = lsem_open(lpx_ipc_name(nempty_name), flag, FILE_MODE, BUFF_SIZE);
    arg.nstored = lsem_open(lpx_ipc_name(nstored_name), flag, FILE_MODE, 0);
    arg.lock = lsem_open(lpx_ipc_name(lock_name), flag, FILE_MODE, 1);
    
    pthread_t pid1, pid2;
    
    int val = 0;
    lsem_getvalue(arg.nstored, &val);
    lsem_getvalue(arg.nempty, &val);

    pthread_setconcurrency(2); 
    lpthread_create(&pid1, NULL, named_produce, &arg);
    lpthread_create(&pid2, NULL, named_consume, &arg);

    lpthread_join(pid1, NULL);
    lpthread_join(pid2, NULL);

    lsem_unlink(lpx_ipc_name(nempty_name));
    lsem_unlink(lpx_ipc_name(nstored_name));
    lsem_unlink(lpx_ipc_name(lock_name));
}

void unnamed_sem_test()
{
    int items = 10;

    named_share arg;
    srand(time(NULL));
    arg.items = items;
    memset(arg.buff, -1, sizeof(int) * BUFF_SIZE);

    arg.lock = (sem_t*)malloc(sizeof(sem_t));
    arg.nempty = (sem_t*)malloc(sizeof(sem_t));
    arg.nstored = (sem_t*)malloc(sizeof(sem_t));
    
    lsem_init(arg.lock, 0, 1);
    lsem_init(arg.nempty, 0, BUFF_SIZE);
    lsem_init(arg.nstored, 0, 0);
    
    pthread_t pid1, pid2;


    pthread_setconcurrency(2); 
    lpthread_create(&pid1, NULL, named_produce, &arg);
    lpthread_create(&pid2, NULL, named_consume, &arg);

    lpthread_join(pid1, NULL);
    lpthread_join(pid2, NULL);

    lsem_destroy(arg.lock);
    lsem_destroy(arg.nempty);
    lsem_destroy(arg.nstored);

    SAFE_RELEASE(arg.lock);
    SAFE_RELEASE(arg.nempty);
    SAFE_RELEASE(arg.nstored);
}

int sem_test()
{
    //named_sem_test();
    unnamed_sem_test();
}