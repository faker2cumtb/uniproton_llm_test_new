#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>
#include <utime.h>
#include <errno.h>
#include <fcntl.h>

extern int remove(const char *_file);

int NET_Setsockopt_ENOTSOCK()
{
    int iRet = 0;
    int sock;
    int isOk = 0;
    int bOOBLine = 1;
    int fd, ret;
    char path[50] =FS_ROOT;

    strcat(path, "/t1est12a.txt");
    remove(path);

    fd = open(path,O_RDWR|O_CREAT);
    if (fd < 0)
    {
        printf("open fail\n");
        return -1;
    }
    
    iRet = setsockopt(fd, SOL_SOCKET, SO_OOBINLINE, (char *)&bOOBLine, sizeof(int));
    if ((-1 == iRet) && (errno == ENOTSOCK))
    {
        isOk = 1;
    }
    else
    {
        isOk = 0;
    }
    
    close(fd);

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
        TEST_FAILPRINT("setsockopt");
        return PTS_FAIL;
    }
}
