
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <wait.h>
#include <mqueue.h>
#include <sys/select.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <time.h>
#include "lsafe.h"

void err_exit(const char *buff, int err_ret)
{
    perror(buff);  
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

void lmq_notify(mqd_t mq, struct sigevent *ev)
{
    int ret = mq_notify(mq, ev);
    ERROR_CHECK(ret, <, 0, mq, "register the singal event failed!");   
}

__sighandler_t lsignal(int __sig, __sighandler_t __handler)
{
    __sighandler_t ret = signal(__sig, __handler);
    ERROR_CHECK(ret, ==, SIG_ERR, __sig, "register singal %d handler failed!");
    return ret;
}

void lsigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
    int ret = sigprocmask(how, set, oldset);
    ERROR_CHECK(ret, <, 0, how, "process the singal %d failed!");
}

void lsigsuspend(const sigset_t *mask)
{
    int ret = sigsuspend(mask);
    ERROR_CHECK(ret, <, 0, mask, "suspend the singal %d failed!");
}

int lselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
    int ret = select(nfds, readfds, writefds, exceptfds, timeout);
    ERROR_CHECK(ret, < , 0, nfds, "listen %d failed!");
    return ret;
}

lsig_handle_t* lsig_rt(int signo, lsig_handle_t *func, sigset_t *mask)
{
    struct sigaction act, oact;
    
    act.sa_mask = *mask;
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = func;
    if(signo == SIGALRM)
    {
    #ifdef SA_RESTART
        act.sa_flags |= SA_RESTART;
    #endif
    }
    
    int ret = sigaction(signo, &act, &oact);
    ERROR_CHECK(ret, <, 0, signo, "set the signal %d handle function failed!");
    return oact.sa_sigaction;
}

void lsigqueue(pid_t pid, int sig, const union sigval value)
{
    int ret = sigqueue(pid, sig, value);
    ERROR_CHECK(ret, <, 0, sig, "send signal %d failed!");
}

int lmsgget(key_t key, int msgflg)
{
    int ret = msgget(key, msgflg);
    ERROR_CHECK(ret, <, 0, key, "open the message queue %d failed!");
    return ret;
}

void lmsgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg)
{
    int ret = msgsnd(msqid, msgp, msgsz, msgflg);
    ERROR_CHECK(ret, <, 0, msqid, "send data into %d failed!");
}

ssize_t lmsgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg)
{
    int ret = msgrcv(msqid, msgp, msgsz, msgtyp, msgflg);
    ERROR_CHECK(ret, <, 0, msqid, "read data from queue %d failed!");
    return ret;
}

/*
void lmsgctl(int msqid, int cmd, struct msqid_ds *buf)
{
    int ret = msgctl(msqid, cmd, buf);
    ERROR_CHECK(ret, <, 0, msqid, "control the queue %d failed!");
}

*/

key_t lftok(const char *pathname, int proj_id)
{
    key_t ret = ftok(pathname, proj_id);
    ERROR_CHECK(ret, <, 0, pathname, "get the token for %s failed!");
    return ret;
}

void lpthread_mutex_init (pthread_mutex_t *__mutex, const pthread_mutexattr_t *__mutexattr)
{
    int ret = pthread_mutex_init(__mutex, __mutexattr);
    ERROR_CHECK(ret, !=, 0, ret, "initialize mutex failed %d!");
}

void lpthread_mutex_destroy (pthread_mutex_t *__mutex)
{
    int ret = pthread_mutex_destroy(__mutex);
    ERROR_CHECK(ret, !=, 0, ret, "destroy mutex failed %d!");
}

int lpthread_mutex_trylock (pthread_mutex_t *__mutex)
{
    int ret = pthread_mutex_trylock(__mutex);
    //ERROR_CHECK(ret, !=, 0, ret, "initialize mutex failed %d!");
    return ret;
}

void lpthread_mutex_lock (pthread_mutex_t *__mutex)
{
    int ret = pthread_mutex_lock(__mutex);
    ERROR_CHECK(ret, !=, 0, ret, "lock mutex failed %d!");
}

void lpthread_mutex_unlock (pthread_mutex_t *__mutex)
{
    int ret = pthread_mutex_unlock(__mutex);
    ERROR_CHECK(ret, !=, 0, ret, "unlock mutex failed %d!");
}

