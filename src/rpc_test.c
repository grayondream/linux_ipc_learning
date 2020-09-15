#include "rpc_test.h"
#include <rpc/rpc.h>


void rpc_test(int argc, char **argv)
{
    if(argc != 2)
        return;

    char *host = "127.0.0.1";
    square_in in;
    CLIENT *fd = clnt_create(host, SQUARE_PROG, SQUARE_VERS, "tcp");

    in.arg1 = atoi(argv[1]);
    square_out *out = squareproc_1(&in, fd);
    if(out == NULL)
        return;

    printf("result = %d\n", out->res1);
    return;
}