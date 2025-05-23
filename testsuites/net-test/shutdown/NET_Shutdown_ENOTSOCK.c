#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>
#include <utime.h>
#include <errno.h>
#include <fcntl.h>
extern int remove(const char *_file);

int NET_Shutdown_ENOTSOCK()
{
    int iRet = 0;
    int isOk = 0;
    int sock;
    int fd, ret;
    char path[50] =FS_ROOT;

    strcat(path, "/3912.txt");
    remove(path);
    fd = open(path,O_RDWR|O_CREAT);
    if (fd < 0)
    {
        printf("open fail\n");
        return -1;
    }

    iRet = shutdown(fd, 1);
    if ((-1 == iRet) && ( errno == ENOTSOCK))
    {
        isOk = 1;
    }
    
    
    if (isOk)
    {
        TEST_OKPRINT();
        close(fd);
        remove(path);
        return PTS_PASS;
    }
    else
    {
        printf("iRet =%d errno=%d\n", iRet, errno);
        close(fd);
        remove(path);
        TEST_FAILPRINT("shutdown");
        return PTS_FAIL;
    }
}
