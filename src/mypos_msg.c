#include "mypos_msg.h"
#include <mqueue.h>
#include <signal.h>
#include <bits/sigset.h>
#include <sys/select.h>

//获取2个参数
//c 表示创建消息队列 d表示删除消息队列
//第二个参数为消息队列的路径
void handle_msg(int argc, char **argv)
{
    if(argc != 3)
        err_exit(NULL, -1);
    
    mqd_t fd = 0;
    int flag = O_RDWR | O_CREAT;
    char ch = argv[1][0];
    switch(ch)
    {
    case 'c':
        fd = lmq_open(argv[2], flag, 666,NULL); ///dev/mqueue/
        lmq_close(fd);
        break;
    case 'd':
        lmq_unlink(argv[2]);
        break;
    default:
        err_exit("unknown parameters!", -1);
    }
    
}

//程序分为客户端和服务端，客户端发送数据，服务端接受数据
//接受三个参数，第一个c或者s表示客户端和服务端，第二个参数指定消息队列的文件名
void ipc_mq(int argc, char **argv)
{
    if(argc != 3)
        err_exit(NULL, -1);
    
    mqd_t fd = 0;
    int flag = 0;
    int mode = FILE_MODE;
    char ch = argv[1][0];
    char buff[MAX_LEN] = {0};
    int len = 0;
    int prior = 20;
    switch(ch)
    {
    case 'c':
        flag = O_CREAT | O_WRONLY;
        fd = lmq_open(argv[2], flag, mode, NULL);
        //输入的前两位为消息的优先级
        //输入的格式为优先级+空格+消息
        while(lfgets(buff, MAX_LEN, stdin) != NULL)
        {
            buff[2] = '\0';
            prior = atoi(buff);
            char *msg = buff + 3;
            len = strlen(msg);
            if(msg[len - 1] == '\n')
                len--;
            msg[len] = '\0';
            lmq_send_msg(fd, msg, len, prior);
        }
        break;
    case 's':
        flag = O_RDONLY;
        fd = lmq_open(argv[2], flag, mode, NULL);
        struct mq_attr attr;
        lmq_getattr(fd, &attr);
        printf("message size %d, max message %d\n", attr.mq_msgsize, attr.mq_maxmsg);
        while((len = lmq_receive_msg(fd, buff, attr.mq_msgsize, &prior)) > 0)
        {
            printf(buff);
            printf(" ,prior is %d!\n", prior);
        }
        
        break;
    }
    
    lmq_close(fd);
}

mqd_t sg_mq;
struct sigevent sg_ev;
struct mq_attr sg_attr;
//信号处理函数
static void single_mq_handle(int sig_no)
{
    printf("the program come into the handler!\n");
    //这个函数并不是异步信号安全的函数
    char buff[MAX_LEN];
    int prior;
    lmq_receive_msg(sg_mq, buff, sg_attr.mq_msgsize, &prior);
    printf("receive singale and the buffer is %s, and the prior is %d!\n", buff, prior);
    lmq_notify(sg_mq, &sg_ev);         //再次注册
}

//程序分为客户端和服务端，客户端发送数据，服务端接受数据
//接受三个参数，第一个c或者s表示客户端和服务端，第二个参数指定消息队列的文件名
void single_mq_test(int argc, char **argv)
{
    if(argc != 3)
        err_exit(NULL, -1);
    
    int flag = 0;
    int mode = FILE_MODE;
    char ch = argv[1][0];
    char buff[MAX_LEN] = {0};
    int len = 0;
    int prior = 20;
    switch(ch)
    {
    case 'c':
        flag = O_CREAT | O_WRONLY;
        sg_mq = lmq_open(argv[2], flag, mode, NULL);
        //输入的前两位为消息的优先级
        //输入的格式为优先级+空格+消息
        lfgets(buff, MAX_LEN, stdin);
        
        buff[2] = '\0';
        prior = atoi(buff);
        char *msg = buff + 3;
        len = strlen(msg);
        
        if(msg[len - 1] == '\n')
            len--;
        msg[len] = '\0';
        lmq_send_msg(sg_mq, msg, len, prior);
        
        break;
    case 's':
        flag = O_RDONLY;
        sg_mq = lmq_open(argv[2], flag, mode, NULL);
        lmq_getattr(sg_mq, &sg_attr);
        lsignal(SIGUSR1, single_mq_handle);
        sg_ev.sigev_signo = SIGUSR1;
        sg_ev.sigev_notify = SIGEV_SIGNAL;
        lmq_notify(sg_mq, &sg_ev);
        for(;;)
            pause();
        break;
    }
    
    lmq_close(sg_mq);
}

