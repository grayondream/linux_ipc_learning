&emsp;&emsp;操作系统中每个进程都是独立的资源分配的最小单位，互相是隔离的。进程通信就是为了使得不同进程之间互相访问资源并进行协调工作。
&emsp;&emsp;需要注意的是示例代码中所有以```l```开头的代码都是经过封装的库函数，和库函数的功能没有区别，比如:
```c
int lwrite(int writefd, char *buff, int len)
{
    int ret = write(writefd, buff, len);
    ERROR_CHECK(ret, <, 0, writefd, "write data into %d failed!");    
    return ret;
}
```
# 1 管道
## 1.1 无名管道
### 1.1.1 简介
&emsp;&emsp;在类Unix操作系统（以及一些其他借用了这个设计的操作系统，如Windows）中，管道是一系列将标准输入输出链接起来的进程，其中每一个进程的输出被直接作为下一个进程的输入。管道，顾名思义，就是数据会在管道中从一端流向另一端，因此是半双工的，即同一个管道只能有一个读端和写段。当然也有支持全双工管道的操作系统，但是日常中主流依然是linux和windows因此不做讨论。
&emsp;&emsp;最简单的例子就是```linux shell```中如```cat filename | sort```其中```cat filename```的输出会作为```sort```的输入。其中特殊的“|”字符告诉命令行解释器（Shell）将前一个命令的输出通过“管道”导入到接下来的一行命令作为输入。
### 1.1.2 相关api
#### 1.1.2.1 ```pipe```
```c
#include <unistd.h>
int pipe(int pipefd[2]);
```
- ```pipe```接受一个两个元素的数组，创建匿名管道；
  - 该管道的读描述符存放在```pipefd[0]```中；
  - 写描述符存放在```pipefd[1]```中；
- 返回值；
  - 0   表示成功；
  - -1   表示失败，并且设置```errno```。

&emsp;&emsp;通过```pipe```创建的管道如下图所示：
![](img/nonamepipe.drawio.svg)

&emsp;&emsp;既然管道是用来进行进程通信的，那么一个进程如何获取另一个进程创建的管道描述符？无法直接获取因为是匿名管道，只能通过```fork```共享文件描述符。因此可以看到匿名管道只能进行具有共同祖先的进程之间的通信。

&emsp;&emsp;下面的示例是一个单客户端，单服务器，具体功能是父进程作为客户端，向服务器发送一个文件路径名；子进程作为服务端接收到文件路径名之后读取其中的数据并将该数据返回给客户端，客户端进行回显。

```c
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
```
&emsp;从示例中可以看到，进程创建了两个匿名管道，文件描述符分别保存于```fd1,fd2```，随后父子进程分别关闭一个的读端和另一个的写端。由于linux中万物皆未文件，因此对管道的操作和对文件的操作无异。父子进程的数据交互模型如下所示：
![](img/nonamepipe_fork.drawio.svg)

#### 1.1.2.2 ```popen```
```c
#include <stdio.h>
FILE *popen(const char *command, const char *type);
int pclose(FILE *stream);
```
- ```popen```：创建一个管道并启动一个进程执行命令```command```，该进程要么从管道读取标准输入，要么从管道写入标准输出；
  - ```command```：接受一个shell命令行；
  - ```type```：表示进程如何操作管道：
    - ```"w"```：调用进程读写```command```的标准输入；
    - ```"r"```：调用进程读进```command```的标准输出；
  - 返回值```NULL```，失败；
  - 返回值非```NULL```，成功；
- ```pclose```：关闭通过```popen```打开的文件描述符；
  - 返回值-1，关闭失败；
  - 返回值0，关闭成功。

&emsp;&emsp;下面的例子：用户输入一个命令，通过```popen```打开管道读取该命令的输出。
```c
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
```
## 1.2 有名管道
### 1.2.1 简介
&emsp;&emsp;命名管道是计算机进程间的一种先进先出通信机制。是类Unix系统传统管道的扩展。传统管道属于匿名管道，其生存期不超过创建管道的进程的生存期。但命名管道的生存期可以与操作系统运行期一样长。

### 1.2.2 相关API
```c
#include <sys/types.h>
#include <sys/stat.h>
int mkfifo(const char *pathname, mode_t mode);
int unlink(const char *pathname);
```
- ```mkfifo```创建一个有名管道，该管道拥有一个系统上的路径名，因此不同进程之间可以通过该路径名读写管道；
  - ```pathname```：与管道相关联的路径名；
  - ```mode```：文件权限；
  - 返回值-1：创建失败；
  - 返回值0：创建成功；
