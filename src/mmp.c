#include "mmp.h"
#include <sys/mman.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/wait.h>

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

void mmp_test(int argc, char **argv)
{
    //mmap_test(argc, argv);
    mmap_size_test(argc, argv);
}