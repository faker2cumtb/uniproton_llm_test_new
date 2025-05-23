#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>
#include <net/if.h>

int NET_IF_Nameindex() {
    struct if_nameindex *interfaces;
    struct if_nameindex *iface;

    // 调用 if_nameindex 获取接口信息
    interfaces = if_nameindex();
    if (interfaces == NULL) {
        perror("if_nameindex error");
        return 1;
    }

    // 遍历接口信息并打印
    for (iface = interfaces; iface->if_name != NULL; iface++) {
        printf("Interface index: %u, Name: %s\n", iface->if_index, iface->if_name);
    }

    // 释放接口信息数组
    free(interfaces);

    return 0;
}