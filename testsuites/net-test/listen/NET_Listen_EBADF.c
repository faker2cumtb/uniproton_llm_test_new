#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>
int NET_Listen_EBADF()
{
    int nSockFd = 0;
    int flag = 0;
    int ret = 0;
    int fd = 0;
    
    fd = socket(AF_INET, SOCK_STREAM, 0);
  	if (fd < 0)
  	{
  		return -1;
  	}
  	
  	close(fd);
 
    ret = listen(fd, 5);
    if (ret != -1 || errno != EBADF)
    {
        TEST_FAILPRINT("listen");
        flag = 1;
        return PTS_FAIL;
    }
   
    /*判断测试结果*/
    if (flag == 1)
    {
        TEST_FAILPRINT("listen");
        return PTS_FAIL;
    }
    else
    {
        TEST_OKPRINT();
        return PTS_PASS;
    }
}

