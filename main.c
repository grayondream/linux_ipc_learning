#include "mypipe.h"
#include "mypos_msg.h"
#include "myvmsg.h"
#include "mutex_test.h"

int main(int argc, char**argv)
{
    //pipe_main(argc, argv);
    //posix_msg_test_main(argc, argv);
    //vmsg_test(argc, argv);
    mutex_test(argc, argv);
}