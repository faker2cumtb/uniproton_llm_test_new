
/* 

【测试目的】:
设置文件偏移位置为-1时，测试lseek是否返回错误码EINVAL。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
设置文件偏移位置为-1。

【预期结果】:
返回值为-1，错误码为EINVAL。

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
int	OS_FS_Lseek_EINVAL( )
{
    unsigned int fd ;
    unsigned int ret;

    int failed = 0;
    /*请求驱动控制设备处理例程*/
    char path[256] = FS_ROOT;
    strcat(path, "/test.txt");
    fd = open(path,  O_RDWR|O_CREAT, 0777);
//    fd = creat(path, 0777);
    if (fd < 0)
    {
        failed = 1;
        TEST_ERRPRINT("open error");
    }
    close(fd);
    fd = open(path, O_RDONLY, 0777);
    if (fd < 0 )
    {
        failed = 1;
        TEST_ERRPRINT("read");
    }
    ret = lseek(fd, 0, -1);
    if (ret != -1)
    {
        failed = 1;
        TEST_ERRPRINT("lseek");
    }
    if (errno != EINVAL)
    {
        failed = 1;
        TEST_ERRPRINT("lseek");
    }
    close(fd);

    ret = remove(path);
    if (ret != 0)
    {
        failed = 1;
        TEST_ERRPRINT("remove");
    }
    if (!failed)
    {
        TEST_OKPRINT();
        return	PTS_PASS;
    }
    else
    {
        TEST_ERRPRINT("test is not pass");
        return	PTS_FAIL;
    }

    return	PTS_FAIL;
}
