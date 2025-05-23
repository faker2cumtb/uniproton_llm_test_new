#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>

int flag = 0;

int NET_Socket_EMFILE() {
    int sock;
    int count = 0;

    while (1) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            if (errno == EMFILE) {
                TEST_OKPRINT();
                flag = 0;
                break; 
            } else {
                TSTDEF_FAILPRINT(errno);
                flag = 1;
                break; // 捕获到其他错误
            }
        }

        // 逐步增加套接字数量
        count++;
        
    }

    printf("Successfully created socket %d\n", count);
    // 关闭所有已创建的套接字
    for (int i = 0; i < count; i++) {
        close(sock);
    }

    return flag;
}