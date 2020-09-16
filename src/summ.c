#include "summ.h"
#include <wait.h>

speed_info write_ipc(int fd, int writefd)
{
    struct stat buf;
    fstat(fd, &buf);
    int size = buf.st_size;
    char buff[MAX_LEN];
    int n = 0;
    speed_info info = {0, 0};
    lwrite(writefd, &size, sizeof(size));
    while((n = lread(fd, buff, MAX_LEN)) > 0)
    {
        int start = clock();
        lwrite(writefd, buff, n);
        int end = clock();
        info.time += (end - start);
        info.bytes += n;
    }
    
    return info;
}

speed_info read_ipc(int readfd)
{
    char buff[MAX_LEN];
    int n = 0;
    speed_info info = {0, 0};
    int size = 0;
    lread(readfd, &size, sizeof(size));
    while(size > info.bytes)
    {
        int start = clock();
        if((n = lread(readfd, buff, MAX_LEN)) <= 0)
        {
            break;
        }

        int end = clock();
        info.time += (end - start);
        info.bytes += n;
    }
    
    return info;
}

void pipe_performance_test(int argc, char **argv)
{
    int fd1[2];
    lpipe(fd1);
    int fd = lopen("/home/grayondream/altas/ipc/build/core", O_RDONLY);

    pid_t pid = lfork();
    if(pid == 0)
    {
        speed_info info = write_ipc(fd, fd1[1]);
        printf("write n = %9d, time = %9d mbs=%.3f\n", info.bytes, info.time, (info.bytes * 1.0 / (1024 * 1024)) / (info.time * 1.0 / CLOCKS_PER_SEC));
    }
    else
    {
        speed_info info = read_ipc(fd1[0]);
        printf("read  n = %9d, time = %9d mbs=%.3f\n", info.bytes, info.time, (info.bytes * 1.0 / (1024 * 1024)) / (info.time * 1.0 / CLOCKS_PER_SEC));
    }
    
    waitpid(pid, NULL, 0);
}

void summary_test(int argc, char **argv)
{
    pipe_performance_test(argc, argv);
}