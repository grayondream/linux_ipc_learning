#include "mypipe.h"
#include "lsafe.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>


void pipe_client(int readfd, int writefd)
{
    char buff[MAX_LEN] = {0};
    lfgets(buff, MAX_LEN, stdin);
    int len = strlen(buff);
    if(buff[len - 1] == '\n')
        len--;
        
    lwrite(writefd, buff, len);
    while((len = lread(readfd, buff, MAX_LEN) > 0))
    {
        //lwrite(stdout, buff, len);
        printf(buff);
    }
}

/*
 * @brief   客户端发送过来的是一个路径名，服务端尝试打开该文件并将文件中的数据写入管道发送给客户端
 */
void pipe_server(int readfd, int writefd)
{
    char buff[MAX_LEN] = {0};
    int ret = 0;
    //int ret = lread(readfd, buff, MAX_LEN);
    ret = lread(readfd, buff, MAX_LEN);
    if(ret == 0)
    {
        safe_exit("come into the end of stream!\n");
    }
    
    buff[ret] = '\0';
    int fd = lopen(buff, O_RDONLY);
    while((ret = lread(fd, buff, MAX_LEN)) > 0)
    {
        lwrite(writefd, buff, ret);
    }
    
    lclose(fd);
}

void pipe_test()
{
    int fd1[2] = {0};        //0 write 1 read
    int fd2[2] = {0};
    pid_t pid;
    
    lpipe(fd1);
    lpipe(fd2);
    pid = lfork();
    if(pid == 0)  //子进程
    {
        lclose(fd1[1]);
        lclose(fd2[0]);
        pipe_server(fd1[0], fd2[1]);
        safe_exit(NULL);
    }
    else
    {
        lclose(fd1[0]);
        lclose(fd2[1]);
        pipe_client(fd2[0], fd1[1]);
        lwaitpid(pid, NULL, 0);
    }
}

/*
 * popen test 
 */
void popen_test()
{
    char buff[MAX_LEN] = {0};
    lfgets(buff, MAX_LEN, stdin);
    int fd = lpopen(buff, "r");
    while(lfgets(buff, MAX_LEN, fd) != NULL)
    {
        printf(buff);
    }
    
}

/*
 * mkfifo test
 */
 
void mkfifo_test()
{
    char *file1 = "./father";
    char *file2 = "./child";
    int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
    
    lmkfifo(file1, mode);
    lmkfifo(file2, mode);
    
    pid_t id = fork();
    if(id == 0) //子进程
    {
        int read_fd = lopen(file1, O_RDONLY);
        int write_fd = lopen(file2, O_WRONLY);
        pipe_server(read_fd, write_fd);
        
        lclose(read_fd);
        lclose(write_fd);
        safe_exit(NULL);
    }
    else
    {
        int write_fd = lopen(file1, O_WRONLY);
        int read_fd = lopen(file2, O_RDONLY);
        pipe_client(read_fd, write_fd);
        lwaitpid(id, NULL, 0);
        
        lclose(read_fd);
        lclose(write_fd);
        
        lunlink(file1);
        lunlink(file2);
        safe_exit(NULL);
    }
}

void mkfifo_server_process()
{
    char *file1 = "./father";
    char *file2 = "./child";
    int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
    
    lmkfifo(file1, mode);
    lmkfifo(file2, mode);
    
    int read_fd = lopen(file1, O_RDONLY);
    int write_fd = lopen(file2, O_WRONLY);
    pipe_server(read_fd, write_fd);
    
    lclose(read_fd);
    lclose(write_fd);
    safe_exit(NULL);
}

void mkfifo_client_process()
{
    char *file1 = "./father";
    char *file2 = "./child";
    int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
    
    lmkfifo(file1, mode);
    lmkfifo(file2, mode);
    
    int write_fd = lopen(file1, O_WRONLY);
    int read_fd = lopen(file2, O_RDONLY);
    pipe_client(read_fd, write_fd);
    
    lclose(read_fd);
    lclose(write_fd);
    
    lunlink(file1);
    lunlink(file2);
    safe_exit(NULL);
    
}

#define MULT_SERVER_NAME "/home/grayondream/altas/ipc/build/tmp/server"
#define MULT_CLIENT_NAME "/home/grayondream/altas/ipc/build/tmp/client.%d"

//server 通过发送id + ' ' + '文件名'的格式向指定的client发送请求，id取值范围为01-20占两个字节
void mult_fifo_server_process()
{
    int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
    lmkfifo(MULT_SERVER_NAME, mode);
    int server_rfd = lopen(MULT_SERVER_NAME, O_RDONLY);
    while(1)
    {
        char buff[MAX_LEN] = {0};
        lfgets(buff, MAX_LEN, stdin);
        //将文件名发送给指定的客户端
        int len = strlen(buff);
        if(buff[len - 1] == '\n')
        len--;
        
        buff[len] = '\0';
        buff[2] = '\0';
        int id = atoi(buff);
        char *ptr = buff + 3;
        
        //打开客户端的fifo
        char client[MAX_LEN] = {0};
        snprintf(client, MAX_LEN, MULT_CLIENT_NAME, id);
        lmkfifo(client, mode);
        int client_wfd = lopen(client, O_WRONLY);
        
        //将数据写入客户端的fifo
        lwrite(client_wfd, ptr, strlen(ptr));
        lunlink(client);
        
        //等待客户端回传，状态
        while(lread(server_rfd, buff, MAX_LEN) > 0)
            printf(buff);
            
        printf("\n");
    }
    
    lunlink(server_rfd);
}

void mult_fifo_client_process()
{
    int id = 1;
    int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP ;
    lmkfifo(MULT_SERVER_NAME, mode);
    int server_wfd = lopen(MULT_SERVER_NAME, O_WRONLY);
    
    char client_name[MAX_LEN] = {0};
    snprintf(client_name, MAX_LEN, MULT_CLIENT_NAME, id);
    lmkfifo(client_name, mode);
    int client_rfd = lopen(client_name, O_RDONLY);
    
    char buff[MAX_LEN] = {0};
    int len = lread(client_rfd, buff, MAX_LEN);
    if(buff[len - 1] == '\n')
        len--;
        
    buff[len] = '\0';
    int fd = lopen(buff, O_RDONLY);
    while((len = lread(fd, buff, MAX_LEN)) > 0)
    {
        printf(buff);
    }
    
    //向服务器返回信息
    snprintf(buff, MAX_LEN, "%d load data from %s end!", id, client_name);
    lwrite(server_wfd, buff, MAX_LEN);
    
    lunlink(server_wfd);
    lunlink(client_rfd);
    
    safe_exit(NULL);
}