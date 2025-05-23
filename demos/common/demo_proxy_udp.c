#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <net/if.h>
#include <sys/uio.h>
#include <demo.h>

#define MAXLINE 100
#define UDPPORT 8888

static void build_addr(struct in_addr *addr, int b1, int b2,
                       int b3, int b4)
{
    char *cs = (char *)addr;

    cs[0] = b1 & 0xff;
    cs[1] = b2 & 0xff;
    cs[2] = b3 & 0xff;
    cs[3] = b4 & 0xff;
}

/* udp client */
int proxy_udp_demo()
{
    int cfd = 0;
    ssize_t sret = 0;
    socklen_t salen = 0;
    char str[] = "hello server! from proxy udp demo";
    char buf[MAXLINE];
    struct sockaddr_in DstAddr;
    struct sockaddr_in SrcAddr;
    int opt = 1;

    if((cfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        printf("socket() error");
        return -1;
    }

    salen = sizeof(struct sockaddr_in);

    // // 设置本地地址 192.168.10.10:8888
    SrcAddr.sin_family = AF_INET;
    SrcAddr.sin_addr.s_addr = INADDR_ANY;
    SrcAddr.sin_port = htons(UDPPORT);

    // 绑定 socket 到端口
    if (setsockopt(cfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        printf("setsockopt() error");
        close(cfd);
        return -1;
    }
    // 绑定
    if (bind(cfd, (struct sockaddr *)&SrcAddr, sizeof(SrcAddr)) < 0) {
        printf("bind() error");
        close(cfd);
        return -1;
    }

    // 设置目标地址 192.168.10.100:8888
    memset(&DstAddr, 0, salen);
    DstAddr.sin_family = AF_INET;
    DstAddr.sin_port = htons(UDPPORT);
    build_addr(&DstAddr.sin_addr , 192, 168, 10, 100);

    sret = sendto(cfd, str, sizeof(str), 0, (struct sockaddr *)&DstAddr, salen);
    printf("sendto ret: %ld, addrlen: %d, data: %s\n", sret, salen, str);
    if(sret < 0) {
        printf("sendto() error");
        close(cfd);
        return -1;
    }

    sret = recvfrom(cfd, buf, sizeof(buf), 0, (struct sockaddr *)&DstAddr, &salen);
    buf[sret] = '\0';
    printf("recvfrom ret: %ld, addrlen: %d, data: %s\n", sret, salen, buf);
    if(sret < 0) {
        printf("recvfrom() error");
        close(cfd);
        return -1;
    }

    close(cfd);
    return 0;
}