
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <limits.h>
#ifndef UNUSED_ARG
#define UNUSED_ARG(x)     (void)x
#endif /* UNUSED_ARG */

extern int test_restart_flag;
#define PTS_PASS        0
#define PTS_FAIL        1
#define PTS_UNRESOLVED  2
#define PTS_UNSUPPORTED 4
#define PTS_UNTESTED    5

#define AFILE_FAILED -1
#define AFILE_OK 0

#include "prt_buildef.h"

#define PTS_NOTBEGIN 6
#define PTS_FINISH 7

#define PTS_SEND   -1
#define PTS_RECV    -2
#define PTS_JUDGEONHOST  -3
#define TEST_RESTART {test_restart_flag = 1;} 

#ifdef __cplusplus

#define TEST_OKPRINT()      
#define TEST_ERRFPRINT(err) std::cout << "***Test Failed, error: " << err << ", filename: " << __FILE__ << " line: " << __LINE__ << "***" << std::endl;
#define TEST_FAILRINT()     std::cout << "***Test Failed, filename: " << __FILE__ << " line: " << __LINE__ << "***" << std::endl;
	
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

#endif /* SRC_INCLUDE_CONFIG_H_ */

#define GoOnChoice(str,a)

#define  TESTCASE_Normal  1
#define  TESTCASE_Errno 1
#define  TESTCASE_SDC_SUPPORT 0
#define  RTOS7_X 0
#define  RTOS2_X 1

#define Skip_TestCase()   printf("****test_begin****\n***Test Pass SKIP_TESTCASE ***\n****test_end****\n");



//nfs和fatfs文件系统

#ifdef __cplusplus        //c++根目录需要加一个“/”
#define PATH_F "/"
#else
#define PATH_F
#endif


#define FS_ROOT  "/root"PATH_F
#define FS_TMP  "/tmp"PATH_F

typedef struct
{
    int isPass;              //测试结果TEST_PASS~TEST_UNRESOLVED
    char retValue[100];            //测试中接口真实返回值
}T_TestResults;

extern T_TestResults c_Results;
#define T_VOID      void             /* void类型 */
#define FIOFSTATVFSGETBYNAME 60              /* 通过文件名获取文件系统信息*/

