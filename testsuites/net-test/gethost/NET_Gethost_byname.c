#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>

int NET_Gethost_byname() {
    const char *hostname = host_IP; // 要查询的主机名
    struct hostent *host_info;

    // 查询主机名，使用 AF_INET 表示 IPv4
    host_info = gethostbyname(hostname);
    if (host_info == NULL) {
        printf("gethostbyname error\n");
        return 1;
    }

    // 打印主机信息
    printf("Host name: %s\n", host_info->h_name);

    // 打印 IP 地址
    char **addr_list = host_info->h_addr_list;
    printf("IP addresses:\n");
    for (int i = 0; addr_list[i] != NULL; i++) {
        struct in_addr addr;
        memcpy(&addr, addr_list[i], sizeof(struct in_addr));
        printf("  %s\n", inet_ntoa(addr));
    }

    // 打印别名
    if (host_info->h_aliases) {
        printf("Aliases:\n");
        for (char **alias = host_info->h_aliases; *alias != NULL; alias++) {
            printf("  %s\n", *alias);
        }
    }

    return 0;
}