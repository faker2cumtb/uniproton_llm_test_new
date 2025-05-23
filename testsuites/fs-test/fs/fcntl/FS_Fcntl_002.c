
/*
【测试目的】:
当参数fd为无效文件描述符、参数cmd为不支持的命令，调用fcntl接口对文件描述符 fd 执行 cmd 命令是否成功。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
调用接口remove删除ST_FUNC_FS_FCNTL_001.txt文件；调用接口fcntl对无效文件描述符执行 F_DUPFD命令；调用接口fcntl对文件描述符执行 不支持的命令；判断返回值。

【预期结果】:
返回值为-1。

*/

/**************************** 引用部分 ***************************************/
#include <string.h>
#include <errno.h>
#include "test.h"
#include <fcntl.h>
/**************************** 定义部分 ***************************************/
/**************************** 实现部分 ***************************************/
int OS_FS_Fcntl_002()
{
    int iRet = 0;
    char filename[50] = FS_ROOT;
	int fd;
	
	strcat(filename, "/text.txt");
		
    /*创建文件*/
	fd = open(filename,   O_RDWR|O_CREAT, 0777);
    // fd = open(filename, O_CREAT|O_RDWR, 0777);;
    if (iRet == -1)
    {
        TEST_ERRPRINT("creat");
        return PTS_FAIL;
    }

    /*fcntl 接口对文件描述符fd 执行cmd 命令。文件描述符无效*/
    iRet = fcntl(1000, F_DUPFD);
    if ((iRet != -1)&&(errno != EBADF))
    {
        TEST_ERRPRINT("fcntl");
        return PTS_FAIL;
    }
    /*fcntl cmd命令错误*/
    iRet = fcntl(iRet, 5);
    if( (iRet != -1)&&(errno != EINVAL))
    {
        TEST_ERRPRINT("fcntl");
        return PTS_FAIL;
    }

	close(fd);
	remove(filename);
    TEST_OKPRINT();
    return PTS_PASS;
}
