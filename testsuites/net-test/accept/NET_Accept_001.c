#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>

#define ROUND (100*1024)
#define BUFF_SIZE 1460

#define get_local_ip() inet_addr(server_IP)

static uint8_t  sendBuff[BUFF_SIZE];
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static int ready = 0;
static random_port = 0;

static void *tcp_client(void *arg)
{
    int nSockFd;
    int status;
    struct sockaddr_in remote_addr;
    int nDataLen;
    uint32_t i;

    struct timespec ts;

    ts.tv_sec = 0;			//0s
    ts.tv_nsec = 20000;	
    nanosleep(&ts,NULL);


    nSockFd = socket(PF_INET, SOCK_STREAM, 0);
    if (nSockFd < 0)
    {
        printf("###sock err 1\n");
        return (void *) -4;
    }

    remote_addr.sin_family = PF_INET;
    remote_addr.sin_addr.s_addr = inet_addr(server_IP);
    remote_addr.sin_port = htons(51111);

    printf("connect('%d.%d.%d.%d', %d)\n",
           remote_addr.sin_addr.s_addr & 0xFF,
           (remote_addr.sin_addr.s_addr >> 8) & 0xFF,
           (remote_addr.sin_addr.s_addr >> 16) & 0xFF,
           (remote_addr.sin_addr.s_addr >> 24) & 0xFF,
           ntohs(remote_addr.sin_port) );
           
    ts.tv_sec = 1;
    ts.tv_nsec = 20000;
    nanosleep(&ts,NULL);

    status = connect(nSockFd, (struct sockaddr *) &remote_addr, sizeof(remote_addr));
    if (status != 0)
    {
        printf("###sock err conn:%s\n", strerror(errno));
    }

    // Signal that the client has connected
    pthread_mutex_lock(&mutex);
    ready = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    return 0;
}

int NET_Accept_001()
{
    int sock = -1, subSock;
    unsigned int clientAddrLen;
    struct sockaddr_in local_addr, clientAddr;
    pthread_t new_th;

    


    // /* Create a new thread */
    // if (pthread_create(&new_th, NULL, tcp_client, NULL) != 0)
    // {
    //     printf("Error creating thread\n");
    //     return PTS_UNRESOLVED;
    // }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sock)
    {
        TEST_FAILPRINT("socket");
        return PTS_UNRESOLVED;
    }


    local_addr.sin_family = PF_INET;
    local_addr.sin_addr.s_addr = get_local_ip();
    local_addr.sin_port = htons(51111);

    if (bind(sock, (struct sockaddr *) &local_addr, sizeof(local_addr)))
    {
        printf("###bind err 2: %s\n", strerror(errno)); // 输出错误信息
        return PTS_FAIL;
    }
    
    if (listen(sock, 5))//5代表最大连接数，代表服务端可以工作了，做好只要有客户端请求，就可以派发相应操作的准备
    {
        printf("###listen err 3\n");
        return PTS_FAIL;
    }

    /* Create a new thread */
    if (pthread_create(&new_th, NULL, tcp_client, NULL) != 0)
    {
        printf("Error creating thread\n");
        return PTS_UNRESOLVED;
    }

    clientAddrLen = sizeof(clientAddr);

    // Wait for the client to connect
    pthread_mutex_lock(&mutex);
    while (!ready) {
        pthread_cond_wait(&cond, &mutex); // Wait until the client is ready
    }
    pthread_mutex_unlock(&mutex);

    subSock = accept(sock, (struct sockaddr *)&clientAddr, &clientAddrLen);

    close(subSock);
    close(sock);
    // 销毁互斥锁和条件变量
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    if (-1 != subSock)
    {
        TEST_OKPRINT();
        return PTS_PASS;
    }
    else
    {
        TEST_ERRPRINT("accept");
        return PTS_FAIL;
    }
}
