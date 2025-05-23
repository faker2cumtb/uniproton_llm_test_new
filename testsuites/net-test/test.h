/*
 * return codes
 */
#ifndef _TEST_H
#define _TEST_H
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include <limits.h>
#include <lwip/netif.h>
#ifndef UNUSED_ARG
#define UNUSED_ARG(x)     (void)x
#endif /* UNUSED_ARG */
/*
 * @brief 波特率表
 */
#define BAUD_9600       9600
#define BAUD_19200      19200
#define BAUD_38400      38400
#define BAUD_57600      57600
#define BAUD_115200     115200

#define server_IP "127.0.0.1"
#define client_IP "127.0.0.1"
#define host_IP "127.0.0.1"

//网络用例的IP在这里使用宏进行统一管理，执行测试时只要将以下9个ip修改与目标机同一网段即可
#define TTOS_IP_vm1 "192.168.118.200"
#define TTOS_IP_vm2 "192.168.118.201"
#define TTOS_IP_vm3 "192.168.118.202"
#define TTOS_IP_vm4 "192.168.118.203"
#define TTOS_IP_vm5 "192.168.118.204"
#define TTOS_IP_vm6 "192.168.118.205"
#define TTOS_IP_vm7 "192.168.118.206"
#define TTOS_IP_vm8 "192.168.118.207"
#define COMMONNET_IP "192.168.118.208"


#define SELF_DEFINE_TYPE 0x0700  //类型帧：要大于0x0600

//以下4个ip无需修改
#define TTOS_IP_vm12 "192.168.117.207"
#define TTOS_IP_vm13 "192.168.119.216"
#define TTOS_IP_vm22 "192.168.117.212"
#define TTOS_IP_vm23 "192.168.119.211"

//posix用例自动化
struct test_fn_arr{
    int (*fn)(void);
	int (*beforefn)(void);
	int (*afterfn)(void);
    const char *fn_name;
};

extern int test_restart_flag;
#define PTS_PASS        0
#define PTS_FAIL        1
#define PTS_UNRESOLVED  2
#define PTS_UNSUPPORTED 4
#define PTS_UNTESTED    5

#define PTS_NOTBEGIN 6
#define PTS_FINISH 7

#define PTS_SEND   -1
#define PTS_RECV    -2
#define PTS_JUDGEONHOST  -3
#define TEST_RESTART {test_restart_flag = 1;}    //针对测试工具添加宏定义

//VMK_VINT_EXCEPTION栈保护异常中断号定义
#ifdef _X86_
#define EXCEPTION_NUM 14
#elif defined(_MIPS_)
#ifdef CONFIG_OS_LP64
#define EXCEPTION_NUM 3
#else
#define EXCEPTION_NUM 1
#endif
#elif defined(__AARCH32__)
#define EXCEPTION_NUM 3
#elif defined(__AARCH64__)
#define EXCEPTION_NUM 36
#elif defined(__ARC__)
#define EXCEPTION_NUM 14
#endif


#ifdef _X86_
#define ETH_NAME "en1"
#elif defined (_MIPS_)
#define ETH_NAME "gm1"
#elif defined(__AARCH32__)
#define ETH_NAME  "gm1"
#elif defined(__AARCH64__)
#define ETH_NAME  "gm1"
#endif

/*
 * @brief 各个平台的测试串口
 */
#ifdef _X86_
#define TEST_COM    "/COM2"
#elif defined (_MIPS_)
#define TEST_COM    "/COM4"
#elif defined(__AARCH32__)
#define TEST_COM    "/COM1"
#elif defined(__AARCH64__)
#define TEST_COM    "/COM1"
#endif

#ifdef __cplusplus

#define TEST_OKPRINT()      
#define TEST_ERRFPRINT(err) std::cout << "***Test Failed, error: " << err << ", filename: " << __FILE__ << " line: " << __LINE__ << "***" << std::endl;
#define TEST_FAILRINT()     std::cout << "***Test Failed, filename: " << __FILE__ << " line: " << __LINE__ << "***" << std::endl;

#define VERIFY(condition) do{\
    if (condition) {\
        TEST_OKPRINT();\
        c_Results.isPass=PTS_PASS;\
    } else {\
        TEST_FAILRINT();\
        c_Results.isPass=PTS_FAIL;\
        return;\
    }\
} while (0)

