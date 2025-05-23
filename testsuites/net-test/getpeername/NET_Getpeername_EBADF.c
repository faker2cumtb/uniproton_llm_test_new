#include <pthread.h>
#include <stdio.h>
#include <sched.h>
#include <sys/socket.h>
#include "../test.h"

int NET_Getpeername_EBADF()
{
	unsigned short port = 0x1124;
    struct sockaddr  addr;
    socklen_t addr_len = sizeof(addr);
    int ret;

    int nSockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (nSockFd < 0)
    {
        printf("create socket error, errno=%d\n", errno);
        return PTS_FAIL;
    }

    close(nSockFd);
    ret = getpeername(nSockFd, (struct sockaddr *)&addr, &addr_len);
    if( (ret == -1) && (errno == EBADF))
    {
    	TEST_OKPRINT();
        return PTS_PASS;
    } else {
        printf("getpeername failed, error=%d\n", errno);
        TSTDEF_FAILPRINT(errno)	;
        return PTS_FAIL;
    }


}


