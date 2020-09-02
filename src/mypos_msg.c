#include "mypos_msg.h"
#include <mqueue.h>

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
//接受两个参数，第一个c或者s表示客户端和服务端，第二个参数指定消息队列的文件名
void ipc_mq(int argc, char **argv)
{
    if(argc != 3)
        err_exit(NULL, -1);
    
    mqd_t fd = 0;
    int flag = 0;
    int mode = 666;
    char ch = argv[1][0];
    
    fd = lmq_open(argv[2], flag, mode, NULL);
    char buff[MAX_LEN] = {0};
    int len = 0;
    int prior = 20;
    switch(ch)
    {
    case 'c':
        while(lfgets(buff, MAX_LEN, stdin) != NULL)
        {
            len = strlen(buff);
            if(buff[len - 1] == '\n')
                len--;
            buff[len] = '\0';
            lmq_send_msg(fd, buff, len, prior);
        }
        break;
    case 's':
        while((len = lmq_receive_msg(fd, buff, MAX_LEN, &prior)) > 0)
        {
            printf(buff);
            printf("prior is %d!\n", prior);
        }
        
        break;
    }
    
    lmq_close(fd);
}

void posix_msg_test_main(int argc, char**argv)
{   
    //handle_msg(argc, argv);
    ipc_mq(argc, argv);
}