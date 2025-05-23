
/* 

【测试目的】:
长度小于0，ftruncate返回错误码EINVAL

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
调用ftruncate截断文件，设置截断长度为-2，判断返回值。

【预期结果】:
返回错误码EINVAL。

【评价准则】:
与预期的测试结果一致
*/

/**************************** 引用部分 *****************************************/
#include <string.h>
#include <errno.h>
#include "test.h"
#include <fcntl.h>
/*************************** 前向声明部分 ****************************************/
/**************************** 定义部分 *****************************************/
/****************************** 实现部分 *********************************/

int OS_FS_Ftruncate_EINVAL()
{
    int ret = 0;
    int ret1 = 0;
    char str[10] = "111111111";
    int iRet;
    char path[50] = FS_ROOT;

    strcat(path, "/ST_INTE_FS_FTRUNCATE_004.txt");

    /*创建文件*/
	ret1 = open(path,   O_RDWR|O_CREAT, 0777);
    // ret1 = creat(path, 0777);
    if (ret1 < 0)
    {
        TEST_ERRPRINT("creat");
        return PTS_FAIL;
    }
    /*向文件写入内容*/
    ret = write(ret1, str, 8);
    if (ret != 8)
    {
        TEST_ERRPRINT("write");
        return PTS_FAIL;
    }
    close(ret1);
    iRet = open(path, O_CREAT | O_RDWR, 0777);
    if (iRet == -1)
    {
        TEST_ERRPRINT("creat");
        return PTS_FAIL;
    }


    //截断文件
    ret = ftruncate(iRet, -2);
    if (ret != -1)
    {
        TEST_ERRPRINT("ftruncate");
        return PTS_FAIL;
    }
     if(errno ==EINVAL)
     {
    	  TEST_OKPRINT();
     }

    close(iRet);
    remove(path);

    return PTS_PASS;
}
