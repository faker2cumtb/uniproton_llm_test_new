#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>

int NET_Gethost_name() {
    char hostname[256]; // 用于存储主机名
    size_t len = sizeof(hostname); // 主机名缓冲区的大小

    // 调用 gethostname
    if (gethostname(hostname, len) == -1) {
        perror("gethostname error");
        return 1;
    }

    // 输出主机名
    printf("Current hostname: %s\n", hostname);

    return 0;
}