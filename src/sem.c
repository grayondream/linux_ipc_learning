#include "sem.h"
#include <time.h>
#include <math.h>
#include "lsafe.h"
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

#define THREAD_SIZE 3
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

typedef struct multp_singc_share
{
    int i;
    sem_t nempty;
    sem_t nstored;
    sem_t lock;
    int items;
    char buff[BUFF_SIZE];
}multp_singc_share;

void* multp_singc_produce(void *arg)
{
    multp_singc_share *ptr = arg;
    for(;;)
    {
        lsem_wait(&(ptr->nempty));
        lsem_wait(&(ptr->lock));
        if(ptr->i >= ptr->items)
        {
            lsem_post(&(ptr->lock));
            lsem_post(&(ptr->nempty));
            return NULL;
        }
        
        ptr->buff[ptr->i % BUFF_SIZE] = rand() % 100;
        printf("produce %d at %d\n", ptr->buff[ptr->i * BUFF_SIZE], ptr->i % BUFF_SIZE);
        ptr->i++;
        lsem_post(&ptr->lock);
        lsem_post(&ptr->nstored);
    }
}

void* multp_singc_consume(void *arg)
{
    multp_singc_share *ptr = arg;
    for(int i = 0;i < ptr->items;i ++)
    {
        lsem_wait(&(ptr->nstored));
        lsem_wait(&(ptr->lock));
        
        printf("consume %d at %d\n", ptr->buff[i % BUFF_SIZE], i % BUFF_SIZE);

        lsem_post(&(ptr->lock));
        lsem_post(&(ptr->nempty));
    }
}

void multp_singc_test()
{
    multp_singc_share arg;
    pthread_t pro_th[THREAD_SIZE], con_th;

    arg.items = 10;
    arg.i = 0;
    memset(arg.buff, 0, BUFF_SIZE * sizeof(int));
    lsem_init(&(arg.lock), 0, 1);
    lsem_init(&(arg.nempty), 0, BUFF_SIZE);
    lsem_init(&(arg.nstored), 0, 0);

    pthread_setconcurrency(THREAD_SIZE + 1); 
    for(int i = 0;i < THREAD_SIZE;i ++)
    {
        lpthread_create(&pro_th[i], NULL, multp_singc_produce, &arg);
    }
    
    lpthread_create(&con_th, NULL, multp_singc_consume, &arg);

    for(int i = 0;i < THREAD_SIZE;i ++)
    {
        lpthread_join(pro_th[i], NULL);
    }

    lpthread_join(con_th, NULL);

    lsem_destroy(&(arg.lock));
    lsem_destroy(&(arg.nempty));
    lsem_destroy(&(arg.nstored));
}

typedef struct multp_multc_share
{
    int pi;
    int ci;
    sem_t nempty;
    sem_t nstored;
    sem_t lock;
    int items;
    char buff[BUFF_SIZE];
}multp_multc_share;

void* multp_multc_produce(void *arg)
{
    multp_multc_share *ptr = arg;
    for(;;)
    {
        lsem_wait(&(ptr->nempty));
        lsem_wait(&(ptr->lock));
        if(ptr->pi >= ptr->items)
        {
            lsem_post(&(ptr->nstored));
            lsem_post(&(ptr->nempty));
            lsem_post(&(ptr->lock));
            return NULL;
        }
        
        ptr->buff[ptr->pi % BUFF_SIZE] = rand() % 100;
        printf("produce %d at %d\n", ptr->buff[ptr->pi * BUFF_SIZE], ptr->pi % BUFF_SIZE);
        ptr->pi++;
        lsem_post(&ptr->lock);
        lsem_post(&ptr->nstored);
    }
}

void* multp_multc_consume(void *arg)
{
    multp_multc_share *ptr = arg;
    for(;;)
    {
        lsem_wait(&(ptr->nstored));
        lsem_wait(&(ptr->lock));
        if(ptr->ci >= ptr->items)
        {
            lsem_post(&(ptr->nstored));
            lsem_post(&(ptr->lock));
            return NULL;
        }

        printf("consume %d at %d\n", ptr->buff[ptr->ci % BUFF_SIZE], ptr->ci % BUFF_SIZE);
        ptr->ci++;
        lsem_post(&(ptr->lock));
        lsem_post(&(ptr->nempty));
    }
}

