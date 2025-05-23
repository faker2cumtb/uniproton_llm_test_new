
/* 

【测试目的】:
当截断长度小于0，测试truncate返回错误码EINVAL。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
当截断长度小于0，调用truncate截断文件；判断返回值。

【预期结果】:
返回错误码EINVAL。

【评价准则】:
与预期的测试结果一致

*/

/**************************** 引用部分 *****************************************/
#include <stdio.h>
#include <utime.h>
#include <errno.h>
#include <fcntl.h>
#include "test.h"

int OS_FS_Truncate_EINVAL()
{
    int ret = 0,i=0,j=0;
    int fd;
    int flag = 0;
    char str[10] = "1111111111";
    char rmdirfile[50] = FS_ROOT;
    FILE *fp={0};
    char rbuf[10]="\0";

    strcat(rmdirfile, "/ST_INTE_FS_TRUNCATE_006.txt");

    remove(rmdirfile);

    fd = open(rmdirfile, O_RDWR|O_CREAT, 0777);
//    fd = open(rmdirfile, O_CREAT|O_RDWR, 0777);
    if (fd < 0)
    {
        TEST_FAILPRINT("creat");
        flag = 1;
    }

    ret = write(fd, str, 8);
    if (ret != 8)
    {
        TEST_FAILPRINT("write");
        flag = 1;
    }
	
	close(fd);
    ret = truncate(rmdirfile, -5);

    if (ret != -1)
    {
        TEST_FAILPRINT("truncate");
        flag = 1;
    }

    if(errno !=EINVAL)
    {
    	flag = 1;
    }

    remove(rmdirfile);

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