- ```unlink```：从系统中删除该有名管道，这里需要理清不同进程之间的关系防止其他进程在操作数据之前就管道被其他进程删除；
  - ```pathname```：管道的路径名；
  - 返回值 -1：创建失败；
  - 返回值0：创建成功。

&emsp;&emsp;通过```mkfifo```创建管道时，在管道被删除之前会在文件系统上创建一个文件,如果使用```ls -l```查看该文件的属性会看到如下，文件首字母是```p```表示是一个管道文件。
```bash
prw-rw-r-- 1 grayondream grayondream    0 9月   2 12:28 fater
```
### 1.2.3 示例
&emsp;&emsp;下面的示例是在匿名管道上的进程通信的基础上修改的，只是把匿名管道更换为有名管道，功能类似。
```c
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
```

&emsp;&emsp;下面的示例是对上面示例的一个小修改，将程序分离成了两个单独的进行。
```c
//client
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
```

```c
//server
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
```

&emsp;&emsp;下面是上面程序的进阶版本，程序的模型是单服务器多客户端，服务器拥有管道```/home/grayondream/altas/ipc/build/tmp/server```，每个客户端都有一个自己的管道```/home/grayondream/altas/ipc/build/tmp/client.id```，基本流程为：
1. 用户在服务器端输入id+空格+路径名，其中id占两个字节即只能是两位数，范围为[0,99]；
2. 服务端将该文件路径写入到管道```/home/grayondream/altas/ipc/build/tmp/client.id```中，比如```/home/grayondream/altas/ipc/build/tmp/client.1```；
3. 客户端通过读取自身的管道之后获得文件名，然后读取该文件并回显其中的内容，客户端向服务端发送确认信息，客户端退出；
4. 服务端进行下一轮。

```c
#define MULT_SERVER_NAME "/home/grayondream/altas/ipc/build/tmp/server"
#define MULT_CLIENT_NAME "/home/grayondream/altas/ipc/build/tmp/client.%d"

//server 通过发送id + ' ' + '文件名'的格式向指定的client发送请求，id取值范围为01-20占两个字节
void mult_fifo_server_process()
{
    printf("start server and send file into client!\n");
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
        
        //等待客户端回传，状态
        while(lread(server_rfd, buff, MAX_LEN) > 0)
            printf(buff);
            
        printf("\n");
        lunlink(client);
    }
    
    lunlink(server_rfd);
}
```

```c
//client
void mult_fifo_client_process()
{
    printf("start client waiting for the post from server!\n");
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
    
    lunlink(client_name);
    lunlink(MULT_SERVER_NAME);
    
    safe_exit(NULL);
}
```
&emsp;&emsp;从程序中可以看出上面的客户端服务器模型是一个迭代服务器模型即每次服务端处理一个链接，也可以采用多进程```fork```或者多线程```pthread```处理请求，即并发服务器。

## 1.3 管道和FIFO的其他属性和限制
&emsp;&emsp;管道的打开和文件的操作无异，当管道被打开之后可以通过api```fcntl```修改文件的属性。
```c
#include <unistd.h>
#include <fcntl.h>
int fcntl(int fd, int cmd, ... /* arg */ );
```

&emsp;&emsp;管道或者FIFO的读取和写入的若干规则：
- 如果读取请求的数据量多于管道或者FIFO中的数据量，那么只返回其中的数据；
- 如果写入的数据字节数小于或者等于```PIPE_BUF```(Posix限制)，那么```write```能够保证是原子的；否则无法保证是原子的；
- 设置```O_NONBLOCK```即非阻塞，并不会对```write```原子性有影响，当设置非阻塞：
  - 写的字节数小于等于```PIPE_BUF```：
    - 如果管道或者FIFO中有足够存放请求字节数的空间，则所有数据写入；
    - 如果管道或者FIFO中没有足以存放请求字节数的空间，则立即返回一个```EAGAIN```错误；
  - 写入的字节数大于```PIPE_BUF```：
    - 如果管道或者FIFO中至少有一个字节的空间，则写入管道或者FIFO中能够容纳的数据；
    - 如果管道已经满了，则立即返回一个```EAGAIN```错误。
- 如果向没有为读打开的管道或者FIFO写入，那么内核产生```SIGPIPE```信号：
  - 如果进程未捕捉也未忽略该信号，则默认行为为终止程序；
  - 如果进程捕捉或者忽略了该信号并从信号处理程序中返回，则```write```返回一个```EPIPE```错误。

&emsp;&emsp;管道和FIFO的限制:
- ```OPEN_MAX```：每个进程可以打开的最大描述符数量；
- ```PIPE_BUF```：可原子的写管道或者FIFO的字节数。


# 2 消息队列
