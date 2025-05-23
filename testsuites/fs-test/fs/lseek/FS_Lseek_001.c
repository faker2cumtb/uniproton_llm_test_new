
/* 
【测试目的】:
验证设置文件偏移位置功能。

【测试类型】:
功能测试

【先决条件】:
操作系统已运行。

【测试步骤】:
创建一个文件；写入文件；设置文件偏移位置为2，检查是否返回成功；读取文件，检查设置文件偏移位置是否成功。

【预期结果】:
设置文件偏移位置返回成功；读取文件，设置文件偏移位置成功。

【评价准则】:
与预期的测试结果一致

*/


/**************************** 引用部分 *****************************************/
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include "test.h"

/*************************** 前向声明部分 ****************************************/

/**************************** 定义部分 *****************************************/

/****************************** 实现部分 *********************************/

int OS_FS_Lseek_001( )
{
    int fd ;
    int ret;
    char ch[11];
    int failed;
    char path[50] = FS_ROOT;
	failed = 0;
    strcat(path, "/test.txt");
    remove(path);
    fd = open(path,  O_RDWR|O_CREAT, 0777);
//    fd = creat(path, 0777);
    if (fd < 0)
    {
        failed = 1;
        TEST_ERRPRINT("open error");
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

    ret = lseek(fd, 2, SEEK_SET);
    if (ret != 2)
    {
        failed = 1;
        TSTDEF_FAILPRINT(errno);
    }
    ret = read(fd, ch, 3);
    if (ret != 3 )
    {
        failed = 1;
        TEST_ERRPRINT("read");
    }

    if (ch[0] != 'c' || ch[1] != 'd' || ch[2] != 'e')
    {
        failed = 1;
        TEST_ERRPRINT("read no 'c'");
    }

    close(fd);
    remove(path);
    if (!failed)
    {
        TEST_OKPRINT();
        return PTS_PASS;
    }
    else
    {
        TEST_ERRPRINT("");
        return PTS_FAIL;
    }
}
