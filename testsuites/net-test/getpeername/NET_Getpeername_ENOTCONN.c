#include <pthread.h>
#include <stdio.h>
#include <sched.h>
#include <sys/socket.h>
#include "../test.h"

int NET_Getpeername_ENOTCONN()
{
    struct sockaddr  addr;
    socklen_t addr_len = sizeof(addr);
    int ret;

    int nSockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (nSockFd < 0)
    {
        printf("create socket error, errno=%d\n", errno);
        return PTS_FAIL;
    }

    ret = getpeername(nSockFd, (struct sockaddr *)&addr, &addr_len);
    if( (ret == -1) && (errno == ENOTCONN))
    {
    	TEST_OKPRINT();
        return PTS_PASS;
    } else {
        printf("getpeername failed, error=%d\n", errno);
        TSTDEF_FAILPRINT(errno)	;
        return PTS_FAIL;
    }

}
