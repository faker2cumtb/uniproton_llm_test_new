/* 

【测试目的】:
测试truncate截断文件，文件路径中某个目录项不是目录时，测试truncate截断文件是否成功，是否返回的错误码为ENOTDIR。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
调用truncate截断文件,文件路径中某个目录项不是目录；判断返回值。

【预期结果】:
返回值为-1；错误码为ENOTDIR。

【评价准则】:
与预期的测试结果一致

 */

/**************************** 引用部分 *****************************************/
#include <stdio.h>
#include <utime.h>
#include <errno.h>
#include <fcntl.h>
#include "test.h"

int OS_FS_Truncate_ENOTDIR()
{
    int flag = 0;
    int ret = 0;
    int ret1 = 0;
    char str[10] = "123456789";
    char rmdirfile[50] = FS_ROOT;

    strcat(rmdirfile, "/text.txt");
    
    /*创建文件*/
    remove(rmdirfile);
    ret1 = open(rmdirfile, O_RDWR|O_CREAT, 0777);
//    ret1 = open(rmdirfile, O_CREAT|O_RDWR, 0777);
    if (ret1 < 0)
    {
        TEST_FAILPRINT("creat  failed");
        flag = 1;
    }
    /*向文件写入内容*/
    ret = write(ret1, str, 8);
    if (ret != 8)
    {
        TEST_FAILPRINT("write  failed");
        flag = 1;
    }
    
    close(ret1);
    strcat(rmdirfile,"/text.txt");
    /*截取文件，文件路径中某个目录项不是目录。*/
    ret = truncate(rmdirfile, 0);
    if (ret != -1 || errno != ENOTDIR)
    {
        printf("truncate  failed  errno  is  %d \n",errno);
        TEST_FAILPRINT("truncate");
        flag = 1;
    }

    remove(rmdirfile);
    /*判断测试结果*/
    if (flag == 1)
    {
        TEST_FAILPRINT("truncate");
        return PTS_FAIL;
    }
    else
    {
        TEST_OKPRINT();
        return PTS_PASS;
    }

}
