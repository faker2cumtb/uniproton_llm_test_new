#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>

int NET_Addrinfo() {
    const char *hostname = host_IP; // 要解析的主机名
    const char *service = "80";                // 服务名或端口号
    struct addrinfo hints, *res, *p;
    int status;

    // 初始化 hints 结构
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // 支持 IPv4 和 IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP

    // 调用 getaddrinfo
    status = getaddrinfo(hostname, service, &hints, &res);
    if (status != 0) {
        printf("getaddrinfo err:%s\n", strerror(errno));
        return 1;
    }

    // 遍历 addrinfo 链表并打印结果
    printf("Address information for %s:%s:\n", hostname, service);
    for (p = res; p != NULL; p = p->ai_next) {
        char ipstr[INET6_ADDRSTRLEN]; // IPv4 和 IPv6 地址字符串
        void *addr;

        // 获取地址指针
        if (p->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
        }

        // 将地址转换为字符串
        inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
        printf("  %s: %s\n", p->ai_family == AF_INET ? "IPv4" : "IPv6", ipstr);
    }

    // 释放 addrinfo 结构
    freeaddrinfo(res);
    return 0;
}