volatile sig_atomic_t sig_mask = 0;
//信号处理函数
static void safe_single_mq_handle(int sig_no)
{
    sig_mask = 1;
}

//程序分为客户端和服务端，客户端发送数据，服务端接受数据
//接受三个参数，第一个c或者s表示客户端和服务端，第二个参数指定消息队列的文件名
void safe_single_mq_test(int argc, char **argv)
{
    if(argc != 3)
        err_exit(NULL, -1);
    
    mqd_t mq;
    int flag = 0;
    int mode = FILE_MODE;
    char ch = argv[1][0];
    char buff[MAX_LEN] = {0};
    int len = 0;
    int prior = 20;
    switch(ch)
    {
    case 'c':
        flag = O_CREAT | O_WRONLY;
        mq = lmq_open(argv[2], flag, mode, NULL);
        //输入的前两位为消息的优先级
        //输入的格式为优先级+空格+消息
        lfgets(buff, MAX_LEN, stdin);
        
        buff[2] = '\0';
        prior = atoi(buff);
        char *msg = buff + 3;
        len = strlen(msg);
        
        if(msg[len - 1] == '\n')
            len--;
        msg[len] = '\0';
        lmq_send_msg(mq, msg, len, prior);
        
        break;
    case 's':
        flag = O_RDONLY;
        struct sigevent ev;
        struct mq_attr attr;
        sigset_t new_set, old_set, zero_set;
        
        __sigemptyset(&new_set);
        __sigemptyset(&old_set);
        __sigemptyset(&zero_set);
        __sigaddset(&new_set, SIGUSR1);
        
        mq = lmq_open(argv[2], flag, mode, NULL);
        lmq_getattr(mq, &attr);
        lsignal(SIGUSR1, safe_single_mq_handle);
        ev.sigev_signo = SIGUSR1;
        ev.sigev_notify = SIGEV_SIGNAL;
        lmq_notify(mq, &ev);
        for(;;)
        {
            lsigprocmask(SIG_BLOCK, &new_set, &old_set);
            while(sig_mask == 0)
                lsigsuspend(&zero_set);
                
            sig_mask = 0;
            lmq_notify(mq, &ev);         //再次注册
            char buff[MAX_LEN];
            int prior;
            int len = 0;
            while((len = lmq_receive_msg(mq, buff, attr.mq_msgsize, &prior)) > 0)       //保证即便读取当前消息时，其他到来的消息也能读取到
            {
                printf("receive singale and the buffer is %s, and the prior is %d!\n", buff, prior);
            }
            
            lsigprocmask(SIG_UNBLOCK, &new_set, NULL);
        }
            
        break;
    }
    
    lmq_close(sg_mq);
}

int pipe_fd[2] = {0};
static void safe_pipe_mq_handle(int sig)
{
    lwrite(pipe_fd[1], "", 1);
}

void safe_pipe_mq_test(int argc, char **argv)
{
    if(argc != 3)
        err_exit(NULL, -1);
    
    mqd_t mq;
    int flag = 0;
    int mode = FILE_MODE;
    char ch = argv[1][0];
    char buff[MAX_LEN] = {0};
    int len = 0;
    int prior = 20;
    struct sigevent ev;
    struct mq_attr attr;
    fd_set rset;
    switch(ch)
    {
    case 'c':
        flag = O_CREAT | O_WRONLY;
        mq = lmq_open(argv[2], flag, mode, NULL);
        //输入的前两位为消息的优先级
        //输入的格式为优先级+空格+消息
        lfgets(buff, MAX_LEN, stdin);
        
        buff[2] = '\0';
        prior = atoi(buff);
        char *msg = buff + 3;
        len = strlen(msg);
        
        if(msg[len - 1] == '\n')
            len--;
        msg[len] = '\0';
        lmq_send_msg(mq, msg, len, prior);
        
        break;
    case 's':
        flag = O_RDONLY;
        
        mq = lmq_open(argv[2], flag, mode, NULL);
        lmq_getattr(mq, &attr);
        lpipe(pipe_fd);
        
        
        lsignal(SIGUSR1, safe_pipe_mq_handle);
        ev.sigev_signo = SIGUSR1;
        ev.sigev_notify = SIGEV_SIGNAL;
        lmq_notify(mq, &ev);
        
        FD_ZERO(&rset);
        for(;;)
        {
            FD_SET(pipe_fd[0], &rset);
            int fds = lselect(pipe_fd[0] + 1, &rset, NULL, NULL, NULL);
            if(FD_ISSET(pipe_fd[0], &rset))
            {
                char ch;
                lread(pipe_fd[0], &ch, 1);
                char buff[MAX_LEN];
                int prior;
                int len = 0;
                while((len = lmq_receive_msg(mq, buff, attr.mq_msgsize, &prior)) > 0)       //保证即便读取当前消息时，其他到来的消息也能读取到
                {
                    printf("receive singale and the buffer is %s, and the prior is %d!\n", buff, prior);
                }
                
                lmq_notify(mq, &ev);         //再次注册
            }
        }
            
        break;
    }
    
    lmq_close(sg_mq);
}

