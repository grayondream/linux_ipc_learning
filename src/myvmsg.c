#include "lsafe.h"
#include "myvmsg.h"
#include <string.h>
#include <stdlib.h>

//客户端发送的消息中格式为：消息类型+空格+消息，消息类型占2位
void vmsg_base_client(int readfd, int writefd)
{
    char buff[MAX_LEN] = {0};
    while(lfgets(buff, MAX_LEN, stdin))
    {
        int len = strlen(buff);
        if(buff[len - 1] == '\n')
            len--;
        
        buff[len] = '\0';
        buff[2] = '\0';
        int type = atoi(buff);
        buff[2] = ' ';
        mymsg_buf mymsg;
        mymsg.len = strlen(buff);
        memset(mymsg.data, 0, MAX_LEN);
        memmove(mymsg.data, buff, mymsg.len);
        mymsg.type = 20;
        lmsgsnd(writefd, &(mymsg.type), sizeof(mymsg.len) + mymsg.len, 0);
        
        //接受服务端的ack
        memset(mymsg.data, 0, MAX_LEN);
        len = lmsgrcv(readfd, &(mymsg.type), MAX_LEN, type, 0);
        printf("ack from server:type=%d, msg=%s", mymsg.type, mymsg.data);
        
        printf("\n");
    }
}

void vmsg_base_server(int readfd, int writefd)
{
    mymsg_buf buf;
    while(lmsgrcv(readfd, &buf.type, MAX_LEN, 20, 0))
    {
        if(buf.data[buf.len] == '\n')
            buf.len--;
        
        buf.data[buf.len] = '\0';
        printf("read data from client: type=%d, msg=%s\n", buf.type, buf.data);
        
        buf.data[2] = '\0';
        int type = atoi(buf.data);
        memset(buf.data, 0, MAX_LEN);
        memmove(buf.data, "ACK ", 4);
        buf.type = type;
        buf.len = strlen(buf.data);
        lmsgsnd(writefd, &(buf.type), sizeof(buf.len) + buf.len, 0);
    }
}

//客户端从标准输入读取消息发送给服务端，服务端读取到消息之后回显并给客户端发送确认信息，客户端收到后回显
//很明显的是当前的流程是同步的，如果客户端收不到服务端的ack或者服务端收不到客户端的消息都会阻塞
void vmsg_base_test(int argc, char **argv)
{   
    if(argc != 4)
    {
        err_exit(NULL, -1);
    }
    
    char ch = argv[1][0];
    char *read_name = argv[2];
    char *write_name = argv[3];
    unsigned long flag = IPC_CREAT | SVMSG_MODE;    //SVMSG_MODE=0666
    key_t readkey = lftok(read_name, 0);
    key_t writekey = lftok(write_name, 0);
    
    int readfd = lmsgget(readkey, flag);
    int writefd = lmsgget(writekey, flag);
    printf("message queue readkey=%d, writekey=%d, readfd=%d, writefd=%d\n", readkey, writekey, readfd, writefd);
    switch(ch)
    {
    case 'c':
        vmsg_base_client(readfd, writefd);
        break;
    case 's':
        vmsg_base_server(readfd, writefd);
        break;
    }
}

#define SERVER_TYPE 20
//通过信号处理程序实现客户端和服务端异步的响应
//每个客户端有一个私人的消息队列，客户端创建队列之后按照格式：消息队列编号+空格+消息类型+空格+消息的格式向服务端发送消息
void vmsg_sig_client(int readfd, int writefd)
{
    char buff[MAX_LEN] = {0};
    lfgets(buff, MAX_LEN, stdin);        
    int len = strlen(buff);
    if(buff[len - 1] == '\n')
        len--;
    
    //解析消息类型
    buff[len] = '\0';
    buff[2] = '\0';
    int type = atoi(buff);
    buff[2] = ' ';
    
    //格式为 from [readfd] to [writefd] [msg]
    char send_msg[MAX_LEN] = {0};
    sprintf(send_msg, "from %d to %d %s", readfd, writefd, buff);
    
    mymsg_buf mymsg;
    mymsg.len = strlen(send_msg);
    memset(mymsg.data, 0, MAX_LEN);
    memmove(mymsg.data, send_msg, mymsg.len);
    mymsg.type = SERVER_TYPE;
    lmsgsnd(writefd, &(mymsg.type), sizeof(mymsg.len) + mymsg.len, 0);
    
    //接受服务端的ack
    memset(mymsg.data, 0, MAX_LEN);
    mymsg.type = type;
    len = lmsgrcv(readfd, &(mymsg.type), MAX_LEN, mymsg.type, 0);
    printf("ack from server:type=%d, msg=%s\n", mymsg.type, mymsg.data);
}

void sig_child(int signo)
{
    pid_t pid = 0;
    int stat = 0;
    while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
        ;
    
    return;
}

//每当有一个客户端发送了消息则服务端开启一个进程来处理该客户端的请求
void vmsg_sig_server(int readfd, int writefd)
{
    for(;;)
    {
        lsignal(SIGCHLD, sig_child);
        
        mymsg_buf read_buf;
        memset(read_buf.data, 0, MAX_LEN);
        read_buf.type = SERVER_TYPE;
        int len = lmsgrcv(readfd, &(read_buf.type), MAX_LEN, read_buf.type, 0);
        printf(read_buf.data);
        printf("\n");
        
        //解析客户端的id
        int type = 0;
        sscanf(read_buf.data, "from %d to %d %d %s", &writefd, &readfd, &type, NULL);   
        pid_t id = lfork();
        if(id == 0)         //子进程
        {
            memset(read_buf.data, 0, MAX_LEN);
            memmove(read_buf.data, "ACK ", 4);
            read_buf.type = type;
            read_buf.len = strlen(read_buf.data);
            //printf("send message!type=%d,len=%d writefd=%d\n", read_buf.type, read_buf.len, writefd);
            lmsgsnd(writefd, &(read_buf.type), sizeof(read_buf.len) + read_buf.len, 0);
        }
        else
        {
            
        }
        
    }
}

//第二个参数表示当前进程是服务端还是客户端，c/s
//第三个参数为服务端的消息队列描述
void vmsg_sig_test(int argc, char **argv)
{
    if(argc != 3)
    {
        err_exit(NULL, -1);
    }
    
    char ch = argv[1][0];
    char *server_name = argv[2];
    unsigned long flag = IPC_CREAT | SVMSG_MODE;    //SVMSG_MODE=0666
    key_t serverkey = lftok(server_name, 0);
    int clientfd = 0;
    
    int serverfd = lmsgget(serverkey, flag);
    printf("message queue serverkey=%d, serverfd=%d\n", serverkey, serverfd);
    switch(ch)
    {
    case 'c':
        clientfd = lmsgget(IPC_PRIVATE, flag);
        vmsg_sig_client(clientfd, serverfd);
        break;
    case 's':
        vmsg_sig_server(serverfd, 0);
        break;
    }
}

void vmsg_poll_client(int readfd, int writefd)
{}

void vmsg_poll_server(int readfd, int writefd)
{}

void vmsg_poll_test(int argc, char **argv)
{}

void vmsg_test(int argc, char **argv)
{
    //vmsg_base_test(argc, argv);
    vmsg_sig_test(argc, argv);
    //vmsg_poll_test(argc, argv);
}