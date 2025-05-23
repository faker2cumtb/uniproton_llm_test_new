#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>

static int isOk = 0;

int NET_Getsockopt_EBADF()
{
    int iRet;
    int bOOBLine = 1, len;    
    int sock = -1;
	len = sizeof(int);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    close(sock);
    
    iRet = getsockopt(sock, SOL_SOCKET, SO_OOBINLINE, &bOOBLine, (socklen_t *)&len);
    if ((-1 == iRet) && (errno == EBADF))
    {
        isOk = 1;
    }
    else
    {
        isOk = 0;
    }

    if (isOk)
    {
        TEST_OKPRINT();
        return PTS_PASS;
    }
    else
    {
        TEST_FAILPRINT("getsockopt");
        return PTS_FAIL;
    }
}