mqd_t thread_mq;

struct mq_attr thread_mq_attr;
struct sigevent thread_mq_sig;
void safe_thread_mq_handle(int val)
{
    char buff[MAX_LEN];
    int prior;
    int len = 0;
    while((len = lmq_receive_msg(thread_mq, buff, thread_mq_attr.mq_msgsize, &prior)) > 0)       //保证即便读取当前消息时，其他到来的消息也能读取到
    {
        printf("receive singale and the buffer is %s, and the prior is %d!\n", buff, prior);
    }
    
    lmq_notify(thread_mq, &thread_mq_sig);         //再次注册
}

//启动一个线程来处理事件，异步读写
void safe_thread_mq_test(int argc, char **argv)
{
    if(argc != 3)
        err_exit(NULL, -1);
    
    int flag = 0;
    int mode = FILE_MODE;
    char ch = argv[1][0];
    char buff[MAX_LEN] = {0};
    int len = 0;
    int prior = 20;
    switch(ch)
    {
    case 'c':
        flag = O_CREAT | O_WRONLY;
        thread_mq = lmq_open(argv[2], flag, mode, NULL);
        //输入的前两位为消息的优先级
        //输入的格式为优先级+空格+消息
        lfgets(buff, MAX_LEN, stdin);
        
        buff[2] = '\0';
        prior = atoi(buff);
        char *msg = buff + 3;
        len = strlen(msg);
        
        if(msg[len - 1] == '\n')
            len--;
        msg[len] = '\0';
        lmq_send_msg(thread_mq, msg, len, prior);
        
        break;
    case 's':
        flag = O_RDONLY;
        
        thread_mq = lmq_open(argv[2], flag, mode, NULL);
        lpipe(pipe_fd);
        lmq_getattr(thread_mq, &thread_mq_attr);
        lsignal(SIGUSR1, safe_thread_mq_handle);
        
        thread_mq_sig.sigev_notify = SIGEV_THREAD;
        thread_mq_sig.sigev_value.sival_ptr = NULL;
        thread_mq_sig._sigev_un._sigev_thread._attribute = NULL;
        thread_mq_sig._sigev_un._sigev_thread._function = safe_thread_mq_handle;
        
        lmq_notify(thread_mq, &thread_mq_sig);
        
        for(;;)
        {
            pause();
        }
            
        break;
    }
    
    lmq_close(sg_mq);
}

void sig_handle(int signo, siginfo_t *info, void *context)
{
    printf("received signal %d, code = %d, ival = %d\n", signo, info->si_code, info->si_value.sival_int);
}

void sig_test(int argc, char **argv)
{
    printf("SIGRTMIN=%d, SIGRTMAX=%d\n", (int)(SIGRTMIN), (int)(SIGRTMAX));
    pid_t pid;
    pid = fork();
    
    if(pid == 0)
    {
        sigset_t newset;
        
        sigemptyset(&newset);
        sigaddset(&newset, SIGRTMAX);
        sigaddset(&newset, SIGRTMAX - 1);
        sigaddset(&newset, SIGRTMAX - 2);
        sigprocmask(SIG_BLOCK, &newset, NULL);
        
        lsig_rt(SIGRTMAX, sig_handle, &newset);
        lsig_rt(SIGRTMAX - 1, sig_handle, &newset);
        lsig_rt(SIGRTMAX - 2, sig_handle, &newset);
        
        sleep(6);
        sigprocmask(SIG_UNBLOCK, &newset, NULL);
        sleep(3);
        return ;
    }
    else
    {
        /* code */
        sleep(3);
        union sigval val;
        for(int i = SIGRTMAX;i >= SIGRTMAX - 2;i--)
        {
            for(int j = 0;j <= 2;j ++)
            {
                val.sival_int = j;
                lsigqueue(pid, i, val);
                printf("send signal = %d, val = %d\n", i, j);
            }
        }
    }
    
    return 0;
}

void posix_msg_test_main(int argc, char**argv)
{   
    //handle_msg(argc, argv);
    //ipc_mq(argc, argv);
    //single_mq_test(argc, argv);
    //safe_single_mq_test(argc, argv);
    //safe_pipe_mq_test(argc, argv);
    //safe_thread_mq_test(argc, argv);
    sig_test(argc, argv);
}
