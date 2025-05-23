#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>

int NET_Gethost_addr() {
    const char *ip_address = host_IP; // 要查询的 IP 地址
    struct in_addr addr;
    struct hostent *host_info;

    // 将字符串 IP 地址转换为 in_addr 结构
    if (inet_pton(AF_INET, ip_address, &addr) <= 0) {
        fprintf(stderr, "Invalid IP address: %s\n", ip_address);
        return 1;
    }

    // 调用 gethostbyaddr
    host_info = gethostbyaddr(&addr, sizeof(addr), AF_INET);
    if (host_info == NULL) {
        printf("host_info err\n");
        return 1;
    }

    // 打印主机信息
    printf("Host name: %s\n", host_info->h_name);
    printf("IP address: %s\n", ip_address);

    // 打印别名
    for (char **alias = host_info->h_aliases; *alias != NULL; alias++) {
        printf("Alias: %s\n", *alias);
    }

    return 0;
}