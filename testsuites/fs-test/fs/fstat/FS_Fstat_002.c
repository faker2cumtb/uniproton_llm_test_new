
/* 

【测试目的】:
当输入参数为NULL时，测试fstat接口用于获取文件相关信息，并设置到参数 st 中是否成功。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
调用接口creat创建文件；存放文件信息的 struct stat 类型指针传值为NULL,调用接口fstat获取文件信息；调用接口close关闭文件；调用接口remove删除文件。判断返回值。

【预期结果】:
返回值为-1。

*/

/**************************** 引用部分 ***************************************/
#include <string.h>
#include <errno.h>
#include "test.h"
#include <fcntl.h>

/**************************** 定义部分 ***************************************/
/**************************** 实现部分 ***************************************/
int OS_FS_Fstat_002(int argv[])
{
    int iRet1, iRet = 0;
	char path[50] = FS_ROOT;
	
	strcat(path, "/text.txt");

    /*创建文件*/
    // iRet1 = creat(path, 0777);
    // if (iRet1 == -1)
    // {
    //     TEST_ERRPRINT("creat");
    //     return PTS_FAIL;
    // }
    iRet1 = open(path, O_CREAT, 0777);
    if (iRet1 == -1)
    {
        TEST_ERRPRINT("creat");
        return PTS_FAIL;
    }

    /*fstat 接口用于获取文件相关信息，并设置到参数st 中。*/
    iRet = fstat(iRet1, NULL);
    if (iRet != -1)
    {
        TEST_ERRPRINT("fstat");
        return PTS_FAIL;
    }
    close(iRet1);
    remove(path);
    TEST_OKPRINT();
    return PTS_PASS;
}
