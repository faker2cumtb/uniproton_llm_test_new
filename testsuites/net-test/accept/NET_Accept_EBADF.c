#define __SYLIXOS_KERNEL

#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>

#define get_local_ip() inet_addr(server_IP)
static random_port = 0;

int NET_Accept_EBADF(void)
{
    int clientAddrLen;
    int subSock;
    int sock;
    struct sockaddr_in clientAddr;

    


    clientAddr.sin_family = PF_INET;//网络传输
    clientAddr.sin_addr.s_addr = inet_addr(server_IP);
    clientAddr.sin_port = htons(52111);//本机的转为网络的端口号
    

    sock = socket(AF_INET, SOCK_DGRAM, 0);;
    if (-1 == sock)
    {
        return PTS_FAIL ;
    }
    
    close(sock);
    
    subSock = accept(sock, (struct sockaddr *)&clientAddr, (socklen_t *)&clientAddrLen);

    if ((-1 == subSock) && (errno == EBADF))
    {
        TEST_OKPRINT();
        close(subSock);
        return PTS_PASS;
    }

    close(subSock);
    return PTS_FAIL;
}
