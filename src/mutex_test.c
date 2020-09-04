#include "mutex_test.h"
#include "lsafe.h"
#include <string.h>

#define MAX_LEN 256
struct mutex_shared
{
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    int buff[MAX_LEN];
    int n_ready;
};

void producer(void *arg)
{
    struct mutex_shared *new_data = (struct mutex_shared *)(arg);
    lpthread_mutex_lock(&(new_data->mutex));
    int i = 0;
    for(;i < MAX_LEN && new_data->buff[i] != -1;i ++)
        ;
        
    if(i != MAX_LEN)
    {
        new_data->buff[i] = 1;
        new_data->n_ready ++;
        printf("produce %d into buffer!\n", i);
    }
    
    lpthread_mutex_unlock(&(new_data->mutex));
}

void comsumer(void *arg)
{
    struct mutex_shared *new_data = (struct mutex_shared *)(arg);
    lpthread_mutex_lock(&(new_data->mutex));
    int i = 0;
    for(;i < MAX_LEN && new_data->buff[i] == -1;i ++)
        ;
        
    if(i != MAX_LEN)
    {
        new_data->buff[i] = -1;
        printf("consume %d into buffer!\n", i);
    }
    
    lpthread_mutex_unlock(&(new_data->mutex));
}


void pro_com_test(int argc, char **argv)
{
    const int produce_no = 10;
    const int consume_no = 10;
    pthread_t con_ths[consume_no];
    pthread_t pro_ths[produce_no];
    
    struct mutex_shared data;
    data.n_ready = 0;
    
    lpthread_mutex_init(&data.mutex, NULL);
    memset(data.buff, -1, MAX_LEN);
    
    //创建线程
    for(int i = 0;i < produce_no;i ++)
    {
        lpthread_create(&pro_ths[i], NULL, producer, &data);
    }
    
    for(int i = 0;i < consume_no;i ++)
    {
        lpthread_create(&con_ths[i], NULL, comsumer, &data);
    }
    
    //等待
    for(int i = 0;i < produce_no;i ++)
    {
        lpthread_join(pro_ths[i], NULL);
    }
    
    for(int i = 0;i < consume_no;i ++)
    {
        lpthread_join(con_ths[i], NULL);
    }
}


void cond_producer(void *arg)
{
    for(;;)
    {
        struct mutex_shared *new_data = (struct mutex_shared *)(arg);
        lpthread_mutex_lock(&(new_data->mutex));
        int i = 0;
        for(;i < MAX_LEN && new_data->buff[i] != -1;i ++)
            ;
            
        if(i != MAX_LEN)
        {
            new_data->buff[i] = 1;
            if(new_data->n_ready == 0)
            {
                lpthread_cond_signal(&new_data->cond);
            }
            
            new_data->n_ready ++;
            printf("produce %d into buffer!\n", i);
        }
        
        lpthread_mutex_unlock(&(new_data->mutex));
    }
}

void cond_comsumer(void *arg)
{
    struct mutex_shared *new_data = (struct mutex_shared *)(arg);
    lpthread_mutex_lock(&(new_data->mutex));
    int i = 0;
    for(;i < MAX_LEN && new_data->buff[i] == -1;i ++)
        ;
        
    if(i != MAX_LEN)
    {
        if(new_data->n_ready == 0)
        {
            lpthread_cond_wait(&new_data->cond, &new_data->mutex);
        }
        
        new_data->n_ready--;
        new_data->buff[i] = -1;
        printf("consume %d into buffer!\n", i);
    }
    
    lpthread_mutex_unlock(&(new_data->mutex));
}

void pro_cond_test(int argc, char **argv)
{
    const int produce_no = 10;
    const int consume_no = 10;
    pthread_t con_ths[consume_no];
    pthread_t pro_ths[produce_no];
    
    struct mutex_shared data;
    data.n_ready = 0;
    
    lpthread_mutex_init(&data.mutex, NULL);
    lpthread_cond_init(&data.cond, NULL);
    memset(data.buff, -1, MAX_LEN);
    
    //创建线程
    for(int i = 0;i < produce_no;i ++)
    {
        lpthread_create(&pro_ths[i], NULL, cond_producer, &data);
    }
    
    for(int i = 0;i < consume_no;i ++)
    {
        lpthread_create(&con_ths[i], NULL, cond_comsumer, &data);
    }
    
    //等待
    for(int i = 0;i < produce_no;i ++)
    {
        lpthread_join(pro_ths[i], NULL);
    }
    
    for(int i = 0;i < consume_no;i ++)
    {
        lpthread_join(con_ths[i], NULL);
    }
}

int mutex_test(int argc, char **argv)
{
    pro_cond_test(argc, argv);
}

