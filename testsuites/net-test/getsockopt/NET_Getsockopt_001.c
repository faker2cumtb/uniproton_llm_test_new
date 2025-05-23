#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>

int NET_Getsockopt_001()
{
    int iRet;
    int sock;
    int optval = 1;
    socklen_t len;
    int isOk = 0;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sock)
    {
        TEST_FAILPRINT("socket");
        return PTS_UNRESOLVED;
    }

    len = sizeof(optval);
    iRet = getsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)&optval, &len);
    if (0 <= iRet)
    {
        isOk = 1;
    }
    else
    {
        isOk = 0;
    }
    close(sock);

    if (isOk)
    {
        TEST_OKPRINT();
        return PTS_PASS;
    }
    else
    {
        printf("iRet = %d\n", iRet);
        TEST_ERRPRINT("");
        return PTS_FAIL;
    }
}
