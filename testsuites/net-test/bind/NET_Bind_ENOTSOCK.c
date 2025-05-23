#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>
#include <utime.h>
#include <errno.h>
#include <fcntl.h>
 #include <unistd.h>

int NET_Bind_ENOTSOCK(int argv[])
{
    int iRet = 0;
    int sock = -1;
    struct sockaddr_in localAddr;
    int fd, ret;
    char path[50] =FS_ROOT;

    strcat(path, "/test222.txt");
    remove(path);
//    fd = creat(path, 0777);
    fd = open(path, O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR);
    if (fd < 0)
    {
        printf("open fail\n");
        return -1;
    }
    
    iRet = bind(fd, (struct sockaddr *)&localAddr, sizeof(struct sockaddr));
    if (iRet == -1 && errno == ENOTSOCK)
    {
        TEST_OKPRINT();
        remove(path);
        return PTS_PASS;
    }
    else
    {
        printf("iRet =%d errno=%d\n", iRet, errno);
        remove(path);
        TEST_FAILPRINT("bind");
        return PTS_FAIL;
    }
}
