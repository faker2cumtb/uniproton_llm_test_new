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

static void *udp_server(void *arg1)
{
    int nSockFd, client_fd;

    struct sockaddr_in local_addr, remote_addr;
    uint32_t i;
    int len, nDataLen;
    char buf[256] = "send test";

    nSockFd = socket(PF_INET, SOCK_DGRAM, 0);

    local_addr.sin_family = PF_INET;//网络传输
    local_addr.sin_addr.s_addr = get_local_ip();//服务端ip
    local_addr.sin_port = htons(57799);//本机的转为网络的端口号

    if (bind(nSockFd, (struct sockaddr *) &local_addr, sizeof(local_addr)))
    {
    	printf("server bind err\n");
    }

    for (i = 0; i < BUFF_SIZE; i++)
    {
        sendBuff[i] = i % 26 + 'A';
    }

    // Wait for the client to connect
    pthread_mutex_lock(&mutex);
    while (!ready) {
        pthread_cond_wait(&cond, &mutex); // Wait until the client is ready
    }
    pthread_mutex_unlock(&mutex);

    nDataLen = recvfrom(nSockFd, recvBuff, BUFF_SIZE, 0, (struct sockaddr *) &local_addr, (socklen_t *) &len);
    if (nDataLen <= 0)
    {
        printf("recvfrom error: %d,%d byte\n", nDataLen, i);
    }

	close(nSockFd);
    return NULL;
}

int NET_Sendto_001(void)
{
    struct sockaddr_in servAddr;
    pthread_t new_th;
    char buf[256] = "send test";
	struct timespec ts;

    


    servAddr.sin_family = PF_INET;
    servAddr.sin_addr.s_addr = get_local_ip();
    servAddr.sin_port = htons(57799);
  
    // /* Create a new thread */
    // pthread_create(&new_th, NULL, udp_server, NULL);

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    ts.tv_sec = 0;			//0s
    ts.tv_nsec = 20000;	//20ms
    nanosleep(&ts,NULL);

    /* Create a new thread */
    pthread_create(&new_th, NULL, udp_server, NULL);

    iRet = sendto(sock, buf, strlen(buf), 0, (struct sockaddr *)&servAddr, sizeof(struct sockaddr_in));
    // Signal that the client has connected
    pthread_mutex_lock(&mutex);
    ready = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

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
        TEST_FAILPRINT("SENDTO TEST FAILED");
        return PTS_FAIL;
    }
}


