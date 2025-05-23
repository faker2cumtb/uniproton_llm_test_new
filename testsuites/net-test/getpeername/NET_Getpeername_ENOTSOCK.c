#include <pthread.h>
#include <stdio.h>
#include <sched.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>
#include <utime.h>
#include <errno.h>
#include <fcntl.h>

int NET_Getpeername_ENOTSOCK()
{
	unsigned short port = 0x1125;
    struct sockaddr  addr;
    socklen_t addr_len = sizeof(addr);
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
    ret = getpeername(fd, (struct sockaddr *)&addr, &addr_len);
    if( (ret == -1) && (errno == ENOTSOCK))
    {
    	TEST_OKPRINT();
        close(fd);
        remove(path);
        return PTS_PASS;
    } else {
        printf("getpeername failed, error=%d\n", errno);
        TSTDEF_FAILPRINT(errno)	;
        close(fd);
        remove(path);
        return PTS_FAIL;
    }


}


