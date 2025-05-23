#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>
#include <utime.h>
#include <errno.h>
#include <fcntl.h>
int NET_Listen_ENOTSOCK()
{
    int nSockFd = 0;
    int flag = 0;
    int ret = 0;
    int sock1 = -1;
    int fd;
    char path[50] =FS_ROOT;

    strcat(path, "/test333.txt");
    remove(path);

    fd = open(path,O_RDWR|O_CREAT);
    if (fd < 0)
    {
        printf("open fail\n");
        return -1;
    }

    ret = listen(fd, 5);
    if (ret != -1 || errno != ENOTSOCK)
    {
        TEST_FAILPRINT("listen");
        remove(path);
        flag = 1;
        return PTS_FAIL;
    }
    close(nSockFd);

    /*判断测试结果*/
    if (flag == 1)
    {
        TEST_FAILPRINT("listen");
        remove(path);
        return PTS_FAIL;
    }
    else
    {
        TEST_OKPRINT();
        remove(path);
        return PTS_PASS;
    }
}

