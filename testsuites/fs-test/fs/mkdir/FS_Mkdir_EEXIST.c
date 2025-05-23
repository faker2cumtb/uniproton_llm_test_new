
/* 
【测试目的】:
已经存在,创建同名目录，测试mkdir是否返回错误码EEXIST。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
调用mkdir命令创建目录"/mkdirdir"；再次调用mkdir命令创建目录"/mkdirdir"；判断返回值。

【预期结果】:
失败返回－1，错误码为EEXIST。

【评价准则】:
与预期的测试结果一致

【假设和约束】:
无

【设计者】:
XXX

【设计时间】:
2020-01-18

</EXECL> */

/**************************** 引用部分 *****************************************/
#include <string.h>
#include <errno.h>
#include "test.h"
#include <fcntl.h>
/**************************** 定义部分 *****************************************/
static int failed = 0;
/****************************** 实现部分 *********************************/
/**
 * @brief
 *  初始化任务。
 *
 * @param[in] 无。
 *
 * @return	 PTS_PASS测试成功，PTS_FAIL测试失败。
 */

int OS_FS_Mkdir_EEXIST()
{
    int	 iRet;
    int ret=0;
    char dirname[100] = FS_ROOT;

    strcat(dirname, "/mkdirdir_001");
    ret = rmdir(dirname);
    if(ret !=0 && errno != ENOENT)
    {
    	printf("ret =%d",ret);
    	TSTDEF_ERRPRINT(errno);
    	 return ;
    }
   ret= remove(dirname);
    if(ret !=0 && errno != ENOENT)
    {
    	printf("ret =%d",ret);
    	TSTDEF_ERRPRINT(errno);
    	 return ;
    }
    /*prepare*/
    iRet = mkdir(dirname, 0777);
    if (-1 == iRet)
    {
        TSTDEF_ERRPRINT(errno);
        failed = 1;
        return ;
    }
    /*test*/
    iRet = mkdir(dirname, 0777);
    if (-1 != iRet || errno !=  EEXIST)
    {
        TSTDEF_ERRPRINT(errno);
        failed = 1;
    }
    else
    {
        TEST_OKPRINT();
    }
    /*clear environment*/
    rmdir(dirname);
	remove(dirname);
    if (!failed)
    {
        return PTS_PASS;
    }
    else
    {
        return PTS_FAIL;
    }
}
