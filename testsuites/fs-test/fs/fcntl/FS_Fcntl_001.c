
/* 

【测试目的】:
测试fcntl接口对文件描述符 fd 执行 cmd 命令是否成功。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
调用接口remove删除ST_INTE_FS_FCNTL_001.txt文件。调用接口creat创建ST_INTE_FS_FCNTL_001.txt文件。再调用接口open打开ST_INTE_FS_FCNTL_001.txt文件。调用接口fcntl对文件描述符执行 F_DUPFD命令。调用接口fcntl对文件描述符执行 F_GETFD命令。调用接口fcntl对文件描述符执行 F_SETFD命令。调用接口fcntl对文件描述符执行 F_GETFL命令。调用接口fcntl对文件描述符执行 F_SETFL命令。再调用接口close关闭ST_INTE_FS_FCNTL_001.txt文件。再调用接口remove删除ST_INTE_FS_FCNTL_001.txt文件。判断返回值。

【预期结果】:
返回值为0。

*/


#include <string.h>
#include <errno.h>
#include "test.h"
#include <fcntl.h>

int OS_FS_Fcntl_001()
{

    int iRet = 0, ret1;
    int flag = 0;
    int new_fd;
    char filename[50] = FS_ROOT;

	strcat(filename, "/ST_INTE_FS_FCNTL_001.txt");
	
    remove(filename);
    ret1 = open(filename, O_RDONLY, 0777);
    if (ret1 == -1)
    {
        TEST_ERRPRINT("creat");
        flag = 1;
    }

    new_fd = fcntl(ret1, F_DUPFD, 0);
    if (iRet == -1)
    {
        TEST_ERRPRINT("fcntl");
        flag = 1;
    }
    iRet = fcntl(ret1, F_GETFD, &flag);
    if (iRet == -1)
    {
        TEST_ERRPRINT("fcntl");
        flag = 1;
    }
    iRet = fcntl(ret1, F_SETFD, 1);
    if (iRet == -1)
    {
    	TSTDEF_FAILPRINT(errno);
        flag = 1;
    }

	iRet = fcntl(new_fd, F_GETFL, &flag);
    if (iRet == -1)
    {
    	TSTDEF_FAILPRINT(errno);
        flag = 1;
    }
    
    iRet = fcntl(ret1, F_GETFL, &flag);
    if (iRet == -1)
    {
    	TSTDEF_FAILPRINT(errno);
        flag = 1;
    }
    iRet = fcntl(ret1, F_SETFL, 1);
    if (iRet == -1)
    {
    	TSTDEF_FAILPRINT(errno);
        flag = 1;
    }
    close(ret1);
    close(new_fd);
    remove(filename);
    if (flag == 1)
    {
        TEST_FAILPRINT("fcntl");
        return PTS_FAIL;
    }
    else
    {
        TEST_OKPRINT();
        return PTS_PASS;
    }
}
