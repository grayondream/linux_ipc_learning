#include <unistd.h>
#include <stdio.h>
#include <mqueue.h>
#include <errno.h>
#include <sys/stat.h>
#include <signal.h>

#define MAX_LEN 1024
#define FILE_MODE S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP 


void err_exit(const char *buff, int err_ret);
#define ERROR_CHECK(ret, op, failed, val, desc)  if((ret) op (failed))\
                                              {\
                                              char (buff)[MAX_LEN] = {0};\
                                              snprintf((buff), MAX_LEN, (desc), (val));\
                                              fprintf(stderr, "errno is %d,", errno);     \
                                              err_exit((buff), (ret));\
                                              }
/*
 * @brief   针对api的包装函数  
 */
void safe_exit(const char *buff);
int lread(int readfd, char *buff, int len);
int lwrite(int writefd, char *buff, int len);
char* lfgets(char *buff, int len, FILE *stream);
int lopen(char *filename, int mode);
void lclose(int fd);
void lpipe(int *fd);
int lfork();
void lwaitpid(int pid, int *status, int option);
FILE* lpopen(char *command, char* type);
void lpclose(FILE *fd);
void lmkfifo(char *file, int mode);
void lunlink(char *file);


/*
 * 消息队列相关api的封装
 */
mqd_t lmq_open(char *name, int flag, int mode, struct mq_attr *attr);
void lmq_close(mqd_t mq);
void lmq_unlink(char *name);
void lmq_getattr(mqd_t mq, struct mq_attr *attr);
void lmq_setattr(mqd_t mq, const struct mq_attr *attr, struct mq_attr *oattr);
void lmq_send_msg(mqd_t mq, const char *ptr, int len, int prior);
int lmq_receive_msg(mqd_t mq, char *ptr, int len, int *prior);
void lmq_notify(mqd_t mq, struct sigevent *ev);
__sighandler_t lsignal(int sig, __sighandler_t handler);
void lsigprocmask(int how, const sigset_t *set, sigset_t *oldset);
void lsigsuspend(const sigset_t *mask);
int lselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
typedef void (*lsig_handle_t)(int signo, siginfo_t *info, void *context);
lsig_handle_t* lsig_rt(int signo, lsig_handle_t *func, sigset_t *mask);
void lsigqueue(pid_t pid, int sig, const union sigval value);
