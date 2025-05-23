#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>


#define BUFF_SIZE 1460

static uint8_t  sendBuff[BUFF_SIZE];
static uint8_t  RecvBuff[BUFF_SIZE];

#define get_local_ip() inet_addr(server_IP)
static uint8_t RecvBuff[BUFF_SIZE];
static int sock = -1;
static int iRet = 0;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static volatile int server_ready = 0; // 服务器是否准备好的标志

printf_thread(pthread_t mthread, const char *str){
    printf("%s information: ", str);
    int policy;
    struct sched_param param;
    
    if(pthread_getschedparam(mthread, &policy, &param)){
        printf("get params error");
    }

    // 打印调度策略
    printf("Scheduling policy: ");
    switch (policy) {
        case SCHED_FIFO:
            printf("SCHED_FIFO\n");
            break;
        case SCHED_RR:
            printf("SCHED_RR\n");
            break;
        case SCHED_OTHER:
            printf("SCHED_OTHER\n");
            break;
        default:
            printf("Unknown\n");
    }

    // 打印调度优先级
    printf("Scheduling priority: %d\n", param.sched_priority);

}

static void *tcp_server(void *arg1)
{
    printf_thread(pthread_self(), "tcp_server");
    int nSockFd, client_fd;
    struct sockaddr_in local_addr, remote_addr;
    int len, nDataLen;

    // 创建套接字
    nSockFd = socket(PF_INET, SOCK_STREAM, 0);
    if (nSockFd < 0) {
        perror("Socket creation failed");
        return NULL;
    }

    // 绑定地址和端口
    local_addr.sin_family = PF_INET;
    local_addr.sin_addr.s_addr = get_local_ip(); // 服务器 IP
    local_addr.sin_port = htons(56633); // 服务器端口

    if (bind(nSockFd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        perror("Bind failed");
        close(nSockFd);
        return NULL;
    }

    // 监听连接
    if (listen(nSockFd, 5) < 0) {
        perror("Listen failed");
        close(nSockFd);
        return NULL;
    }

    printf("Server is listening on port 56633...\n");

    // 接受客户端连接
    len = sizeof(remote_addr);
    // 服务器已准备好，通知主线程
    server_ready = 1;
    client_fd = accept(nSockFd, (struct sockaddr *)&remote_addr, (socklen_t *)&len);
    if (client_fd < 0) {
        perror("Accept failed");
        close(nSockFd);
        return NULL;
    }

    printf("Client connected.\n");
    // 接收和发送数据
    while (1) {
        nDataLen = recv(client_fd, RecvBuff, BUFF_SIZE, 0);
        if (nDataLen <= 0) {
            break;
        }
        send(client_fd, RecvBuff, nDataLen, 0); // 回显数据
        usleep(1000); // 模拟处理延迟
    }

    // 关闭套接字
    close(client_fd);
    close(nSockFd);
    return NULL;
}

int NET_Recv_EBADF(void)
{
    printf_thread(pthread_self(), "NET_Recv_EBADF");
    return 1;
    int sock_err = 0;
    struct sockaddr_in servAddr;
    pthread_t new_th;
    char *buffer;

    servAddr.sin_family = PF_INET;
    servAddr.sin_addr.s_addr = get_local_ip();
    servAddr.sin_port = htons(56633);
 
    buffer = (char *) malloc (BUFF_SIZE * sizeof(char));
    if (NULL == buffer)
    {
		close(sock);
        TEST_FAILPRINT("RECV TEST FAILED");
        return PTS_FAIL;
    }

    /* Create a new thread */
    pthread_create(&new_th, NULL, tcp_server, NULL);
    printf("--------start--------");
   
    while (!server_ready) {
        usleep(1000); // 等待条件变量信号，同时释放互斥锁
    }
    
    printf("Server is ready. Proceeding with client connection...\n");

    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    connect(sock, (struct sockaddr *)&servAddr, sizeof(struct sockaddr));

    close(sock);

    iRet = recv(sock, buffer, BUFF_SIZE, MSG_OOB);
    if ((iRet == -1) && (errno == EBADF))
    {
        // 销毁互斥锁和条件变量
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
        TEST_OKPRINT();
        return PTS_PASS;
    }
    else
    {
        // 销毁互斥锁和条件变量
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
        TEST_FAILPRINT("RECV TEST FAILED");
        return PTS_FAIL;
    }
}