#include <pthread.h>
#include "../test.h"
#include <sys/socket.h>
#include <sys/select.h>
#include <stdio.h>
#include <netinet/in.h>

#define BUFF_SIZE 10
static uint8_t  RecvBuff[BUFF_SIZE];
static uint8_t  sendBuff[BUFF_SIZE]="sssssssssssssssss";
static random_port = 0;

int NET_Bind_EADDRINUSE(int argv[])
{
	int iRet1,iRet2,iRet3,iRet4;
	int i=0,k=0;
	int nSockFd,client_fd,nSockFd1;
	struct sockaddr_in local_addr,remote_addr;
	int len,nDataLen;	
	fd_set read_set;
	fd_set write_set;
	fd_set except_set;
	struct timeval time_;

	

	
	time_.tv_sec=1;
	time_.tv_usec=100;
	nSockFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	nSockFd1 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (nSockFd < 0 ||nSockFd1<0)
	{
		printf("###sock err 1\n");
		return -4;
	}
	// local_addr.sin_len = sizeof(local_addr);//地址长度
	local_addr.sin_family = AF_INET;//网络传输
	local_addr.sin_addr.s_addr = inet_addr(server_IP);//服务端ip
	local_addr.sin_port = htons(53333);//本机的转为网络的端口号

	iRet1 = bind(nSockFd1, (struct sockaddr *) &local_addr, sizeof(local_addr));
	iRet2 = bind(nSockFd, (struct sockaddr *) &local_addr, sizeof(local_addr));
	if((iRet1 == 0 )&&( iRet2== -1)&&(errno ==EADDRINUSE))
	{
       TEST_OKPRINT();

	}
	else{
		printf("iRet1=%d iRet2=%d errno =%d\n",iRet1,iRet2,errno);
		TEST_FAILRINT();
	}

	close(nSockFd);
	close(nSockFd1);

}
