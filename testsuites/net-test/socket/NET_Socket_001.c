
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>

static int isOk = 0;

int NET_Socket_001(int argv[])
{

    int sock = -1;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 != sock)
    {
        isOk = 1;
    }
    else
    {
        isOk = 0;
    }
    
//    closesocket(sock);
	close(sock);

    if (isOk)
    {
        TEST_OKPRINT();
        return PTS_PASS;
    }
    else
    {
        TEST_FAILPRINT("socket");
        return PTS_FAIL;
    }
}
