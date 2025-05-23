
/* 

【测试目的】:
同时设置了O_CREAT和O_EXCL选项，且文件存在,测试open打开文件是否成功。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
同时设置了O_CREAT和O_EXCL选项，且文件存在；
查看接口返回值。

【预期结果】:
返回值为-1，错误号为EEXIST。

【评价准则】:
与预期的测试结果一致

*/

/**************************** 引用部分 *****************************************/

#include <stdio.h>
#include <utime.h>
#include <errno.h>
#include <fcntl.h>
#include "test.h"

int OS_FS_Open_EEXIST()
{
    int fp;
    int isOK = 0;
    char path[250] = FS_ROOT;
    strcat(path, "/ST_FUNC_FS_OPEN_016.txt");
    fp = open(path, O_CREAT, 0777);
    if (fp < 0)
    {
        TEST_ERRPRINT("creat");
        isOK = 1;
    }
    close(fp);
    fp = open(path, O_CREAT | O_EXCL, 0777);

    if (fp != -1 || errno != EEXIST)
    {
        TEST_ERRPRINT("fopen");
        isOK = 1;
    }
    close(fp);
    remove(path);
    if (isOK)
    {
        TEST_ERRPRINT("fopen");
        return PTS_FAIL;
    }
    else
    {
        TEST_OKPRINT();
        return PTS_PASS;
    }
}