void lpthread_create (pthread_t * __newthread, const pthread_attr_t * __attr, void *(*__start_routine) (void *), void * __arg)
{
    int ret = pthread_create(__newthread, __attr, __start_routine, __arg);
    ERROR_CHECK(ret, !=, 0, 0, "create thread failed!");
}

void lpthread_exit (void *__retval)
{
    pthread_exit(__retval);
}

int lpthread_join (pthread_t __th, void **__thread_return)
{
    int ret = pthread_join(__th, __thread_return);
    ERROR_CHECK(ret, !=, 0, 0, "join thread failed!");
}

void lpthread_cond_init (pthread_cond_t * __cond, const pthread_condattr_t * __cond_attr)
{
    int ret = pthread_cond_init(__cond, __cond_attr);
    ERROR_CHECK(ret, !=, 0, ret, "initialize conda failed!");
}

void lpthread_cond_destroy (pthread_cond_t *__cond)
{
    int ret = pthread_cond_destroy(__cond);
    ERROR_CHECK(ret, !=, 0, ret, "destroy conda failed!");
}

void lpthread_cond_signal (pthread_cond_t *__cond)
{
    int ret = pthread_cond_signal(__cond);
    ERROR_CHECK(ret, !=, 0, ret, "wake conda failed!");
}

void lpthread_cond_broadcast (pthread_cond_t *__cond)
{
    int ret = pthread_cond_broadcast(__cond);
    ERROR_CHECK(ret, !=, 0, ret, "initialize conda failed!");
}

void lpthread_cond_wait (pthread_cond_t * __cond, pthread_mutex_t * __mutex)
{
    int ret = pthread_cond_wait(__cond, __mutex);
    ERROR_CHECK(ret, !=, 0, ret, "initialize conda failed!");
}

void lpthread_cond_timedwait (pthread_cond_t * __cond, pthread_mutex_t * __mutex, const struct timespec * __abstime)
{
    int ret = pthread_cond_timedwait(__cond, __mutex, __abstime);
    ERROR_CHECK(ret, !=, 0, ret, "initialize conda failed!");
}

int lpthread_rwlock_init(lpthread_rwlock_t * rwlock, const lpthread_rwlockattr_t * attr)
{
    if(attr != 0)
    {
        err_exit("Error Parameters", attr);
    }
    
    int ret = 0;
    if((ret = pthread_mutex_init(&rwlock->mutex, NULL)) != 0)
    {
        return ret;
    }
    
    if((ret = pthread_cond_init(&rwlock->cond_readers, NULL)) != 0)
    {
        pthread_mutex_destroy(&rwlock->mutex);
        return ret;
    }
    
    if((ret = pthread_cond_init(&rwlock->cond_writers, NULL)) != 0)
    {
        pthread_mutex_destroy(&rwlock->mutex);
        pthread_cond_destroy(&rwlock->cond_readers);
    }
    rwlock->magic = MAGIC_CHECK;
    rwlock->wait_readers = 0;
    rwlock->wait_writers = 0;
    rwlock->refercount = 0;
    return ret;
}

int lpthread_rwlock_destroy(lpthread_rwlock_t *rwlock)
{
    if(rwlock->magic != MAGIC_CHECK)
    {
        return EINVAL;
    }
    
    if(rwlock->refercount != 0 || 0 != rwlock->wait_readers || 0 != rwlock->wait_writers)
    {
        return EBUSY;
    }
    
    int ret = 0;
    if((ret =  pthread_mutex_destroy(&rwlock->mutex)) != 0)
        return ret;
        
    if(( ret = pthread_cond_destroy(&rwlock->cond_readers)) != 0)
        return ret;
        
    if(( ret = pthread_cond_destroy(&rwlock->cond_writers)) != 0)
        return ret;
    
    return ret;
}

