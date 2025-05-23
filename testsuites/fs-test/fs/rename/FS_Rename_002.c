
/* 

【测试目的】:
测试 rename 接口正常功能。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
1.使用rename接口重命名与父目录不同的新文件名；
2.查看rename接口的返回值。

【预期结果】:
1.rename接口参数src或dest的父目录不相同时，重命名成功。

【评价准则】:
与预期的测试结果一致
 */

#include "test.h"
#include <pthread.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include<sys/types.h>
#include <errno.h>

OS_FS_Rename_002()
{
    int fd;
    int ret;
    int err = 0;
    char dirname[128]=FS_ROOT;
    char fname[128]=FS_ROOT;
    char dirname1[128]=FS_ROOT;
    char fname1[128]=FS_ROOT;
    char fnamex[128]=FS_ROOT;

    strcat(dirname, "/einDir");
    strcat(fname, "/einDir/file");
    strcat(dirname1, "/einDir1");
    strcat(fname1, "/einDir1/file1");
    strcat(fnamex, "/einDir1/file");

    remove(fname);
    remove(fname1);
    remove(fnamex);
    rmdir(dirname);
    rmdir(dirname1);


    mkdir(dirname,0777);
    fd = open(fname, O_RDWR|O_CREAT, 0777);
    if(fd < 0) {
    	TEST_FAILRINT();
    	printf("errno is %d\n",errno);
        err++;
    }

    close(fd);

   mkdir(dirname1,0777);

    //EINVAL 参数 src 或 dest 的父目录不相同；
    ret = rename(fname, fname1);
    //printf("%d %d ret:%d,errno:%d\n",__func__,__LINE__,ret,errno);
    if(ret != 0 ) {
    	TEST_FAILRINT();
    	printf("errno is %d\n",errno);
        err++;
    }

    fd=open(fname, O_RDWR);
    if(fd!=-1)
    {
    	TEST_FAILRINT();
        err++;
    }

    fd=open(fname1, O_RDWR);
    if(fd==-1)
    {
    	TEST_FAILRINT();
    	printf("errno is %d\n",errno);
        err++;
    }
    close(fd);

    fd=open(fnamex, O_RDWR);
    if(fd!=-1)
    {
    	TEST_FAILRINT();
    	printf("errno is %d\n",errno);
        err++;
    }
    
    if (err == 0)
    {
    	TEST_OKPRINT();
        close(fd);
        remove(fname);
        remove(fname1);
        remove(fnamex);
        rmdir(dirname);
        rmdir(dirname1);
    }
    else
    {
    	TEST_FAILRINT();
        close(fd);
        remove(fname);
        remove(fname1);
        remove(fnamex);
        rmdir(dirname);
        rmdir(dirname1);
	}
}
