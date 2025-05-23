#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>

static int isOk = 0;
#define ROUND (100*1024)
 
#define BUFF_SIZE 1460
#define get_local_ip() inet_addr(server_IP)

static int iRet = 0;
static int sock = -1;
static int server_ready = 0;

static uint8_t  sendBuff[BUFF_SIZE];
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
    struct timespec ts;
    
    nSockFd = socket(PF_INET, SOCK_STREAM, 0);
    if (nSockFd < 0)
    {
        printf("###sock err 1\n");
    }

    local_addr.sin_family = PF_INET;//网络传输
    local_addr.sin_addr.s_addr =inet_addr(server_IP);//服务端ip
    local_addr.sin_port = htons(54333);//本机的转为网络的端口号

    if (bind(nSockFd, (struct sockaddr *) &local_addr, sizeof(local_addr)))
    {
        printf("###bind err : %s\n", strerror(errno));
    }

    if (listen(nSockFd, 5))//5代表最大连接数，代表服务端可以工作了，做好只要有客服端请求，就可以派发相应操作的 准备
    {
        printf("###listen err : %s\n", strerror(errno));
    }
    
    len = sizeof(remote_addr);

    // Wait for the client to connect
    pthread_mutex_lock(&mutex);
    while (!ready) {
        pthread_cond_wait(&cond, &mutex); // Wait until the client is ready
    }
    pthread_mutex_unlock(&mutex);

    client_fd = accept(nSockFd, (struct sockaddr *) &remote_addr, (socklen_t *) &len);//remote_addr客服端的地址，服务端接受一个请求服务
    if (client_fd < 0) {
        printf("###accept err : %s\n", strerror(errno));
        close(nSockFd);
    }

	close(nSockFd);
    close(client_fd);
    return NULL;

}

int NET_Connect_001(void)
{
	int fd;
	pthread_t new_th;
    struct sockaddr_in servAddr;
	struct timespec ts;

    


    servAddr.sin_family = PF_INET;
    servAddr.sin_addr.s_addr =get_local_ip();
    servAddr.sin_port = htons(54333);
  
    /* Create a new thread */
    if (pthread_create(&new_th, NULL, tcp_server, NULL) != 0)
    {
        printf("Error creating thread\n");
        return PTS_UNRESOLVED;
    }

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == fd)
    {
        TEST_FAILPRINT("socket");
        return PTS_UNRESOLVED;
    }

    /* 让tcp_server先运行 */
    ts.tv_sec = 0;			//0s
    ts.tv_nsec = 20000;	
    nanosleep(&ts,NULL);
    
    iRet = connect(fd, (struct sockaddr *)&servAddr, sizeof(struct sockaddr));

    // Signal that the client has connected
    pthread_mutex_lock(&mutex);
    ready = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    if (iRet)
    {
        TEST_FAILRINT();
        close(fd);
        // 销毁互斥锁和条件变量
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
        return PTS_FAIL;
    }
    else
    {
        TEST_OKPRINT();
        close(fd);
        // 销毁互斥锁和条件变量
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
        return PTS_PASS;
    }
}


