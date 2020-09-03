#include "lsafe.h"
#include <signal.h>

void handle_msg(int argc, char **argv);
static void single_mq_handle(int sig_no);
void single_mq_test(int argc, char **argv);
static void safe_single_mq_handle(int sig_no);
void safe_single_mq_test(int argc, char **argv);
static void safe_pipe_mq_handle(int sig);
void safe_pipe_mq_test(int argc, char **argv);
void safe_thread_mq_handle(int sig);
void safe_thread_mq_test(int argc, char **argv);
void sig_handle(int signo, siginfo_t *info, void *context);
void sig_test(int argc, char **argv);
void posix_msg_test_main(int argc, char** argv);