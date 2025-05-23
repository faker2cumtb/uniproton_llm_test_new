
/* 
【测试目的】:
打开的文件为特殊字符或块文件，但与此特殊文件关联的设备不存在,测试open打开文件是否成功。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
打开的文件为特殊字符或块文件，但与此特殊文件关联的设备不存在；查看接口返回值。

【预期结果】:
返回值为-1。错误号为ENXIO。

【评价准则】:
与预期的测试结果一致
 */


/**************************** 引用部分 *****************************************/
#include <stdio.h>
#include <utime.h>
#include <errno.h>
#include <fcntl.h>
#include "test.h"


/**************************** 前向声明部分 *****************************************/

/**************************** 定义部分 *****************************************/


/**************************** 实现部分 *****************************************/
static int failed = 0;

int OS_FS_Open_ENXIO()
{
    int fd;
    char path[270] = "/dev/dummy";

    fd = open(path, O_RDONLY | O_NOCTTY);
    printf("fd=%d,errno=%d\n", fd, errno);
    if (fd != -1 || errno != ENXIO)
    {
        failed = 1;
        TEST_FAILPRINT("open did not success");
    }

    close(fd);
    if (failed)
    {
        TEST_FAILPRINT("test did not success");
        return PTS_UNRESOLVED;
    }

    TEST_OKPRINT();
    return PTS_PASS;
}

