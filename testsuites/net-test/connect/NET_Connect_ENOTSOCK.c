
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>
#include <utime.h>
#include <errno.h>
#include <fcntl.h>

int NET_Connect_ENOTSOCK(int argv[])
{
    int iRet = 0;
    int sock = -1;
    int isOk = 0;
    struct sockaddr_in servAddr;
    int failed = 0;
    int fd, ret;
    char path[50] =FS_ROOT;

    strcat(path, "/test.txt");
    remove(path);
    //    fd = creat(path, 0777);
    fd = open(path,O_RDWR|O_CREAT);
    if (fd < 0)
    {
        printf("open fail\n");
        return -1;
    }

    iRet = connect(fd, (struct sockaddr *)&servAddr, sizeof(struct sockaddr));
    if (-1 == iRet && errno == ENOTSOCK)
    {

        isOk = 1;
    }
	
    close(sock);

    if (isOk)
    {
        TEST_OKPRINT();
        remove(path);
        return PTS_PASS;
    }
    else
    {
        printf("iRet =%d errno=%d\n", iRet, errno);
        remove(path);
        TEST_FAILPRINT("CONNECT TEST FAILED");
        return PTS_FAIL;
    }
}

