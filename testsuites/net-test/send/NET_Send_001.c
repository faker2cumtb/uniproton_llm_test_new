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
#define SEND_STR "hello"

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static int ready = 0;
static int subready = 0;

static int iRet = 0;
static int sock = -1;
static random_port = 0;

static void *tcp_server(void *arg1)
{
    int nSockFd, client_fd;
    struct sockaddr_in local_addr, remote_addr;
    int len, nDataLen;

    nSockFd = socket(PF_INET, SOCK_STREAM, 0);
    if (nSockFd < 0)
    {
        printf("###sock err 1\n");
    }

    local_addr.sin_family = PF_INET;//网络传输
    local_addr.sin_addr.s_addr = get_local_ip();//服务端ip
    local_addr.sin_port = htons(57755);//本机的转为网络的端口号

    if (bind(nSockFd, (struct sockaddr *) &local_addr, sizeof(local_addr)))
    {
        printf("###sock err 2\n");
    }

    if (listen(nSockFd, 5))
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
    if ( -1 == client_fd )
    {
    	TEST_FAILPRINT("SEND TEST FAILED");
    }

    // Wait for the client to send
    pthread_mutex_lock(&mutex);
    while (!subready) {
        pthread_cond_wait(&cond, &mutex); // Wait until the client is ready
    }
    pthread_mutex_unlock(&mutex);

    nDataLen = recv(client_fd, recvBuff, BUFF_SIZE, MSG_OOB);
    if (nDataLen <= 0)
    {
        printf("recv error: %d\n", nDataLen);
    }

	close(nSockFd);
	close(client_fd);
}

int NET_Send_001(void)
{
    struct sockaddr_in servAddr;
    pthread_t new_th;
    char buf[256] = "send test";
	struct timespec ts;

    


    servAddr.sin_family = PF_INET;
    servAddr.sin_addr.s_addr = get_local_ip();
    servAddr.sin_port = htons(57755);
  
    /* Create a new thread */
    pthread_create(&new_th, NULL, tcp_server, NULL);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    //让tcp_server运行
    ts.tv_sec = 1;			//0s
    ts.tv_nsec = 20000;	
    nanosleep(&ts,NULL);

    connect(sock, (struct sockaddr *)&servAddr, sizeof(struct sockaddr));

    // Signal that the client has connected
    pthread_mutex_lock(&mutex);
    ready = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    iRet = send(sock, SEND_STR, sizeof(SEND_STR), MSG_OOB);

    // Signal that the client has sended
    pthread_mutex_lock(&mutex);
    subready = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

	close(sock);
    // 销毁互斥锁和条件变量
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    
    if (iRet == sizeof(SEND_STR))
    {
        TEST_OKPRINT();
        return PTS_PASS;
    }
    else
    {
        TEST_FAILPRINT("SEND TEST FAILED");
        return 0;
    }
}



