#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>

#define ROUND (100*1024)

#define BUFF_SIZE 1460
#define get_local_ip() inet_addr(server_IP)
static uint8_t  sendBuff[BUFF_SIZE];
static random_port = 0;

static void *tcp_client(void *arg)
{
    int nSockFd;
    struct sockaddr_in remote_addr;
    int nDataLen;
    uint32_t i;
    int  status;

    nSockFd = socket(PF_INET, SOCK_STREAM, 0);
    if (nSockFd < 0)
    {
        printf("###sock err 1\n");
        return (void *) -4;
    }

    remote_addr.sin_family = PF_INET;
    remote_addr.sin_addr.s_addr = inet_addr(client_IP);
    remote_addr.sin_port = htons(56551);

    printf("connect('%d.%d.%d.%d', %d)\n",
           remote_addr.sin_addr.s_addr & 0xFF,
           (remote_addr.sin_addr.s_addr >> 8) & 0xFF,
           (remote_addr.sin_addr.s_addr >> 16) & 0xFF,
           (remote_addr.sin_addr.s_addr >> 24) & 0xFF,
           ntohs(remote_addr.sin_port) );

    status = connect(nSockFd, (struct sockaddr *) &remote_addr, sizeof(remote_addr));

    close(nSockFd);
    return 0;
}

int NET_Listen_001()
{
    int nSockFd;
    int failed = 0;
    struct sockaddr_in local_addr;
    pthread_t new_th;

    


    /* Create a new thread */
    if (pthread_create(&new_th, NULL, tcp_client, NULL) != 0)
    {
        perror("Error creating thread\n");
        return PTS_UNRESOLVED;
    }

    nSockFd = socket(PF_INET, SOCK_STREAM, 0);
    if (nSockFd < 0)
    {
        TEST_FAILPRINT("socket");
        failed = 1;
    }

    local_addr.sin_family = PF_INET;
    local_addr.sin_addr.s_addr = get_local_ip();
    local_addr.sin_port = htons(56551);

    if (bind(nSockFd, (struct sockaddr *) &local_addr, sizeof(local_addr)))
    {
        TEST_FAILPRINT("bind");
        failed = 1;
    }

    if (listen(nSockFd, 5))
    {
        TEST_ERRPRINT("listen");
        failed = 1;
    }

    close(nSockFd);

    /*判断测试结果*/
    if (failed == 1)
    {
        TEST_ERRPRINT("");
        return PTS_FAIL;
    }
    else
    {
        TEST_OKPRINT();
        return PTS_PASS;
    }
}
