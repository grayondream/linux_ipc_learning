#include "mutex_test.h"
#include "lsafe.h"
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>

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

/*
pthread_t pid1, pid2;

//执行半途中取消另一个线程的运行
void rwlock_thread1(void *arg)
{
    lpthread_rwlock_t *rw = arg;
    printf("trying to get a read lock\n");
    lpthread_rwlock_rdlock(rw);
    printf("thread 1 got a read lock\n");
    //取消线程2， cond_wait可能出现不一致
    sleep(3);
    printf("trying cancel the thread2\n");
    pthread_cancel(pid2);
    printf("canceled the thread2\n");
    sleep(3);
    lpthread_rwlock_unlock(rw);
}

void rwlock_thread2(void *arg)
{
    lpthread_rwlock_t *rw = arg;
    printf("trying to get a write lock\n");
    lpthread_rwlock_wrlock(rw);
    printf("thread 1 got a write lock\n");
    sleep(1);
    lpthread_rwlock_unlock(rw);
}

void rwlock_test()
{
    pthread_setconcurrency(2);

    lpthread_rwlock_t lock;
    lpthread_rwlock_init(&lock, NULL);
    pthread_create(&pid1, NULL, rwlock_thread1, &lock);         //创建第一个线程
    sleep(1);
    pthread_create(&pid2, NULL, rwlock_thread2, &lock);         //创建第一个线程

    void *status;
    pthread_join(pid2, &status);
    if(status != PTHREAD_CANCELED)
    {
        printf("thread 2 status  %p\n", status);
    }

    pthread_join(pid1, &status);
    if(status != NULL)
    {
        printf("thread 2 status  %p\n", status);
    }

    printf("rwlock: refercount=%d, wait_readers=%d, wait_writers=%d\n", lock.refercount, lock.wait_readers, lock.wait_writers);
    lpthread_rwlock_destroy(&lock);
    return 0;
}

*/
void lock_file(int fd, int flag)
{
    if(!flag) return;
    lfcntl_wr_lockw(fd, 0, SEEK_SET, 0);
}

void unlock_file(int fd, int flag)
{
    if(!flag) return;
    lfcntl_unlock(fd, 0, SEEK_SET, 0);
}

void fcntl_test()
{
    char file[] = "/home/grayondream/altas/ipc/build/tmp";
    pid_t pid = getpid();
    int fd = open(file, O_RDWR, FILE_MODE);
    char line[MAX_LEN] = {0};
    int n = 0;
    int flag = 1;
    for(int i = 0;i < 20;i ++)
    {
        lock_file(fd, flag);
        sleep(1);
        lseek(fd, 0, SEEK_SET);
        int len = lread(fd, line, MAX_LEN);
        line[len] = '\0';
        len = sscanf(line, "%d\n", &n);
        printf("pid = %d, no = %d\n", pid, n);
        n++;

        sprintf(line, "%d\n", n);
        lseek(fd, 0, SEEK_SET);
        lwrite(fd, line, MAX_LEN);

        unlock_file(fd, flag);
    }

}

void rd_wr_test()
{
    char file[] = "./1";
    int fd = open(file, O_RDWR, FILE_MODE);
    printf("%s : 父进程尝试拥有1个读锁\n", lget_time());
    lfcntl_rd_lockw(fd, 0, SEEK_SET, 0);
    printf("%s : 父进程拥有1个读锁\n", lget_time());
    pid_t pid1;
    pid_t pid2 = lfork();
    if(pid2 == 0)
    {
        sleep(1);
        printf("%s : 子进程1尝试拥有1个写锁\n", lget_time());
        lfcntl_wr_lockw(fd, 0, SEEK_SET, 0);
        printf("%s : 子进程1拥有1个写锁\n", lget_time());
        sleep(2);
        printf("%s : 子进程1释放了1个写锁\n", lget_time());
        lfcntl_unlock(fd, 0, SEEK_SET, 0);
        _exit(0);
    }

    pid1 = fork();
    if(pid1 == 0)
    {
        sleep(3);
        printf("%s : 子进程2尝试拥有1个读锁\n", lget_time());
        lfcntl_rd_lockw(fd, 0, SEEK_SET, 0);
        printf("%s : 子进程2拥有1个读锁\n", lget_time());
        sleep(4);
        printf("%s : 子进程2释放了1个读锁\n", lget_time());
        lfcntl_unlock(fd, 0, SEEK_SET, 0);
        _exit(0);
    }

    sleep(5);
    printf("%s : 父进程释放了1个读锁\n", lget_time());
    lfcntl_unlock(fd, 0, SEEK_SET, 0);
    waitpid(pid1, 0, 0);
    waitpid(pid2, 0, 0);
    return;
}

void rd_wr_test2()
{
    char file[] = "./1";
    int fd = open(file, O_RDWR, FILE_MODE);
    printf("%s : 父进程尝试拥有1个写锁\n", lget_time());
    lfcntl_wr_lockw(fd, 0, SEEK_SET, 0);
    printf("%s : 父进程拥有1个写锁\n", lget_time());
    pid_t pid1;
    pid_t pid2 = lfork();
    if(pid2 == 0)
    {
        sleep(1);
        printf("%s : 子进程1尝试拥有1个写锁\n", lget_time());
        lfcntl_wr_lockw(fd, 0, SEEK_SET, 0);
        printf("%s : 子进程1拥有1个写锁\n", lget_time());
        sleep(1);
        printf("%s : 子进程1释放了1个写锁\n", lget_time());
        lfcntl_unlock(fd, 0, SEEK_SET, 0);
        _exit(0);
    }

    pid1 = fork();
    if(pid1 == 0)
    {
        sleep(0);
        printf("%s : 子进程2尝试拥有1个读锁\n", lget_time());
        lfcntl_rd_lockw(fd, 0, SEEK_SET, 0);
        printf("%s : 子进程2拥有1个读锁\n", lget_time());
        sleep(1);
        printf("%s : 子进程2释放了1个读锁\n", lget_time());
        lfcntl_unlock(fd, 0, SEEK_SET, 0);
        _exit(0);
    }

    sleep(3);
    printf("%s : 父进程释放了1个写锁\n", lget_time());
    lfcntl_unlock(fd, 0, SEEK_SET, 0);
    waitpid(pid1, 0, 0);
    waitpid(pid2, 0, 0);
    return;
}

int mutex_test(int argc, char **argv)
{
    //pro_cond_test(argc, argv);
    //rwlock_test();
    //fcntl_test();
    rd_wr_test2();
}

