#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>
#include <utime.h>
#include <errno.h>
#include <fcntl.h>

extern int remove(const char *_file);

int NET_Getsockopt_ENOTSOCK()
{
    int iRet = 0;
    int isOk = 0;
    int sock;
    int bOOBLine = 1, len = 16;
  	int fd, ret;
    char path[50] =FS_ROOT;

    strcat(path, "/a92.txt");
    remove(path);
    fd = open(path,O_RDWR|O_CREAT);
    if (fd < 0)
    {
        printf("open fail\n");
        return -1;
    }

    iRet = getsockopt(fd, SOL_SOCKET, SO_OOBINLINE, (char *)&bOOBLine, (socklen_t *)&len);
    if ((-1 == iRet) && (errno == ENOTSOCK))
    {
        isOk = 1;
    }
    else
    {
        isOk = 0;
    }

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
        TEST_FAILPRINT("getsockopt");
        return PTS_FAIL;
    }
}

