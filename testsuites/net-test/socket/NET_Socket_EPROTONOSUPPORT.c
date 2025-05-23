#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>

#define SOCK_STREAM1 11
static int isOk = 0;

int NET_Socket_EPROTONOSUPPORT(int argv[])
{
    int fd = -1;
    int ret = 0;
    int fd1 = -1;
	int ret1 = 0;
    
    /* type错误 */
    fd = socket(AF_INET, SOCK_STREAM1, IPPROTO_TCP);
    if ((-1 == fd) && (errno == EINVAL))
    {
        ret = 1;
    }
     
    /* protocol错误 */
    fd = socket(AF_INET, SOCK_DGRAM, -1);
    if ((-1 == fd) && (errno == EINVAL))
    {
        ret1 = 1;
    }
   
    if (ret & ret1)
    {
        TEST_OKPRINT();
        return PTS_PASS;
    }
    else
    {
        TEST_FAILPRINT("socket");
        return PTS_FAIL;
    }
}