int lpthread_rwlock_rdlock(lpthread_rwlock_t *rwlock)
{
    if(rwlock->magic != MAGIC_CHECK)
        return EINVAL;

    int ret = 0;
    if((ret = pthread_mutex_lock(&rwlock->mutex)) != 0)
        return ret;

    //refercount < 0 表示当前有写
    while(rwlock->refercount < 0 || rwlock->wait_writers > 0)
    {
        rwlock->wait_readers ++;
        //pthread_cleanup_push(lpthread_rwlock_cancel_rdwait, (void*)rwlock);
        ret = pthread_cond_wait(&rwlock->cond_readers, &rwlock->mutex);
        //pthread_cleanup_pop(0);
        rwlock->wait_readers --;
        if(ret != 0)
            break;
    }

    if(ret == 0)
    {
        rwlock->refercount ++;
    }

    ret = pthread_mutex_unlock(&rwlock->mutex);
    return ret;
}

int lpthread_rwlock_wrlock(lpthread_rwlock_t *rwlock)
{
    if(rwlock->magic != MAGIC_CHECK)
        return EINVAL;

    int ret = 0;
    if((ret = pthread_mutex_lock(&rwlock->mutex)) != 0)
        return ret;

    //refercount < 0 表示当前有锁占有
    while(rwlock->refercount != 0)
    {
        rwlock->wait_writers ++;
        //pthread_cleanup_push(lpthread_rwlock_cancel_wrwait, (void*)rwlock);
        ret = pthread_cond_wait(&rwlock->cond_writers, &rwlock->mutex);
        //pthread_cleanup_pop(0);
        rwlock->wait_writers --;
        if(ret != 0)
            break;
    }

    if(ret == 0)
    {
        rwlock->refercount = -1;            //当前有一个写锁占用
    }

    ret = pthread_mutex_unlock(&rwlock->mutex);
    return ret;
}

int lpthread_unlock(lpthread_rwlock_t *rwlock)
{
    if(rwlock->magic != MAGIC_CHECK)
        return EINVAL;

    int ret = 0;
    if((ret = pthread_mutex_lock(&rwlock->mutex)) != 0)
        return ret;

    //释放锁
    if(rwlock->refercount == -1)
    {
        rwlock->refercount = 0;         //释放读锁
    }
    else if(rwlock->refercount > 0)
    {
        rwlock->refercount --;          //释放一个写锁
    }
    else
    {
        return EINVAL;
    }
    
    //条件变量通知，优先处理写锁
    if(rwlock->wait_writers > 0 && rwlock->refercount == 0)
    {
        ret = pthread_cond_signal(&rwlock->cond_writers);
    }
    else if(rwlock->wait_readers > 0)
    {
        ret = pthread_cond_broadcast(&rwlock->cond_readers);
    }

    ret = pthread_mutex_unlock(&rwlock->mutex);
    return ret;
}

int lpthread_rwlock_tryrdlock(lpthread_rwlock_t *rwlock)
{
    if(rwlock->magic != MAGIC_CHECK)
    return EINVAL;

    int ret = 0;
    if((ret = pthread_mutex_lock(&rwlock->mutex)) != 0)
        return ret;

    //refercount < 0 表示当前有锁占有
    if(rwlock->refercount == -1 || rwlock->wait_writers > 0)
    {
        ret = EBUSY;
    }
    else
    {
        rwlock->refercount ++;
    }

    ret = pthread_mutex_unlock(&rwlock->mutex);
    return ret;
}

int lpthread_rwlock_trywrlock(lpthread_rwlock_t *rwlock)
{
    if(rwlock->magic != MAGIC_CHECK)
        return EINVAL;

    int ret = 0;
    if((ret = pthread_mutex_lock(&rwlock->mutex)) != 0)
        return ret;

    //refercount < 0 表示当前有锁占有
    if(rwlock->refercount != 0)
    {
        ret = EBUSY;
    }
    else
    {
        rwlock->refercount = -1;
    }

    ret = pthread_mutex_unlock(&rwlock->mutex);
    return ret;
}

void lpthread_rwlock_cancel_rdwait(void *arg)
{
    lpthread_rwlock_t *rw = arg;
    rw->wait_readers --;
    pthread_mutex_unlock(&rw->mutex);
}

void lpthread_rwlock_cancel_wrwait(void *arg)
{
    lpthread_rwlock_t *rw = arg;
    rw->wait_writers --;
    pthread_mutex_unlock(&rw->mutex);
}

