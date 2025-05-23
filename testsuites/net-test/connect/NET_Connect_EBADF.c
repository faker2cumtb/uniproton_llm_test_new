#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>

static int isOk = 0;
static random_port = 0;
#define get_local_ip() inet_addr(server_IP)

int NET_Connect_EBADF(void)
{
    int iRet = 0;
    int sock = -1;
    struct sockaddr_in servAddr;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    close(sock);

    servAddr.sin_family = PF_INET;
    servAddr.sin_addr.s_addr = get_local_ip();//1号cpu地址
    servAddr.sin_port = htons(54321);

    iRet = connect(sock, (struct sockaddr *)&servAddr, sizeof(struct sockaddr));
    if (-1 == iRet && errno == EBADF)
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
        TEST_FAILPRINT("CONNECT TEST FAILED");
        return PTS_FAIL;
    }
}