void multp_multc_test()
{
    multp_multc_share arg;
    pthread_t pro_th[THREAD_SIZE], con_th[THREAD_SIZE];

    arg.items = 10;
    arg.pi = 0;
    arg.ci = 0;
    memset(arg.buff, 0, BUFF_SIZE * sizeof(int));
    lsem_init(&(arg.lock), 0, 1);
    lsem_init(&(arg.nempty), 0, BUFF_SIZE);
    lsem_init(&(arg.nstored), 0, 0);

    pthread_setconcurrency(THREAD_SIZE + 1); 
    for(int i = 0;i < THREAD_SIZE;i ++)
    {
        lpthread_create(&pro_th[i], NULL, multp_multc_produce, &arg);
    }
    
    for(int i = 0;i < THREAD_SIZE;i ++)
    {
        lpthread_create(&con_th[i], NULL, multp_multc_consume, &arg);
    }
    

    for(int i = 0;i < THREAD_SIZE;i ++)
    {
        lpthread_join(pro_th[i], NULL);
    }

    for(int i = 0;i < THREAD_SIZE;i ++)
    {
        lpthread_join(con_th[i], NULL);
    }

    lsem_destroy(&(arg.lock));
    lsem_destroy(&(arg.nempty));
    lsem_destroy(&(arg.nstored));
}

typedef struct 
{
    struct 
    {
        char buff[MAX_LEN + 1];
        int len;
    }buff[BUFF_SIZE];
    sem_t lock;
    sem_t nempty;
    sem_t nstored;
    int items;
    int readfd;
    int writefd;
}wr_share;

//将buffer中的数据写入文件
void *write_buff(void *arg)
{
    wr_share* ptr = arg;
    int i = 0;
    while(1)
    {
        lsem_wait(&ptr->lock);
        //获取当前缓冲区的操作
        lsem_post(&ptr->lock);

        lsem_wait(&ptr->nstored);
        lwrite(ptr->writefd, ptr->buff[i].buff, ptr->buff[i].len);
        lwrite(STDOUT_FILENO, ptr->buff[i].buff, ptr->buff[i].len);
        i++;
        if(i >= ptr->items)
        {
            i = 0;
        }

        lsem_post(&ptr->nempty);
    }
}

//从文件中读取数据
void *read_buff(void *arg)
{
    wr_share* ptr = arg;
    int i = 0;
    while(1)
    {
        lsem_wait(&ptr->lock);
        //获取当前缓冲区的操作
        lsem_post(&ptr->lock);

        lsem_wait(&ptr->nempty);
        int n = lread(ptr->readfd, ptr->buff[i].buff, MAX_LEN);
        ptr->buff[i].len = n;
        i++;
        if(i >= ptr->items)
        {
            i = 0;
        }

        lsem_post(&ptr->nstored);
    }
}


void read_write_test()
{
    wr_share arg;
    arg.items = BUFF_SIZE;
    #if 0
    char *readfile = "build/CMakeCache.txt";
    char *writefile = "build/mktmp";
    #else
    char *readfile = "CMakeCache.txt";
    char *writefile = "mktmp";
    #endif
    arg.readfd = lopen(readfile, O_RDONLY);
    arg.writefd = lopen(writefile, O_WRONLY | O_CREAT);
    lsem_init(&arg.lock, 0, 1);
    lsem_init(&arg.nempty, 0, arg.items);
    lsem_init(&arg.nstored, 0, 0);

    pthread_t read_th, write_th;
    lpthread_create(&read_th, 0, read_buff, &arg);
    lpthread_create(&write_th, 0, write_buff, &arg);

    lpthread_join(read_th, NULL);
    lpthread_join(write_th, NULL);

    lsem_destroy(&arg.lock);
    lsem_destroy(&arg.nempty);
    lsem_destroy(&arg.nstored);
}


