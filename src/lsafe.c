
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <wait.h>
#include <mqueue.h>
#include "lsafe.h"

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

mqd_t lmq_open(char *name, int flag, int mode, struct mq_attr *attr)
{
    mqd_t ret = mq_open(name, flag, mode, attr);
    ERROR_CHECK(ret, ==, -1, name, "create msg queue %s failed!");
    return ret;
}

void lmq_close(mqd_t mq)
{
    int ret = mq_close(mq);
    ERROR_CHECK(ret, <, 0, mq, "close msg queue %d failed!");
}

void lmq_unlink(char *name)
{
    int ret = mq_unlink(name);
    ERROR_CHECK(ret, <, 0, name, "unlink msg queue %s failed!");
}

void lmq_getattr(mqd_t mq, struct mq_attr *attr)
{
    int ret = mq_getattr(mq, attr);
    ERROR_CHECK(ret, < , 0, mq, "get the attribution from %d failed!");
}

void lmq_setattr(mqd_t mq, const struct mq_attr *attr, struct mq_attr *oattr)
{
    int ret = mq_setattr(mq, attr, oattr);
    ERROR_CHECK(ret, <, 0, mq, "set the attribution into %d failed!");
}

void lmq_send_msg(mqd_t mq, const char *ptr, int len, int prior)
{
    int ret = mq_send(mq, ptr, len, prior);
    ERROR_CHECK(ret, <, 0, mq, "send message into %d failed!");
}

int lmq_receive_msg(mqd_t mq, char *ptr, int len, int *prior)
{
    int ret = mq_receive(mq, ptr, len, prior);
    ERROR_CHECK(ret, <, 0, mq, "receive message from %d failed!");
    return ret;
}