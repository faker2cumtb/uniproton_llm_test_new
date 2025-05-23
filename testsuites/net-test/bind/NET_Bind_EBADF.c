#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>

#define get_local_ip() inet_addr(server_IP)
static random_port = 0;

int NET_Bind_EBADF(int argv[])
{
    int iRet = 0;
    int sock = -1;
    struct sockaddr_in localAddr;
 
    sock = socket(AF_INET, SOCK_STREAM, 0);
    close(sock);

    


    localAddr.sin_family = PF_INET;
    localAddr.sin_addr.s_addr = get_local_ip();
    localAddr.sin_port = htons(52311);

    iRet = bind(sock, (struct sockaddr *)&localAddr, sizeof(struct sockaddr));
    if ((iRet == -1) && (errno == EBADF))
    {
        TEST_OKPRINT();
        return PTS_PASS;
    }
    else
    {
        TEST_FAILPRINT("bind");
        return PTS_FAIL;
    }
}

