
/* 
【测试目的】:
测试open打开文件是否成功,当系统中打开的文件超过了最大值，是否返回的错误码为ENFILE。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
以O_RDWR|O_CREAT模式的调用open打开不同的文件直至打开失败；判断最后一次的返回值。

【预期结果】:
返回值为-1且错误码为ENFILE。

【评价准则】:
与预期的测试结果一致

 */


/**************************** 引用部分 *****************************************/
#include <stdio.h>
#include <utime.h>
#include <errno.h>
#include <fcntl.h>
#include "test.h"
/*************************** 前向声明部分 ****************************************/
/**************************** 定义部分 *****************************************/
/****************************** 实现部分 *********************************/
int	OS_FS_Open_ENFILE()
{
    int fp[260];
    int isOK = 0;
    int i = 0;
    int k = 0;
    char name[255] = {0};
    errno = 0;


    for (i = 0; i < 260; i++)
    {
    	sprintf(name,"%s/testfile%d.txt", FS_ROOT, i);
        fp[i]=-1;
        fp[i] = open(name, O_RDWR | O_CREAT, 0777);
        if (fp[i] == -1)
        {
            break;
        }
    }

    if (errno == ENFILE)
    {
        isOK = 1;
    }
    else
    {
    	printf("errno == %d\n",errno);
    }
    
    for (i = 0; i < 260; i++)
    {
    	sprintf(name,"%s/testfile%d.txt", FS_ROOT, i);
        if(close(fp[i]) !=0)
        		{
        			break;
        		}
    }


    for (k = 0; k < i; k++)
    {
    	sprintf(name,"%s/testfile%d.txt", FS_ROOT, k);
        fp[k] = remove(name);
       	if (fp[k] == -1)
        {
            printf("**********   fail  ************* \n");
            break;
        }
    }

    if (isOK == 1)
    {
        TEST_OKPRINT();
        return PTS_PASS;
    }
    else
    {
        TEST_ERRPRINT("open");
        return PTS_FAIL;
    }
}
