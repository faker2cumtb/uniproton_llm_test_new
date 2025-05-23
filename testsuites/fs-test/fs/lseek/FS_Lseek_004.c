
/* 

【测试目的】:
设置文件偏移位置为0时，测试lseek设置文件偏移位置是否成功。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
设置文件偏移位置为0。

【预期结果】:
不发生偏移，读出的位置为当前位置。

【评价准则】:
与预期的测试结果一致

 */


/**************************** 引用部分 *****************************************/
#include <string.h>
#include <errno.h>
#include "test.h"
#include <fcntl.h>
/*************************** 前向声明部分 ****************************************/
#define SEEK_SET  (0x0000)
/**************************** 定义部分 *****************************************/
/****************************** 实现部分 *********************************/
int	OS_FS_Lseek_004( )
{
    unsigned int fd ;
    unsigned int ret;
    char ch[11];
    int failed = 0;
    /*请求驱动控制设备处理例程*/
    char path[256] = FS_ROOT;
    strcat(path, "/test.txt");
    fd = open(path,  O_RDWR|O_CREAT, 0777);
//    fd = creat(path, 0777);
    if (ret < 0)
    {
        failed = 1;
        TEST_ERRPRINT("creat error");
    }
    ret = write(fd, "abcde", 5);
    if (ret != 5 )
    {
        failed = 1;
        TEST_ERRPRINT("write");
    }
    close(fd);
    fd = open(path, O_RDONLY, 0777);
    if (fd < 0 )
    {
        failed = 1;
        TEST_ERRPRINT("open");
    }

    ret = lseek(fd, 0, SEEK_SET);
    if (ret != 0)
    {
        failed = 1;
        TEST_ERRPRINT("lseek");
    }

    ret = read(fd, ch, 3);
    if (ret != 3 )
    {
        failed = 1;
        TEST_ERRPRINT("read");
    }

    if (ch[0] != 'a' || ch[1] != 'b' || ch[2] != 'c')
    {
        failed = 1;
        TEST_ERRPRINT("read no 'c'");
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
