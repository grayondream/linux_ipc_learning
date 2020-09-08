#ifndef __LSAFE_H__
#define __LSAFE_H__

#include <unistd.h>
#include <stdio.h>
#include <mqueue.h>
#include <errno.h>
#include <sys/stat.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_LEN 1024
#define PATH_MAX 256
#define FILE_MODE S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP 

void err_exit(const char *buff, int err_ret);
#define ERROR_CHECK(ret, op, failed, val, desc)  if((ret) op (failed))\
                                              {\
                                              char (buff)[MAX_LEN] = {0};\
                                              snprintf((buff), MAX_LEN, (desc), (val));\
                                              err_exit((buff), (ret));\
                                              }\

#define ERROR_CHECK_1(func, arg1, ret, op, failed, msg)     ret = func((arg1));   \
                                                            ERROR_CHECK((ret), op, (failed), (arg1), #func" error "msg)

#define ERROR_CHECK_1D(func, arg1, ret) ERROR_CHECK_1(func, arg1, ret, ==, -1, "%d")
#define ERROR_CHECK_1S(func, arg1, ret) ERROR_CHECK_1(func, arg1, ret, ==, -1, "%s")
#define ERROR_CHECK_1P(func, arg1, ret) ERROR_CHECK_1(func, arg1, ret, ==, -1, "%p")

#define ERROR_CHECK_2(func, arg1, arg2, ret, op, failed, msg)  ret = func((arg1), (arg2)); \
                                                               ERROR_CHECK((ret), op, (failed), (arg1), #func""msg)
#define ERROR_CHECK_2D(func, arg1, arg2, ret) ERROR_CHECK_2(func, arg1, arg2, ret, ==, -1, "%d")
#define ERROR_CHECK_2S(func, arg1, arg2, ret) ERROR_CHECK_2(func, arg1, arg2, ret, ==, -1, "%s")
#define ERROR_CHECK_2P(func, arg1, arg2, ret) ERROR_CHECK_2(func, arg1, arg2, ret, ==, -1, "%p")

#define SAFE_RELEASE(p) if(!(p)){free((p)); (p)=NULL;}

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

/*
 * System V消息队列
 */
 typedef struct mymsg_buf
{
    long type;
    int len;
    char data[MAX_LEN];
}mymsg_buf;

int lmsgget(key_t key, int msgflg);
void lmsgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);
ssize_t lmsgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);
void lmsgctl(int msqid, int cmd, struct msqid_ds *buf);
key_t lftok(const char *pathname, int proj_id);


/*
 * 互斥锁
 */
void lpthread_mutex_init (pthread_mutex_t *__mutex, const pthread_mutexattr_t *__mutexattr);  //初始化互斥锁
void lpthread_mutex_destroy (pthread_mutex_t *__mutex);       //销毁一个互斥锁
int lpthread_mutex_trylock (pthread_mutex_t *__mutex);       //尝试获得互斥锁
void lpthread_mutex_lock (pthread_mutex_t *__mutex);          //加锁
void lpthread_mutex_unlock (pthread_mutex_t *__mutex);         //解锁
void lpthread_create (pthread_t * __newthread, const pthread_attr_t * __attr, void *(*__start_routine) (void *), void * __arg);
void lpthread_exit (void *__retval);
int lpthread_join (pthread_t __th, void **__thread_return);

/*
 * 信号量
 */
void lpthread_cond_init (pthread_cond_t * __cond, const pthread_condattr_t * __cond_attr);
void lpthread_cond_destroy (pthread_cond_t *__cond);
void lpthread_cond_signal (pthread_cond_t *__cond);
void lpthread_cond_broadcast (pthread_cond_t *__cond);
void lpthread_cond_wait (pthread_cond_t * __cond, pthread_mutex_t * __mutex);
void lpthread_cond_timedwait (pthread_cond_t * __cond, pthread_mutex_t * __mutex, const struct timespec * __abstime);

/*
 * 读写锁
 */
#define MAGIC_CHECK 0x19283746
typedef struct lpthread_rwlock_t
{
    pthread_mutex_t mutex;              //互斥锁
    pthread_cond_t cond_readers;        //读
    pthread_cond_t cond_writers;        //写
    int wait_readers;                      //等待读者数量
    int wait_writers;                      //等待写数量
    int refercount;                     //引用计数
    int magic;                          //用于检查当前对象是否初始化
}lpthread_rwlock_t;

typedef int lpthread_rwlockattr_t;
#define LPTHREAD_RWLOCK_INITIALIZER {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER, 0, 0, 0, MAGIC_CHECK}

int lpthread_rwlock_init(lpthread_rwlock_t * rwlock, const lpthread_rwlockattr_t * attr);          //初始化读写锁
int lpthread_rwlock_destroy(lpthread_rwlock_t *rwlock);   //销毁读写锁
int lpthread_rwlock_rdlock(lpthread_rwlock_t *rwlock);    //读加锁
int lpthread_rwlock_wrlock(lpthread_rwlock_t *rwlock);    //写加锁
int lpthread_rwlock_unlock(lpthread_rwlock_t *rwlock);    //解锁
int lpthread_rwlock_tryrdlock(lpthread_rwlock_t *rwlock); //非阻塞获得读锁
int lpthread_rwlock_trywrlock(lpthread_rwlock_t *rwlock); //非阻塞获得写锁

void lpthread_rwlock_cancel_rdwait(void *arg);              //线程取消后处理
void lpthread_rwlock_cancel_wrwait(void *arg);

/*
 * 记录锁
 */
int lfcntl_lock(int fd, int cmd, int type, off_t start, int where, off_t len);

pid_t lfcntl_lockable(int fd, int type, off_t start, int where, off_t len);

#define lfcntl_rd_lock( fd, offset, where, len) lfcntl_lock(fd, F_SETLK,  F_RDLCK, offset, where, len)
#define lfcntl_rd_lockw(fd, offset, where, len) lfcntl_lock(fd, F_SETLKW, F_RDLCK, offset, where, len)
#define lfcntl_wr_lock( fd, offset, where, len) lfcntl_lock(fd, F_SETLK,  F_WRLCK, offset, where, len)
#define lfcntl_wr_lockw(fd, offset, where, len) lfcntl_lock(fd, F_SETLKW, F_WRLCK, offset, where, len)
#define lfcntl_unlock(  fd, offset, where, len) lfcntl_lock(fd, F_SETLK,  F_UNLCK, offset, where, len)

//如果未上锁则返回0，上锁了则返回进程id
#define lfcntl_rd_lockable(fd, offset, where, len) lfcntl_lockable(fd, F_RDLCK, offset, where, len)
#define lfcntl_wr_lockable(fd, offset, where, len) lfcntl_lockable(fd, F_WRLCK, offset, where, len)

char* lget_time(void);

/*
 * 信号量相关
 */
sem_t *lsem_open(const char *name, int oflag, mode_t mode, unsigned int value);
void   lsem_close(sem_t *sem);
void   lsem_unlink(const char *name);
void   lsem_post(sem_t *sem);
void   lsem_wait(sem_t *sem);
void   lsem_trywait(sem_t *sem);
void   lsem_getvalue(sem_t *sem, int *sval);

char * px_ipc_name(const char *name);
char * lpx_ipc_name(const char *name);

void lsem_init(sem_t *sem, int pshared, unsigned int value);
void lsem_destroy(sem_t *sem);

#endif