#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>

int NET_Getsockopt_ENOPROTOOPT()
{
    int iRet = 0;
    int sock;
    int isOk = 0;
    int bOOBLine = 1, len;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sock)
    {
        TEST_FAILPRINT("socket");
        return PTS_UNRESOLVED;
    }
    len = sizeof(int);
    iRet = getsockopt(sock, SOL_SOCKET, 0xFFFF, &bOOBLine, (socklen_t *)&len);
    if ((-1 == iRet) && (errno == ENOPROTOOPT))
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
        TEST_FAILPRINT("getsockopt");
        return PTS_FAIL;
    }
}
