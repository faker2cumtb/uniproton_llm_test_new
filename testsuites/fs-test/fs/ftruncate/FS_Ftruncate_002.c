
/* 
【测试目的】:
当截取的长度超出原文件长度时，测试ftruncate截断文件是否成功，验证多出的长度是否添0。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
调用ftruncate接口截断文件，截取的长度超出原文件长度，验证多出的长度是否添0；判断返回值。

【预期结果】:
返回值为0。验证多出的长度添0。

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
int OS_FS_Ftruncate_002()
{
    char str[10] = "111111111";
    char string[100] = "";
    char string1_s[100] = "";
    int iRet;
    int i;
    int ret = 0;
    int ret1 = 0;
    int qq = 0;
    char path[50] = FS_ROOT;

    strcat(path, "/text.txt");

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
    qq = read(iRet, string, 100);
    if (qq != 8)
    {
        TEST_ERRPRINT("write");
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
    ret = ftruncate(iRet, 20);
    if (ret != 0)
    {
        TEST_ERRPRINT("ftruncate");
        return PTS_FAIL;
    }
    //判断读取的长度是否为20
    qq = read(iRet, string1_s, 100);
    if (qq != 20)
    {
        TEST_ERRPRINT("ftruncate");
        return PTS_FAIL;
    }
    //判断文件内容是否正确。
    for (i = 0; i < 20; i++)
    {
        if (i < 8)
        {
            if (string1_s[i] != '1')
            {
                TEST_ERRPRINT("");
                return PTS_FAIL;
            }
        }
        else if (i > 7 && i < 20)
        {
            if (string1_s[i] != '\0')
            {   
            	printf("[%d]",string1_s[i]);
                TEST_ERRPRINT("");
                return PTS_FAIL;
            }
        }
    }
    close(iRet);
    remove(path);
    TEST_OKPRINT();
    return PTS_PASS;
}
