#include "mypipe.h"
#include "mypos_msg.h"
#include "myvmsg.h"
#include "mutex_test.h"
#include "sem.h"
#include "mmp.h"

int main(int argc, char**argv)
{
    //pipe_main(argc, argv);
    //posix_msg_test_main(argc, argv);
    //vmsg_test(argc, argv);
    //mutex_test(argc, argv);
    //sem_test();
    //v_test(argc, argv);
    mmp_test(argc, argv);
}