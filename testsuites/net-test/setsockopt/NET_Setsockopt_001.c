#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include "../test.h"
#include <netinet/in.h>

static int isOk = 0;
static random_port = 0;

int NET_Setsockopt_001(int argv[])
{
	int iRet1,iRet2,iRet3;
	int i=0,k=0;
	int nSockFd_tcp,client_fd;
	int nSockFd_udp;
	struct sockaddr_in local_addr,remote_addr;
	int len,nDataLen;
	int opt=1;
	int get_opt;
	int failed=0;
	struct in_addr addr;
	int get_opt_len;
	int addr_len;
	int buf_len=10;
	addr_len =sizeof(struct in_addr);
	get_opt_len=sizeof(int);
	inet_aton(server_IP,&addr);


	nSockFd_tcp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (nSockFd_tcp < 0)
	{
		printf("###sock err 1\n");
		return -4;
	}
	nSockFd_udp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (nSockFd_udp < 0)
	{
		printf("###nSockFd_udp err 1\n");
		return -4;
	}

	/*SO_BROADCAST*/
	iRet1 = setsockopt(nSockFd_tcp,SOL_SOCKET,SO_BROADCAST,&opt,sizeof(int));
	if(iRet1==0)
	{
		iRet2 = getsockopt(nSockFd_tcp,SOL_SOCKET,SO_BROADCAST,&get_opt,&get_opt_len);
		if((iRet2 !=0) || (get_opt==0))
		{
			printf("iRet2=%d get_opt=%d \n",iRet2,get_opt);
			TEST_FAILRINT();
			failed=1;
		}
	}
	else{
		TEST_FAILRINT();
		failed=1;
	}

	/*SO_KEEPALIVE*/
	iRet1 = setsockopt(nSockFd_tcp,SOL_SOCKET,SO_KEEPALIVE,&opt,sizeof(int));
	if(iRet1==0)
	{
		iRet2 = getsockopt(nSockFd_tcp,SOL_SOCKET,SO_KEEPALIVE,&get_opt,&get_opt_len);
		if((iRet2 !=0) || (get_opt==0))
		{
			printf("iRet2=%d get_opt=%d \n",iRet2,get_opt);
			TEST_FAILRINT();
			failed=1;
		}
	}
	else{
		TEST_FAILRINT();
		failed=1;
	}

	/*SO_REUSEADDR*/
	iRet1 = setsockopt(nSockFd_tcp,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(int));
	if(iRet1==0)
	{
		iRet2 = getsockopt(nSockFd_tcp,SOL_SOCKET,SO_REUSEADDR,&get_opt,&get_opt_len);
		if((iRet2 !=0) || (get_opt==0))
		{
			printf("iRet2=%d get_opt=%d \n",iRet2,get_opt);
			TEST_FAILRINT();
			failed=1;
		}
	}
	else{
		TEST_FAILRINT();
		failed=1;
	}

	/*SO_RCVBUF*/
	iRet1 = setsockopt(nSockFd_tcp,SOL_SOCKET,SO_RCVBUF,&buf_len,sizeof(int));
	if(iRet1==0)
	{
		iRet2 = getsockopt(nSockFd_tcp,SOL_SOCKET,SO_RCVBUF,&get_opt,&get_opt_len);
		if((iRet2 !=0) || (get_opt==0))
		{
			printf("iRet2=%d get_opt=%d errno=%d\n",iRet2,get_opt,errno);
			TEST_FAILRINT();
			failed=1;
		}
	}
	else{
		TSTDEF_FAILPRINT(errno);
		failed=1;
	}


	/*IP_TTL*/
	iRet1 = setsockopt(nSockFd_udp,IPPROTO_IP,IP_TTL,&opt,sizeof(int));
	if(iRet1==0)
	{
		iRet2 = getsockopt(nSockFd_udp,IPPROTO_IP,IP_TTL,&get_opt,&get_opt_len);
		if((iRet2 !=0) || (get_opt==0))
		{
			printf("iRet2=%d get_opt=%d \n",iRet2,get_opt);
			TEST_FAILRINT();
			failed=1;
		}
	}
	else
	{
		TSTDEF_FAILPRINT(errno);
		failed=1;
	}

	/*IP_TOS*/
	iRet1 = setsockopt(nSockFd_udp,IPPROTO_IP,IP_TOS,&opt,sizeof(int));
	if(iRet1==0)
	{
		iRet2 = getsockopt(nSockFd_udp,IPPROTO_IP,IP_TOS,&get_opt,&get_opt_len);
		if((iRet2 !=0) || (get_opt==0))
		{
			printf("iRet2=%d get_opt=%d \n",iRet2,get_opt);
			TEST_FAILRINT();
			failed=1;
		}
	}
	else{
		TSTDEF_FAILPRINT(errno);
		failed=1;
	}

	/*IP_MULTICAST_TTL*/
	iRet1 = setsockopt(nSockFd_udp,IPPROTO_IP,IP_MULTICAST_TTL,&opt,sizeof(int));
	if(iRet1==0)
	{
		iRet2 = getsockopt(nSockFd_udp,IPPROTO_IP,IP_MULTICAST_TTL,&get_opt,&get_opt_len);
		if((iRet2 !=0) || (get_opt==0))
		{
			printf("iRet2=%d get_opt=%d \n",iRet2,get_opt);
			TEST_FAILRINT();
			failed=1;
		}
	}
	else{
		TSTDEF_FAILPRINT(errno);
		failed=1;
	}

	/*IP_MULTICAST_IF*/
	iRet1 = setsockopt(nSockFd_udp,IPPROTO_IP,IP_MULTICAST_IF,&addr,sizeof(struct in_addr));
	if(iRet1==0)
	{
		iRet2 = getsockopt(nSockFd_udp,IPPROTO_IP,IP_MULTICAST_IF,&get_opt,&get_opt_len);
		if((iRet2 !=0) || (get_opt==0))
		{
			printf("iRet2=%d get_opt=%d addr.s_addr=%d\n",iRet2,get_opt,addr.s_addr);
			TEST_FAILRINT();
			failed=1;
		}
	}
	else{
		TSTDEF_FAILPRINT(errno);
		failed=1;
	}

	/*IP_MULTICAST_LOOP*/
	iRet1 = setsockopt(nSockFd_udp,IPPROTO_IP,IP_MULTICAST_LOOP,&opt,sizeof(int));
	if(iRet1==0)
	{
		iRet2 = getsockopt(nSockFd_udp,IPPROTO_IP,IP_MULTICAST_LOOP,&get_opt,&get_opt_len);
		if((iRet2 !=0) || (get_opt==0))
		{
			printf("iRet2=%d get_opt=%d \n",iRet2,get_opt);
			TEST_FAILRINT();
			failed=1;
		}
	}
	else{
		TSTDEF_FAILPRINT(errno);
		failed=1;
	}

	


	/*SO_ACCEPTCONN*/
	local_addr.sin_family = AF_INET;//网络传输
	local_addr.sin_addr.s_addr = inet_addr(server_IP);//服务端ip
	local_addr.sin_port = htons(59911);//本机的转为网络的端口号

	if (bind(nSockFd_tcp, (struct sockaddr *) &local_addr, sizeof(local_addr)))
	{
		printf("###sock err 2\n");
		return -5;
	}

	if (listen(nSockFd_tcp, 5))//5代表最大连接数，代表服务端可以工作了，做好只要有客服端请求，就可以派发相应操作的 准备
	{
		printf("###sock err 3\n");
		return -6;
	}

	iRet2 = getsockopt(nSockFd_tcp,SOL_SOCKET,SO_ACCEPTCONN,&get_opt,&get_opt_len);
	if((get_opt!=1))//errno=0;
	{
		printf("iRet2=%d get_opt=%d \n",iRet2,get_opt);
		TEST_FAILRINT();
		failed=1;
	}
	
	close(nSockFd_tcp);
	close(nSockFd_udp);
	
	if(failed==0)
	{
		TEST_OKPRINT();
		return 0;
	}
	else
	{
		return 1;
	}
}
