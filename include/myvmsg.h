#ifndef __MYVMSG_H__
#define __MYVMSG_H__

#include <sys/msg.h>

#define MSG_R 0400 /* read permission */
#define MSG_W 0200 /* write permission */
#define SVMSG_MODE (MSG_R | MSG_W | MSG_R >>3 | MSG_R >>6)

void vmsg_base_client(int readfd, int writefd);
void vmsg_base_server(int readfd, int writefd);
void vmsg_base_test(int argc, char **argv);

void vmsg_sig_client(int readfd, int writefd);
void vmsg_sig_server(int readfd, int writefd);
void vmsg_sig_test(int argc, char **argv);

void sig_child(int signo);
void vmsg_poll_client(int readfd, int writefd);
void vmsg_poll_server(int readfd, int writefd);
void vmsg_poll_test(int argc, char **argv);

void vmsg_test(int argc, char **argv);

#endif //__MYVMSG_H__