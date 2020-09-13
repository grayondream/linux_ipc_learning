#include "mmp.h"
#include <sys/mman.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/wait.h>
#include <string.h>

typedef struct mmap_shared
{
    sem_t mutex;
    int count;
}mmap_shared;

void mmap_test(int argc, char **argv)
{
    int fd = open("./mmap", O_RDWR | O_CREAT, FILE_MODE);
    mmap_shared *ptr = NULL;
    mmap_shared ele;

    srand(time((void*)0));
    lwrite(fd, &ele, sizeof(ele));
    ptr = lmmap(NULL, sizeof(ele), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    lclose(fd);
    lsem_init(&ptr->mutex, 1, 1);
    int lops = 20;
    pid_t pid = lfork();
    if(pid == 0)
    {
        for(int i = 0;i < lops;i ++)
        {
            lsem_wait(&ptr->mutex);
            printf("child  count = %3d\n", ptr->count++);
            //sleep(rand()%2);
            lsem_post(&ptr->mutex);
        }
    }
    else
    {
        for(int i = 0;i < lops;i ++)
        {
            lsem_wait(&ptr->mutex);
            printf("father count = %3d\n", ptr->count++);
            //sleep(rand()%2);
            lsem_post(&ptr->mutex);
        }
    }
    
    //waitpid(pid, 0, 0);
}

void mmap_size_test(int argc, char **argv)
{
    if(argc != 3)
        return;
    
    int filesize = atoi(argv[1]);
    int mmapsize = atoi(argv[2]);
    int fd = lopen("./mmap", O_RDWR | O_CREAT | O_TRUNC);
    lseek(fd, filesize - 1, SEEK_SET);
    lwrite(fd, " ", 1);

    char *ptr = lmmap(NULL, mmapsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    int pagesize = sysconf(_SC_PAGESIZE);       //获取页面大小
    printf("PAGE_SIZE=%d\n", pagesize);
    for(int i = 0;i < filesize > mmapsize ? filesize : mmapsize; i += pagesize)
    {
        printf("ptr[%d] = %d\n", i, ptr[i]);
        ptr[i] = 1;

        int j = i + pagesize - 1;
        printf("ptr[%d] = %d\n", j, ptr[j]);
        ptr[j] = 1;
    }
}

//服务端程序创建共享内存和信号量
void count_server(char *share_name, char *sem_name)
{
    //共享内存
    share_name = lpx_ipc_name(share_name);
    sem_name = lpx_ipc_name(sem_name);
    shm_unlink(share_name);
    int fd = lshm_open(share_name, O_RDWR | O_CREAT | O_EXCL, FILE_MODE);
    
    lftruncate(fd, sizeof(int));
    char *ptr = lmmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    lclose(fd);

    //信号量
    sem_unlink(sem_name);
    sem_t *mutex = lsem_open(sem_name, O_CREAT | O_EXCL, FILE_MODE, 1);
    lsem_close(mutex);
}

//客户端程序对共享内存区的内容进行修改
void count_client(char *share_name, char *sem_name)
{
    int fd = lshm_open(lpx_ipc_name(share_name), O_RDWR, FILE_MODE);
    char *ptr = lmmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    lclose(fd);

    sem_t *mutex = sem_open(lpx_ipc_name(sem_name), 0);

    int *count = ptr;
    pid_t pid = getpid();
    int lops = 5;
    for(int i = 0;i < lops;i ++)
    {
        lsem_wait(mutex);
        printf("pid = %d, count = %d\n", pid, (*ptr)++);
        lsem_post(mutex);
    }
}

void mmap_count_test(int argc, char **argv)
{
    if(argc != 4)
        return;

    switch (argv[1][0])
    {
    case 'c':
        count_client(argv[2], argv[3]);break;
    case 's':
        count_server(argv[2], argv[3]);break;
    default:
        break;
    }
}

#define MESSAGESIZE 256
#define MESSAGE_NO 16

typedef struct cp_share
{
    sem_t mutex;
    sem_t nempty;
    sem_t nstored;
    sem_t flowmutex;
    int i;
    long nflowing;
    long msg_off[MESSAGE_NO];
    char msg_data[MESSAGESIZE * MESSAGE_NO];
}cp_share;

//消费者
void cp_server(int argc, char **argv)
{
    if(argc != 3)
        err_exit("argc is not 3", -1);

    char *name = argv[2];
    shm_unlink(lpx_ipc_name(name));
    int fd = lshm_open(lpx_ipc_name(name), O_RDWR | O_CREAT | O_EXCL, FILE_MODE);
    cp_share *ptr = lmmap(NULL, sizeof(cp_share), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    lftruncate(fd, sizeof(cp_share));
    lclose(fd);

    //initialize the offset
    for(int i = 0;i < MESSAGE_NO;i ++)
    {
        ptr->msg_off[i] = i * MESSAGESIZE;
    }

    lsem_init(&ptr->nempty, 1, MESSAGE_NO);
    lsem_init(&ptr->nstored, 1, 0);
    lsem_init(&ptr->mutex, 1, 1);
    lsem_init(&ptr->flowmutex, 1, 1);

    int i = 0, lastflow = 0, tmp = 0;
    for(;;)
    {
        lsem_wait(&ptr->nstored);
        lsem_wait(&ptr->mutex);
        int offset = ptr->msg_off[i];
        char *data = &ptr->msg_data[offset];
        printf("消费者：i = %3d, msg=%s\n", i, data);
        i = (i + 1) % MESSAGE_NO;

        lsem_post(&ptr->mutex);
        lsem_post(&ptr->nempty);

        lsem_wait(&ptr->flowmutex);
        tmp = ptr->nflowing;
        lsem_post(&ptr->flowmutex);

        if(tmp != lastflow)
        {
            printf("noverflow = %d\n", tmp);
            lastflow = tmp;
        }
    }
}

//生产者
void cp_client(int argc, char **argv)
{
    if(argc != 5)
        err_exit("argc is not 5", -1);

    int loops = atoi(argv[3]);
    int nus = atoi(argv[4]);
    char *name = argv[2];

    int fd = lshm_open(lpx_ipc_name(name), O_RDWR, FILE_MODE);
    cp_share *ptr = lmmap(NULL, sizeof(cp_share), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    lclose(fd);

    pid_t pid = getpid();
    char msg[MESSAGESIZE];
    for(int i = 0;i < loops;i ++)
    {
        usleep(nus);

        memset(msg, 0, sizeof(char) * MESSAGESIZE);
        snprintf(msg, MESSAGESIZE, "pid = %d, message = %d", pid, i);

        int ret = sem_trywait(&ptr->nempty);
        if(ret == -1)
        {
            if(errno == EAGAIN)
            {
                lsem_wait(&ptr->flowmutex);
                ptr->nflowing ++;
                lsem_post(&ptr->flowmutex);
                continue;
            }
            else
            {
                err_exit("sem_trywait error", -1);
            }
            
        }

        lsem_wait(&ptr->mutex);
        int off = ptr->msg_off[ptr->i];
        ptr->i = (ptr->i + 1) % MESSAGE_NO;
        lsem_post(&ptr->mutex);

        strcpy(&ptr->msg_data[off], msg);       
        lsem_post(&ptr->nstored);
    }
}

void mmap_cp_test(int argc, char **argv)
{
    if(argc < 2)
        err_exit("argc < 2", -1);

    switch (argv[1][0])
    {
    case 'c':
        cp_client(argc, argv);break;
    case 's':
        cp_server(argc, argv);break;
    default:
        break;
    }
}

void mmp_test(int argc, char **argv)
{
    //mmap_test(argc, argv);
    //mmap_size_test(argc, argv);
    //mmap_count_test(argc, argv);
    mmap_cp_test(argc, argv);
}