int lfcntl_lock(int fd, int cmd, int type, off_t start, int where, off_t len)
{
    struct flock arg;

    arg.l_len = len;
    arg.l_start = start;
    arg.l_type = type;
    arg.l_whence = where;
    int ret = fcntl(fd, cmd, &arg);
    if(ret == -1)
        err_exit("get the lock error", -1);
    
    return ret;
}

pid_t lfcntl_lockable(int fd, int type, off_t start, int where, off_t len)
{
    struct flock arg;

    arg.l_len = len;
    arg.l_start = start;
    arg.l_type = type;
    arg.l_whence = where;

    int ret = fcntl(fd, F_GETLK, &arg);
    if(ret == -1)
        return ret;

    if(arg.l_type == F_UNLCK)
        return 0;

    return ret;
}



char* lget_time(void) 
{
    static char str[30];
    struct timeval tv;
    char* ptr;

    if (gettimeofday(&tv, NULL) < 0) {
        err_exit("gettimeofdata error", -1);
    }

    ptr = ctime(&tv.tv_sec);
    strcpy(str, &ptr[11]);
    sprintf(str+8, ".%06ld", tv.tv_usec);

    return str;
}

sem_t *lsem_open(const char *name, int oflag, mode_t mode, unsigned int value)
{
    sem_t* ret = sem_open(name, oflag, mode, value);
    ERROR_CHECK(ret, ==, SEM_FAILED, name, "sem_open error %s");
    return ret;
}

void lsem_close(sem_t *sem)
{
    int ret = 0;
    ERROR_CHECK_1P(sem_close, sem, ret);
}

void lsem_unlink(const char *name)
{
    int ret = 0;
    ERROR_CHECK_1S(sem_unlink, name, ret);
}

void lsem_post(sem_t *sem)
{
    int ret = 0;
    ERROR_CHECK_1P(sem_post, sem, ret);
}

void lsem_wait(sem_t *sem)
{
    int ret = 0;
    ERROR_CHECK_1P(sem_wait, sem, ret);
}

void lsem_trywait(sem_t *sem)
{
    int ret = 0;
    ERROR_CHECK_1P(sem_trywait, sem, ret);
}

void lsem_getvalue(sem_t *sem, int *sval)
{
    int ret = 0;
    ERROR_CHECK_2P(sem_getvalue, sem, sval, ret);
}


char * px_ipc_name(const char *name)
{
        char    *dir, *dst, *slash;
        //分配的空间在哪里释放的哦！@deprecated 
        if ( (dst = malloc(PATH_MAX)) == NULL)
                return(NULL);
 
                /* 4can override default directory with environment variable */
        if ( (dir = getenv("PX_IPC_NAME")) == NULL) {
#ifdef  POSIX_IPC_PREFIX
                dir = POSIX_IPC_PREFIX;         /* from "config.h" */
#else
                dir = "/tmp";                           /* default */
#endif
        }
                /* 4dir must end in a slash */
        slash = (dir[strlen(dir) - 1] == '/') ? "" : "/";
        snprintf(dst, PATH_MAX, "%s%s%s", dir, slash, name);
 
        return(dst);                    /* caller can free() this pointer */
}
/* end px_ipc_name */
 
char * lpx_ipc_name(const char *name)
{
        char    *ptr;
        // gcc -posix -E -dM - </dev/null >
#ifdef linux
        ptr = (char *)name;
#else
        if ( (ptr = px_ipc_name(name)) == NULL)
                err_sys("px_ipc_name error for %s", name);
#endif
        return(ptr);
}

void lsem_init(sem_t *sem, int pshared, unsigned int value)
{
    int ret = sem_init(sem, pshared, value);
    ERROR_CHECK(ret, ==, -1, sem, "sem_open error %p");
}

void lsem_destroy(sem_t *sem)
{
    int ret = 0;
    ERROR_CHECK_1P(sem_destroy, sem, ret);
}

int lsemget(key_t key, int nsems, int semflg)
{
    int ret = semget(key, nsems, semflg);
    ERROR_CHECK(ret, ==, -1, key, "semget %d failed");
    return ret;
}

void lsemop(int semid, struct sembuf *sops, size_t nsops)
{
    int ret = semop(semid, sops, nsops);
    ERROR_CHECK(ret, ==, -1, semid, "semop %d failed!");
}
