#define __SYLIXOS_KERNEL

#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>

static random_port = 0;

int NET_Accept_ENOTSOCK(void)
{
    int s, on = 1;
    int ret1 = -2;
    int ret2 = -2;
    int ret3 = -2;
    int ret4 = -2;
    int ret5 = -2;
    int ret6 = -2;
    int newSock;

    struct sockaddr_in addr;

    int addrLen;
    char recvbuf[1024] = {0};
    int buf_len = sizeof(recvbuf);
    char sendbuf[] = "This is a message from 192.168.137.10";
    int buf_len1 = sizeof(sendbuf);
    struct sockaddr_in srvr;

    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == -1)
    {
        printf( "socket error!\n");
        return PTS_FAIL;
    }

    ret1 = setsockopt( s, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on) );
    if (ret1 == -1)
    {
        printf( "setsockopt error!\n");
        return PTS_FAIL;
    }

    int size = sizeof(on);
    ret6 = getsockopt( s, SOL_SOCKET, SO_REUSEADDR, (char *)&on, &size );
    if (ret6 == -1)
    {
        printf( "getsockopt error!\n");
        return PTS_FAIL;
    }

    memset(&srvr, 0, sizeof(srvr));

    


    srvr.sin_family = AF_INET;
    srvr.sin_port = htons(52222);
    srvr.sin_addr.s_addr = inet_addr(server_IP);
    ret2 = bind(s, (struct sockaddr *)&srvr, sizeof(srvr));
    if ( ret2 == -1)
    {
        printf("bind err: %s\n", strerror(errno));
        return PTS_FAIL;
    }

    ret3 = listen(s, 10);
    if (ret3 == -1)
    {
        printf( "listen error!\n");
        return PTS_FAIL;
    }

    memset(&addr, 0, sizeof(addr));

    addrLen = sizeof (struct sockaddr);
    int s1 = 0;
    newSock = accept(s1,  &addr, &addrLen);//ENOTSOCK 输入参数 s 不是 socket 端口

    if (newSock == -1)
    {
        if (ENOTSOCK ==  errno)
        {
        	TEST_OKPRINT();
            close(newSock);
            close(s);
            return PTS_PASS;
        }
        else
        {
            TSTDEF_ERRPRINT(errno);
            close(newSock);
            close(s);
            return PTS_FAIL;
        }
    }
    else
    {
        printf( "accept success!\n");
        TSTDEF_ERRPRINT(errno);
        close(newSock);
        close(s);
        return PTS_FAIL;
    }
}
