
/* 
【测试目的】:
创建有效的文件描述符，调用fstat成功获取修改该文件的访问和修改时间 。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
创建有效的文件描述符，进行读写操作，调用fstat获取修改该文件的访问和修改时间  。

【预期结果】:
fstat成功获取修改该文件的访问和修改时间 。

【评价准则】:
与预期的测试结果一致
*/

/**************************** 引用部分 *****************************************/
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <utime.h>
#include <test.h>
/*************************** 前向声明部分 ****************************************/
/**************************** 定义部分 *****************************************/
/****************************** 实现部分 *********************************/

int	OS_FS_Fstat_001()
{
	int fp =0,ret=0;
    ssize_t ret1=0;
    char filename[100] =FS_ROOT;
    struct stat sbuf={0};
    struct stat sbuf1={0},sbuf2={0};
    const char wbuf[20]="0123456789";

    strcat(filename, "/utime.txt");
    remove(filename);
    fp=open((const char *)filename,O_RDWR |O_TRUNC |O_CREAT);
    if(fp==-1)
    {
    	printf("open failed ,errno is %d \n",errno);
    	printf("test failed\n");
    }
    close(fp);

	fp=open((const char *)filename,O_RDWR);
    if(fp==-1)
    {
    	printf("open failed ,errno is %d \n",errno);
    	printf("test failed\n");
    }
    ret=fstat(fp,&sbuf);
    if(ret!=0)
    {
    	printf("stat failed ,errno is %d ret is %d\n",errno,ret);
    	printf("test failed\n");
    }

    close(fp);
	
	 sleep(5);
	
    fp=open((const char *)filename,O_RDWR|O_TRUNC );
    if(fp==-1)
    {
    	printf("open failed ,errno is %d \n",errno);
    	printf("test failed\n");
    }
	
    ret1=write(fp, wbuf, 10);
    if(ret==-1)
    {
      	printf("write failed ,errno is %d ret is %d\n",errno,ret);
        printf("test failed\n");
    }
	
    ret1=read(fp, 1, 1);
    if(ret==-1)
    {
      	printf("write failed ,errno is %d ret is %d\n",errno,ret);
        printf("test failed\n");
    }	

	 sleep(3);

    ret=fstat(fp,&sbuf1);
    if(ret!=0)
    {
    	printf("stat failed ,errno is %d ret is %d\n",errno,ret);
    	printf("test failed\n");
    }
		
    if(sbuf.st_atime>=sbuf1.st_atime || sbuf.st_mtime>=sbuf1.st_mtime) //st_atime:time of last access ; st_mtime:time of the last modification
    {
    	printf("stat sttime is%d  ottime is%d\n",sbuf.st_atime,sbuf.st_mtime);
    	printf("stat sttime is%d  ottime is%d\n",sbuf1.st_atime,sbuf1.st_mtime);
    	TEST_FAILRINT();
    }
    else
    {
    	TEST_OKPRINT();
    }

    close(fp);
    remove(filename);
}
