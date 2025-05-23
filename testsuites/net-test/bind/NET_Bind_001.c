#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>

 
#define get_local_ip() inet_addr(server_IP)
static random_port = 0;

int NET_Bind_001()
{
    int iRet = -1;
    int sock = -1;
    int isOk = 0;
    struct sockaddr_in serverAddr;

    


    serverAddr.sin_family = PF_INET;
    serverAddr.sin_addr.s_addr = get_local_ip();
    serverAddr.sin_port = htons(53321);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sock)
    {
        TEST_FAILPRINT(errno);
        close(sock);
        return PTS_UNRESOLVED;
    }

    iRet = bind(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    close(sock);

    if (0 == iRet)
    {
        TEST_OKPRINT();
        return PTS_PASS;
    }
    else
    {
        TEST_ERRPRINT("");
        return PTS_FAIL;
    }
}
