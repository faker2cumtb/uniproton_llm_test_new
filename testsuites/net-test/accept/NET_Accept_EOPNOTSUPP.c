#define __SYLIXOS_KERNEL
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>

#define get_local_ip() inet_addr(server_IP)
static random_port = 0;

int NET_Accept_EOPNOTSUPP(void) {
    int s;
    struct sockaddr_in srvr;
    socklen_t addrLen = sizeof(srvr);
    
    // 创建一个 UDP 套接字
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == -1) {
        printf("socket error!\n");
        return -1;
    } 

    // 绑定套接字
    memset(&srvr, 0, sizeof(srvr));
    srvr.sin_family = AF_INET;
    srvr.sin_port = htons(51112);
    srvr.sin_addr.s_addr = inet_addr(server_IP);

    int ret = bind(s, (struct sockaddr *)&srvr, sizeof(srvr));
    if (ret == -1) {
        printf("bind error!\n");
        close(s);
        return -1;
    } 

    // 尝试接受连接，这里应该返回 EOPNOTSUPP
    int newSock = accept(s, NULL, NULL);
    if (newSock == -1) {
        if (errno == EOPNOTSUPP) {
            TEST_OKPRINT();
            close(s); // 关闭套接字
            return PTS_PASS; // 成功捕获 EOPNOTSUPP 错误
        } else {
            TSTDEF_ERRPRINT(errno);
            close(s);
            return PTS_FAIL; // 捕获到其他错误
        }
    }

    // 关闭套接字
    close(s);
    return PTS_FAIL;
}
