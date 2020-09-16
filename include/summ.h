#include "lsafe.h"

typedef struct speed_info
{
    int time;
    int bytes;
}speed_info;

speed_info write_ipc(int fd, int writefd);
speed_info read_ipc(int readfd);
void pipe_performance_test(int argc, char **argv);

void summary_test(int argc, char **argv);