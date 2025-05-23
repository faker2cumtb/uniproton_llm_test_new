
/* 
【测试目的】:
当输入参数有效时，测试fsync是否成功。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
调用接口creat创建文件；文件描述符为无效文件描述符,调用接口fsync将与文件描述符关联的文件存储到存储设备上；调用接口close关闭文件；调用接口remove删除文件；判断返回值。

【预期结果】:
返回值为0。

*/


/**************************** 引用部分 ***************************************/
#include <string.h>
#include <errno.h>
#include "test.h"
#include <fcntl.h>

/**************************** 定义部分 ***************************************/
/**************************** 实现部分 ***************************************/
int OS_FS_Fsync_001()
{
    int iRet1, iRet = 0;
	char path[50] = FS_ROOT;
	
	strcat(path, "/text.txt");

    /*创建文件*/
	iRet1 = open(path,   O_RDWR|O_CREAT, 0777);
    // iRet1 = creat(path, 0777);
    if (iRet1 == -1)
    {
        TEST_ERRPRINT("creat");
        return PTS_FAIL;
    }

    /* 向文件写入数据 */
    const char *data = "test data";
    if (write(iRet1, data, strlen(data)) == -1) {
        TEST_ERRPRINT("write");
        close(iRet1);
        remove(path);
        return PTS_FAIL;
    }

    /*fsync 接口将与fildes 关联的文件存储到存储设备上*/
    iRet = fsync(iRet1);
    if (iRet != 0)
    {
        TEST_ERRPRINT("fsync");
        TSTDEF_ERRPRINT(errno);
        close(iRet1);
        remove(path);
        return PTS_FAIL;
    }
    close(iRet1);
    remove(path);
    TEST_OKPRINT();
    return PTS_PASS;
}
