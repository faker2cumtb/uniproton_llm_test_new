
/*
【测试目的】:
当输入参数有效时，测试truncate截断文件是否成功。

【测试类型】:
接口测试

【先决条件】:
操作系统已运行。

【测试步骤】:
当输入参数有效时，调用truncate截断文件；判断返回值。

【预期结果】:
返回值为0。

【评价准则】:
与预期的测试结果一致

 */

/**************************** 引用部分 *****************************************/
#include <stdio.h>
#include <utime.h>
#include <errno.h>
#include <fcntl.h>
#include "test.h"

int OS_FS_Truncate_001()
{
    int ret = 0,i=0,j=0;
    int fd;
    int flag = 0;
    char str[11] = "1111111111";
    char rmdirfile[50] = FS_ROOT;
    FILE *fp={0};
    signed char rbuf[11]="\0";

    strcat(rmdirfile, "/TRUNCATE001.txt");

    remove(rmdirfile);
    fd = open(rmdirfile,O_RDWR|O_CREAT, 0777);
    if (fd < 0)
    {
    	TSTDEF_FAILPRINT(errno);
        flag = 1;
    }

    ret = write(fd, str, 8);
    if (ret != 8)
    {
    	TSTDEF_FAILPRINT(errno);
        flag = 1;
    }

	close(fd);
    ret = truncate(rmdirfile, 5);

    if (ret != 0)
    {
    	TSTDEF_FAILPRINT(errno);
        flag = 1;
    }

    fp = fopen(rmdirfile, "r+");
    if (fp ==NULL)
    {
    	TSTDEF_FAILPRINT(errno);
        return PTS_FAIL;
    }

    i=0;
    while(!(feof(fp)))
    {
    	rbuf[i]=getc(fp);
		if(rbuf[i]==EOF)
		{
			break;
		}
    	i++;

    }

    for(i=0;i<5;i++)
    {

    	if(rbuf[i] !='1')
    	{
    		flag = 1;
    	}
    }

#if CONFIG_NFS == 0
    fclose(fp);
#endif
    ret = truncate(rmdirfile, 10);

    if (ret != 0)
    {
        TEST_FAILPRINT("truncate");
        flag = 1;
    }


    fp = fopen(rmdirfile, "r+");
    if (fp == NULL)
    {
        TEST_ERRPRINT("fopen");
        return PTS_FAIL;
    }

    i=0;
    while(!(feof(fp)))
    {
    	rbuf[i]=getc(fp);
		if(rbuf[i]==EOF)
		{
			break;
		}
    	i++;

    }

    if(i!=10)
    {

    	flag = 1;
    }

    for(j=0;j<i;j++)
    {
    	if(j<5)
    	{

        	if(rbuf[j] !='1')
        	{
        		flag = 1;
        	}
    	}
    	else
    	{

         	if(rbuf[j] !='\0')
            	{
            		flag = 1;
            	}
		}

    }


    fclose(fp);
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