int vsem_create(key_t key)
{
    int semid = lsemget(key, 1, 0666 | IPC_CREAT | IPC_EXCL);
    return semid;
}
 
int vsem_open(key_t key)
{
    int semid = lsemget(key, 0, 0);//创建一个信号量集 
    return semid;
}
 
int vsem_p(int semid)//
{
    struct sembuf sb = {0, -1, /*IPC_NOWAIT*/SEM_UNDO};//对信号量集中第一个信号进行操作，对信号量的计数值减1，
    lsemop(semid, &sb, 1);//用来进行P操作 
    return 0;
}
 
int vsem_v(int semid)
{
    struct sembuf sb = {0, 1, /*0*/SEM_UNDO};
    lsemop(semid, &sb, 1);
    return 0;
}
 
int vsem_d(int semid)
{
    int ret = semctl(semid, 0, IPC_RMID, 0);//删除一个信号量集
    return ret;
}
 
int vsem_setval(int semid, int val)
{
    union semun su;
    su.val = val;
    semctl(semid, 0, SETVAL, &su);//给信号量计数值
    printf("value updated...\n");
    return 0;
}
 
int vsem_getval(int semid)//获取信号量集中信号量的计数值
{
    int ret = semctl(semid, 0, GETVAL, 0);//返回值是信号量集中
    printf("current val is %d\n", ret);
    return ret;
}
 
int vsem_getmode(int semid)
{
    union semun su;
    struct semid_ds sem;
    su.buf = &sem;
    semctl(semid, 0, IPC_STAT, su);
    printf("current permissions is %o\n", su.buf->sem_perm.mode);
    return 0;
}
 
int vsem_setmode(int semid, char *mode)
{
    union semun su;
    struct semid_ds sem;
    su.buf = &sem;
 
    semctl(semid, 0, IPC_STAT, su);

    printf("current permissions is %o\n", su.buf->sem_perm.mode);
    sscanf(mode, "%o", (unsigned int *)&su.buf->sem_perm.mode);
    semctl(semid, 0, IPC_SET, &su);
    printf("permissions updated...\n");
 
    return 0;
}
 
void usage(void)
{
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "semtool -c\n");
    fprintf(stderr, "semtool -d\n");
    fprintf(stderr, "semtool -p\n");
    fprintf(stderr, "semtool -v\n");
    fprintf(stderr, "semtool -s <val>\n");
    fprintf(stderr, "semtool -g\n");
    fprintf(stderr, "semtool -f\n");
    fprintf(stderr, "semtool -m <mode>\n");
}
 
void v_test(int argc, char *argv[])
{
    int opt;
 
    opt = getopt(argc, argv, "cdpvs:gfm:");//解析参数
    if (opt == '?')
        exit(EXIT_FAILURE);
    if (opt == -1)
    {
        usage();
        exit(EXIT_FAILURE);
    }
 
    key_t key = ftok(".", 's');
    int semid;
    switch (opt)
    {
    case 'c'://创建信号量集
        vsem_create(key);
        break;
    case 'p'://p操作
        semid = vsem_open(key);
        vsem_p(semid);
        vsem_getval(semid);
        break;
    case 'v'://v操作
        semid = vsem_open(key);
        vsem_v(semid);
        vsem_getval(semid);
        break;
    case 'd'://删除一个信号量集
        semid = vsem_open(key);
        vsem_d(semid);
        break;
    case 's'://对信号量集中信号量设置初始计数值
        semid = vsem_open(key);
        vsem_setval(semid, atoi(optarg));
        break;
    case 'g'://获取信号量集中信号量的计数值
        semid = vsem_open(key);
        vsem_getval(semid);
        break;
    case 'f'://查看信号量集中信号量的权限
        semid = vsem_open(key);
        vsem_getmode(semid);
        break;
    case 'm'://更改权限
        semid = vsem_open(key);
        vsem_setmode(semid, argv[2]);
        break;
    }
 
    return;
}

int sem_test(int argc, char **argv)
{
    //named_sem_test();
    //unnamed_sem_test();
    //multp_singc_test();
    //multp_multc_test();
    //read_write_test();
    v_test(argc, argv);
}