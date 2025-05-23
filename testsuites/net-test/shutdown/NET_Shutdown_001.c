#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>

#define ROUND (100*1024)
#define BUFF_SIZE 1460
static uint8_t  sendBuff[BUFF_SIZE];
static uint8_t  recvBuff[BUFF_SIZE];
 
 
#define get_local_ip() inet_addr(server_IP)
static int iRet = 0;
static int sock = -1;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static int ready = 0;
static random_port = 0;

static void *tcp_server(void *arg1)
{
    int nSockFd, client_fd;
    struct sockaddr_in local_addr, remote_addr;
    uint32_t i;
    int len, nDataLen;
    
    nSockFd = socket(PF_INET, SOCK_STREAM, 0);
    if (nSockFd < 0)
    {
        printf("###sock err 1\n");

    }
   
    local_addr.sin_family = PF_INET;//网络传输
    local_addr.sin_addr.s_addr = get_local_ip();//服务端ip
    local_addr.sin_port = htons(59933);//本机的转为网络的端口号

    if (bind(nSockFd, (struct sockaddr *) &local_addr, sizeof(local_addr)))
    {
        printf("###sock err 2\n");

    }

    if (listen(nSockFd, 5))//5代表最大连接数，代表服务端可以工作了，做好只要有客服端请求，就可以派发相应操作的 准备
    {
        printf("###sock err 3\n");

    }
    printf("listen('%d.%d.%d.%d', %d)\n",
           local_addr.sin_addr.s_addr & 0xFF,
           (local_addr.sin_addr.s_addr >> 8) & 0xFF,
           (local_addr.sin_addr.s_addr >> 16) & 0xFF,
           (local_addr.sin_addr.s_addr >> 24) & 0xFF,
           ntohs(local_addr.sin_port) );

    len = sizeof(remote_addr);

    // Wait for the client to connect
    pthread_mutex_lock(&mutex);
    while (!ready) {
        pthread_cond_wait(&cond, &mutex); // Wait until the client is ready
    }
    pthread_mutex_unlock(&mutex);

    client_fd = accept(nSockFd, (struct sockaddr *) &remote_addr, (socklen_t *) &len);//remote_addr客服端的地址，服务端接受一个请求服务

	close(nSockFd);
	close(client_fd);
    return NULL;
}

int NET_Shutdown_001()
{
    struct sockaddr_in servAddr;
    pthread_t new_th;
	struct timespec ts;

    


    servAddr.sin_family = PF_INET;
    servAddr.sin_addr.s_addr =get_local_ip();
    servAddr.sin_port = htons(59933);

    /* Create a new thread */
    pthread_create(&new_th, NULL, tcp_server, NULL);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    ts.tv_sec = 1;			//0s
    ts.tv_nsec = 20000;	//20ms
    nanosleep(&ts,NULL);

    iRet = connect(sock, (struct sockaddr *)&servAddr, sizeof(struct sockaddr));

    // Signal that the client has connected
    pthread_mutex_lock(&mutex);
    ready = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    iRet = shutdown(sock, SHUT_RDWR);

	close(sock);
    // 销毁互斥锁和条件变量
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    if (iRet != -1)
    {
        TEST_OKPRINT();
        return PTS_PASS;
    }
    else
    {
    	TSTDEF_FAILPRINT(errno);
        return PTS_FAIL;
    }
}