#define TEST_START(casename) do {\
	extern void casename();\
    cout << "****test_begin****" << #casename << endl;\
    casename();\
	if (c_Results.isPass == PTS_PASS) {cout << "***Test Pass, filename: ../../src/" << #casename << ".cpp ***" <<endl;}\
	else {cout << "***Test Failed, filename: ../../src/" << #casename << ".cpp ***" <<endl;}\
    cout << "****test_end****" << #casename << endl;\
} while(0)\
	
#else

#define TEST_OKPRINT()	printf("***Test Pass, filename: %s ***\r\n", __FILE__);
#define TEST_ERRPRINT(err)	printf("***Test Failed, error: %s, filename: %s line: %d***\r\n", err, __FILE__, __LINE__);
#define TEST_FAILPRINT(err)	printf("***Test Failed, error: %s, filename: %s line: %d***\r\n",err, __FILE__, __LINE__);
#define TEST_FAILRINT()  	printf("***Test Failed, filename: %s line: %d***\r\n",  __FILE__, __LINE__);

#define TSTDEF_OKPRINT()	printf("***Test Pass, filename: %s ***\r\n", __FILE__);
#define TSTDEF_ERRPRINT(errno)	printf("***Test Failed, error: %d, filename: %s line: %d***\r\n", errno, __FILE__, __LINE__);
#define TSTDEF_FAILPRINT(errno)	printf("***Test Failed, error: %d, filename: %s line: %d***\r\n",errno, __FILE__, __LINE__);

#define TEST_RESULT 	if(PTS_PASS != status){printf("Result: error,function:%s,line:%d,status:%d\n",__FUNCTION__,__LINE__,status);return ;}

#endif


#ifndef SRC_INCLUDE_CONFIG_H_
#define SRC_INCLUDE_CONFIG_H_

#define GJB_PASS        (0)
#define GJB_FAIL        (-1)

#define PASS_F            "***Test Pass, filename: " __FILE__
#define FAILED_F           "***Test Failed, filename:  "  __FILE__

#define GJB_PRT_INFO(fmt, arg...)       gjb_os_fprintf(fmt, ##arg)
#define GJB_PRT_ERROR_INFO(fmt, arg...) gjb_os_fprintf("<ERROR %d> <FUNC %s> -- <LINE %d> "fmt GJB_CRLF, errno, __func__, __LINE__, ##arg)

#define GJB_PRI_PASS()      gjb_os_result(PASS_F, 0)
#define GJB_PRI_FAIL()      gjb_os_result(FAILED_F, -1)

#endif /* SRC_INCLUDE_CONFIG_H_ */

#define GoOnChoice(str,a)

#ifndef TEST_DYLOAD_H
#define TEST_DYLOAD_H

#define  TESTCASE_Normal  1
#define  TESTCASE_Errno 1
#define  TESTCASE_SDC_SUPPORT 0
#define  RTOS7_X 0
#define  RTOS2_X 1

#define Skip_TestCase()   printf("****test_begin****\n***Test Pass SKIP_TESTCASE ***\n****test_end****\n");

typedef struct
{
    int isPass;              //测试结果TEST_PASS~TEST_UNRESOLVED
    char retValue[100];            //测试中接口真实返回值
}T_TestResults;

extern T_TestResults c_Results;

//nfs和fatfs文件系统

#ifdef __cplusplus        //c++根目录需要加一个“/”
#define PATH_F "/"
#else
#define PATH_F
#endif

#if CONFIG_NFS
#define FS_ROOT  "/nfsd"PATH_F
#else
#define FS_ROOT  "/tmp"PATH_F
#endif



#define EPSION 1.0e-6


/*应用分区空间*/
#define P1_RAM_START				0xA000000
#define P2_RAM_START				0xB000000

#define P1_RAM_LOGIC_START			0x40000000
#define P2_RAM_LOGIC_START			0x40000000


#ifdef __AARCH32__
#define P1_SHARE_MEM_LOGIC_START	0xde000000
#define P2_SHARE_MEM_LOGIC_START	0xde000000
#elif defined(__AARCH64__)
#define P1_SHARE_MEM_LOGIC_START	0xFFFFFFF4e0000000
#define P2_SHARE_MEM_LOGIC_START	0xFFFFFFF4e0000000
#else
#define P1_SHARE_MEM_LOGIC_START	0x50200000
#define P2_SHARE_MEM_LOGIC_START	0x50200000
#endif
#define P1_RAM_RDONLY_START				0x4d000000   //只读空间地址
#define P2_RAM_RDONLY_START				0x4d000000

#define ELF_PATH            "/ata00:2/test_file.out"

/*可加载模块名*/
#define ELF_NAME 	"test_file.out"

/*可加载模块路径*/
#define FILE_PATH 	"/ata00:2"

/*模块中common符号名*/
#define COMSYM 	"iTestCommmon"

/*模块中全局变量名*/
#define GLOSYM 	"iTestGlobal"

/*模块中全局函数名*/
#define GLOFUN 	"func_hello1"

/*模块中局部函数名*/
#define LOCFUN 	"localFunc"

#define ROOTDIR "/ata00:2"

#endif
static  ip_addr_t brief_of_netif_getip(char * netif_name)
{
	struct netif * netif_temp =  NULL;
	ip_addr_t ipaddr = {};
	if (NULL != netif_name)
	{
		netif_temp = netif_find(netif_name);
		if (NULL != netif_temp)
		{
			ipaddr = netif_temp->ip_addr;
		}
	}
	return ipaddr;
}
#endif
