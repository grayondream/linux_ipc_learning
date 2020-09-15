#include "square.h"

square_out* squareproc_1_svc(square_in *in, struct svc_req *rqstp)
{
    static square_out out;
    out.res1 = in->arg1 * in->arg1;
    return &out;
}