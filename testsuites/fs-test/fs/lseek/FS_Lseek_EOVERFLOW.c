
/* 
【测试目的】:
参数offset超过文件偏移量,lseek返回错误码EOVERFLOW。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
创建文件，往文件里写数据并关闭该文件，在打开文件，lseek偏移位置，判断其返回错误码。

【预期结果】:
lseek返回错误码EOVERFLOW。

【评价准则】:
与预期的测试结果一致

 */

/**************************** 引用部分 *****************************************/
#include <string.h>
#include <errno.h>
#include "test.h"
#include <fcntl.h>
#include <limits.h>
/*************************** 前向声明部分 ****************************************/
/**************************** 定义部分 *****************************************/
/****************************** 实现部分 *********************************/
int	OS_FS_Lseek_EOVERFLOW( )
{
	    unsigned int fd ;
	    unsigned int ret;
	    char ch[11];
	    int failed = 0;
	    /*请求驱动控制设备处理例程*/
	    char path[256] = FS_ROOT;
	    strcat(path, "/ST_INTE_FS_LSEEK_006.txt");

	    fd = open(path, O_RDWR|O_CREAT, 0777);
//	    fd = creat(path, 0777);
	    if (ret < 0)
	    {
	        failed = 1;
	        TSTDEF_FAILPRINT(errno);
	    }
	    ret = write(fd, "abcde", 5);
	    if (ret != 5 )
	    {
	        failed = 1;
	        TSTDEF_FAILPRINT(errno);
	    }
	    close(fd);
	    fd = open(path, O_RDONLY, 0777);
	    if (fd < 0 )
	    {
	        failed = 1;
	        TSTDEF_FAILPRINT(errno);
	    }
	    ret=read(fd,ch,1);
	    if(ret==-1)
	    {
	        failed = 1;
	        TSTDEF_FAILPRINT(errno);

	    }

	    ret = lseek(fd,LONG_MAX  , SEEK_CUR);
	    if (ret != -1)
	    {
	        failed = 1;
	        TSTDEF_FAILPRINT(errno);
	    }

	    if(errno !=EOVERFLOW)
	    {
	    	  failed = 1;
	          TSTDEF_FAILPRINT(errno);
	          printf("%d \n",errno);
	    }

	    ret = lseek(fd,LONG_MAX  , SEEK_END);
	    if (ret != -1)
	    {
	        failed = 1;
	        TSTDEF_FAILPRINT(errno);
	    }

	    if(errno !=EOVERFLOW)
	    {
	    	  failed = 1;
	          TSTDEF_FAILPRINT(errno);
	          printf("%d \n",errno);
	    }

	    close(fd);
	    remove(path);
      if (failed)
     {

         TEST_ERRPRINT("test fail");
         return	PTS_FAIL;
     }
     else
     {
         TEST_OKPRINT();
         return	PTS_PASS;
     }


}
