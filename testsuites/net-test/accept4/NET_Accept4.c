#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static int ready = 0;
static int subready = 0;
static random_port = 0;

// 服务器函数
void *server_function(void *arg) {
    int server_fd, client_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    int opt = 1;

    // 创建监听套接字
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 设置套接字选项
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 设置地址和端口
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(51123);

    // 绑定套接字
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 开始监听
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", address.sin_port);

    // Wait for the client to connect
    pthread_mutex_lock(&mutex);
    while (!ready) {
        pthread_cond_wait(&cond, &mutex); // Wait until the client is ready
    }
    pthread_mutex_unlock(&mutex);

    // 接受客户端连接，使用 accept4
    client_fd = accept4(server_fd, (struct sockaddr *)&address, &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (client_fd < 0) {
        perror("accept4 failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Accepted connection from client\n");

    // 接收消息
    char buffer[1024] = {0};

    // Wait for the client to send
    pthread_mutex_lock(&mutex);
    while (!subready) {
        pthread_cond_wait(&cond, &mutex); // Wait until the client is ready
    }
    pthread_mutex_unlock(&mutex);

    ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0'; // 确保字符串以 null 结尾
        printf("Received message: %s\n", buffer);
    } else if (bytes_read == 0) {
        // 连接已关闭
        printf("Connection closed by the client.\n");
    } else {
        // 处理错误
        perror("recv failed");
    }

    // 关闭套接字
    close(client_fd);
    close(server_fd);
    return NULL;
}

int NET_Accept4() {
    pthread_t server_thread;
    int ret = 0;

    


    // 创建服务器线程
    pthread_create(&server_thread, NULL, server_function, NULL);

    sleep(1); // 确保服务器已经启动
    int sock = 0;
    struct sockaddr_in serv_addr;

    // 创建套接字
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        ret = 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(51123);

    // 转换 IPv4 和 IPv6 地址从文本到二进制
    if (inet_pton(AF_INET, server_IP, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        ret = 1;
    }

    // 连接到服务器
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        ret = 1;
    }

    // Signal that the client has connected
    pthread_mutex_lock(&mutex);
    ready = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    // 发送消息
    const char *message = "Hello from client!";
    ssize_t bytes_sent =send(sock, message, strlen(message), 0);
    if (bytes_sent < 0) {
    perror("send failed");
    ret = 1;
    }

    // Signal that the client has sended
    pthread_mutex_lock(&mutex);
    subready = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    // 关闭套接字
    close(sock);
    // 销毁互斥锁和条件变量
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    if (ret == 0)
    {
        TEST_OKPRINT();
        return PTS_PASS;
    }
    else
    {
        TEST_ERRPRINT("accept4");
        return PTS_FAIL;
    }
}