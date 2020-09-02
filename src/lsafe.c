#include "lsafe.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <wait.h>

void err_exit(const char *buff, int err_ret)
{
    fprintf(stderr, buff);   
    fprintf(stderr, "\n");   
    
    _exit(err_ret);
}

void safe_exit(const char *buff)
{
    err_exit(buff, 0);
}

int lread(int readfd, char *buff, int len)
{
    int ret = read(readfd, buff, len);

    ERROR_CHECK(ret, <, 0, readfd, "read data from %d failed!");
    return ret;
}

int lwrite(int writefd, char *buff, int len)
{
    int ret = write(writefd, buff, len);
    ERROR_CHECK(ret, <, 0, writefd, "write data into %d failed!");    
    return ret;
}

char* lfgets(char *buff, int len, FILE *stream)
{
    char *ret = fgets(buff, len, stream);
    ERROR_CHECK(ret, ==, NULL, (int)ret, "read data from stream failed!%d!"); 

    return ret;
}

int lopen(char *filename, int mode)
{
    int ret = open(filename, mode);
    int val = errno;
    ERROR_CHECK(ret, < , 0, filename, "can not open file %s");
    return ret;
}

void lclose(int fd)
{
    int ret = close(fd);
    ERROR_CHECK(ret, <, 0, fd, "close file %d failed!");
}

void lpipe(int *fd)
{
    int ret = pipe(fd);
    ERROR_CHECK(ret, <, 0, (int)fd, "create pipe %d failed!");
}

int lfork()
{
    int ret = fork();
    ERROR_CHECK(ret, < , 0, ret, "create new process failed!%d!")
    
    return ret;
}

void lwaitpid(int pid, int *status, int option)
{
    int ret = waitpid(pid, status, option);
    ERROR_CHECK(ret, < , 0, ret, "wait for child process failed!%d!")
}

FILE* lpopen(char *command, char *type)
{
    FILE* ret = popen(command, type);
    ERROR_CHECK(ret, ==, NULL, command, "popen command %s failed!");
    
    return ret;
}

void lpclose(FILE* fd)
{
    int ret = pclose(fd);
    ERROR_CHECK(ret, <, 0, (int)fd, "pclose %d failed!");
}

void lmkfifo(char *file, int mode)
{
    int ret = mkfifo(file, mode);
    if(ret < 0 && errno == EEXIST)
        return;
        
    ERROR_CHECK(ret, <, 0, file, "create fifo %s failed!");
}

void lunlink(char *file)
{
    int ret = unlink(file);
    int val = errno;
    ERROR_CHECK(ret, <, 0, file, "unlink %s failed!");
}