#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>
#include <utime.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

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

	struct timespec ts;
    nSockFd = socket(PF_INET, SOCK_DGRAM, 0);

    local_addr.sin_family = PF_INET;//网络传输
    local_addr.sin_addr.s_addr = get_local_ip();//服务端ip
    local_addr.sin_port = htons(57744);//本机的转为网络的端口号

    ts.tv_sec = 0;			//0s
    ts.tv_nsec = 20000;	//20ms
    nanosleep(&ts,NULL);

    for (i = 0; i < BUFF_SIZE; i++)
    {
        sendBuff[i] = i % 26 + 'A';
    }

    nDataLen = sendto(nSockFd, sendBuff,8, 0, (struct sockaddr *)&local_addr, sizeof(struct sockaddr_in));
    // Signal that the client has connected
    pthread_mutex_lock(&mutex);
    ready = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    close(nSockFd);
   
    return NULL;
}

int NET_Recvfrom_ENOTSOCK(void)
{
    struct sockaddr_in servAddr;
    int len;
    pthread_t new_th;    
    int fd, ret;
    char path[50] =FS_ROOT;

    strcat(path, "/test1u2.txt");
    remove(path);
    fd = open(path,O_RDWR|O_CREAT);
    if (fd < 0)
    {
        printf("open fail\n");
        return -1;
    }

    


    servAddr.sin_family = PF_INET;
    servAddr.sin_addr.s_addr =get_local_ip();
    servAddr.sin_port = htons(57744);
   
    /* Create a new thread */
    pthread_create(&new_th, NULL, udp_server, NULL);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    bind(sock, (struct sockaddr *) &servAddr, sizeof(servAddr));

    len = sizeof(servAddr);
	close(sock);

    // Wait for the client to connect
    pthread_mutex_lock(&mutex);
    while (!ready) {
        pthread_cond_wait(&cond, &mutex); // Wait until the client is ready
    }
    pthread_mutex_unlock(&mutex);

    iRet = recvfrom(fd, recvBuff, BUFF_SIZE, 0, (struct sockaddr *) &servAddr, (socklen_t *) &len);
    
    if ((iRet == -1) && (errno == ENOTSOCK))
    {
		close(sock);
        // 销毁互斥锁和条件变量
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
        remove(path);
        TEST_OKPRINT();
        return PTS_PASS;
    }
    else
    {
		close(sock);
        // 销毁互斥锁和条件变量
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
        remove(path);
        TEST_FAILPRINT("RECVFROM TEST FAILED");
        return PTS_FAIL;
    }
}



