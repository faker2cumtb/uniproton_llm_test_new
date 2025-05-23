#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>

 
#define get_local_ip() inet_addr(server_IP)

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static int ready = 0;
static random_port = 0;

static void *tcp_server(void *arg)
{
    int nSockFd,client_fd,len;
    int status;
    struct sockaddr_in remote_addr;
    int nDataLen;
    uint32_t i;

    nSockFd = socket(PF_INET, SOCK_STREAM, 0);
    if (nSockFd < 0)
    {
        printf("###sock err 1\n");
        return (void *) -4;
    }

    memset(&remote_addr, 0, sizeof(remote_addr));

    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = get_local_ip();
    remote_addr.sin_port = htons(56666);
    status = bind(nSockFd, (struct sockaddr *)&remote_addr, sizeof(remote_addr));

    status = listen(nSockFd, 1024);
    if (status != 0) {
        printf("listen failed.\n");
        return PTS_FAIL;
    }

    len = sizeof(remote_addr);

    // Wait for the client to connect
    pthread_mutex_lock(&mutex);
    while (!ready) {
        pthread_cond_wait(&cond, &mutex); // Wait until the client is ready
    }
    pthread_mutex_unlock(&mutex);

    client_fd = accept(nSockFd, (struct sockaddr *) &remote_addr, (socklen_t *) &len);//remote_addr客服端的地址，服务端接受一个请求服务
}

int NET_Getsockname_001()
{

    struct sockaddr_in remote_addr, addr;
    socklen_t addr_len = sizeof(addr);
    int ret;
    int status;    pthread_t new_th;
    struct timespec ts;

    


    /* Create a new thread */
    if (pthread_create(&new_th, NULL, tcp_server, NULL) != 0)
    {
        perror("Error creating thread\n");
        return PTS_UNRESOLVED;
    }

    int nSockFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (nSockFd < 0)
    {
        printf("create socket error, errno=%d\n", errno);
        return PTS_FAIL;
    }

    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = get_local_ip();
    remote_addr.sin_port = htons(56666);

    printf("connect('%d.%d.%d.%d', %d)\n",
            remote_addr.sin_addr.s_addr & 0xFF,
           (remote_addr.sin_addr.s_addr >> 8) & 0xFF,
           (remote_addr.sin_addr.s_addr >> 16) & 0xFF,
           (remote_addr.sin_addr.s_addr >> 24) & 0xFF,
           ntohs(remote_addr.sin_port) );

    ts.tv_sec = 1;			//1s
    ts.tv_nsec = 20000;	//20ms
    nanosleep(&ts,NULL);

    status = connect(nSockFd, (struct sockaddr *) &remote_addr, sizeof(remote_addr));

    // Signal that the client has connected
    pthread_mutex_lock(&mutex);
    ready = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    ret = getsockname(nSockFd, (struct sockaddr *)&addr, &addr_len);

    close(nSockFd);
    // 销毁互斥锁和条件变量
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    if (ret == 0) {
        TEST_OKPRINT();
        return PTS_PASS;
    } else {
        return PTS_FAIL;
        TEST_FAILRINT()
    }
}
