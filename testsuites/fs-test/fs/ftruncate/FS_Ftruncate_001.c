
/* 
【测试目的】:
当输入参数有效时，测试ftruncate截断文件是否成功。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
调用truncate截断文件，设置截断长度为2。调用truncate截断文件，设置截断长度为0。判断返回值。

【预期结果】:
返回值为0。

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
int OS_FS_Ftruncate_001()
{
    int ret = 0;
    int ret1 = 0;
    int qq = 0;
    char str[10] = "111111111";
    char string[100] = "";
    int iRet;
    int i;
    char path[50] = FS_ROOT;

    strcat(path, "/text.txt");

    /*创建文件*/
	ret1 = open(path,   O_RDWR|O_CREAT, 0777);
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
    qq = read(iRet, string, 100);
    if (qq != 8)
    {
        TEST_ERRPRINT("read");
        return PTS_FAIL;
    }
    //判断写入是否正确
    for (i = 0; i < 9; i++)
    {
        if (i < 8)
        {
            if (string[i] != '1')
            {
                TEST_ERRPRINT("");
                return PTS_FAIL;
            }
        }
        else if (string[i] == '1')
        {
            TEST_ERRPRINT("");
            return PTS_FAIL;
        }
    }

    close(iRet);
    iRet = open(path, O_CREAT | O_RDWR, 0777);
    if (iRet == -1)
    {
        TEST_ERRPRINT("open");
        return PTS_FAIL;
    }

    //截断文件
    ret = ftruncate(iRet, 2);
    if (ret != 0)
    {
        TEST_ERRPRINT("ftruncate");
        return PTS_FAIL;
    }

    memset(string, 0, 100);
    qq = read(iRet, string, 100);
    if (qq != 2)
    {
        TEST_ERRPRINT("read");
        return PTS_FAIL;
    }

    //判断文件内容是否正确
    for (i = 0; i < 3; i++)
    {
        if (i < 2)
        {
            if (string[i] != '1')
            {
                TEST_ERRPRINT("");
                return PTS_FAIL;
            }
        }
        else if (string[i] == '1')
        {
            TEST_ERRPRINT("");
            return PTS_FAIL;
        }
    }

    //截断文件
    ret = ftruncate(iRet, 0);
    if (ret != 0)
    {
        TEST_ERRPRINT("ftruncate");
        return PTS_FAIL;
    }
#if RTOS7_X
    close(iRet);

    iRet = open(path, O_CREAT | O_RDWR, 0777);
	if (iRet == -1)
	{
		TSTDEF_FAILPRINT(errno);
		return PTS_FAIL;
	}
#endif

    //读取的内容
    qq = read(iRet, string, 100);

#if RTOS7_X
    if (qq >= 0) //读取失败返回-1
#else RTOS2_X
        if (qq != 0) //判断读取的内容是否为空
#endif
    {
        TEST_ERRPRINT("read");
        return PTS_FAIL;
    }
    
    close(iRet);
    remove(path);
    TEST_OKPRINT();
    return PTS_PASS;
}
