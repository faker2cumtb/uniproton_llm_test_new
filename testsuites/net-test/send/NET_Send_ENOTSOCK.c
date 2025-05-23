#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>
#include <utime.h>
#include <errno.h>
#include <fcntl.h>

#define ROUND (100*1024)
#define BUFF_SIZE 1460
static uint8_t  sendBuff[BUFF_SIZE];
static uint8_t  recvBuff[BUFF_SIZE];
 
 
#define get_local_ip() inet_addr(server_IP)

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static int ready = 0;

static int iRet = 0;
static int sock = -1;
static random_port = 0;

static void *tcp_server(void *arg1)
{
    int nSockFd, client_fd;
    struct sockaddr_in local_addr, remote_addr;
    uint32_t i;
    int len, nDataLen;
    
    nSockFd = socket(PF_INET, SOCK_STREAM, 0);

    local_addr.sin_family = PF_INET;//网络传输
    local_addr.sin_addr.s_addr = get_local_ip();//服务端ip
    local_addr.sin_port = htons(57766);//本机的转为网络的端口号

    bind(nSockFd, (struct sockaddr *) &local_addr, sizeof(local_addr));

    listen(nSockFd, 5);//5代表最大连接数，代表服务端可以工作了，做好只要有客服端请求，就可以派发相应操作的 准备

    printf("listen('%d.%d.%d.%d', %d)\n",
           local_addr.sin_addr.s_addr & 0xFF,
           (local_addr.sin_addr.s_addr >> 8) & 0xFF,
           (local_addr.sin_addr.s_addr >> 16) & 0xFF,
           (local_addr.sin_addr.s_addr >> 24) & 0xFF,
           ntohs(local_addr.sin_port));

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

int NET_Send_ENOTSOCK(void)
{
	char buf[256] = "send test";
	int fd, ret;
    char path[50] =FS_ROOT;
    struct sockaddr_in servAddr;
    pthread_t new_th;
	struct timespec ts;
    strcat(path, "/t3est12.txt");
    remove(path);
    fd = open(path,O_RDWR|O_CREAT);
    if (fd < 0)
    {
        printf("open fail\n");
        return -1;
    }

    


    servAddr.sin_family = PF_INET;
    servAddr.sin_addr.s_addr = get_local_ip();
    servAddr.sin_port = htons(57766);


    /* Create a new thread */
    pthread_create(&new_th, NULL, tcp_server, NULL);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    ts.tv_sec = 0;			//0s
    ts.tv_nsec = 20000;	//20ms
    nanosleep(&ts,NULL);

    connect(sock, (struct sockaddr *)&servAddr, sizeof(struct sockaddr));

    // Signal that the client has connected
    pthread_mutex_lock(&mutex);
    ready = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    close(sock);
    iRet = send(fd, buf, sizeof(buf), MSG_OOB);

    if ((iRet == -1) && (errno == ENOTSOCK))
    {
		close(sock);
        remove(path);
        // 销毁互斥锁和条件变量
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
        TEST_OKPRINT();
        return NULL;
    }
    else
    {
		close(sock);
        remove(path);
        // 销毁互斥锁和条件变量
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
        TEST_FAILPRINT("SEND TEST FAILED");
        return NULL;
    }
}



