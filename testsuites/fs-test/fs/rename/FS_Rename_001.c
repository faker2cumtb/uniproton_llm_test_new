/* 

【测试目的】:
调用rename目录文件进行重命名是否成功。

【测试类型】:
功能性测试

【先决条件】:
1.文件系统成功挂载至目标机。

【测试步骤】:
1.调用mkdir创建目录，rename创建的目录是否成功；
2.再调用open创建文件，rename创建的文件是否成功
3.再创建原重命名前的文件和目录，验证rename是否成功。

【预期结果】:
返回PASS

*/


/****************************** 实现部分 *********************************/

#include <stdio.h>
#include <test.h>

int OS_FS_Rename_001(void)
{
	int ret;
	unsigned char dirpath[64] =FS_ROOT;
	unsigned char newpath[64] =FS_ROOT;
	unsigned char pathfile[100]={0};
	unsigned char newfile[100]={0};
	unsigned char str[]="hello word!!";
	FILE *fp=NULL;

	strcat(dirpath,"/dirpath");
	strcat(newpath,"/newpath");
	rmdir(dirpath);
	rmdir(newpath);

	if ((ret = mkdir(dirpath, 777)) !=0)
	{
		printf("\n mkdir error  ret:%d", ret);
		TSTDEF_ERRPRINT(errno);
		return -1;
	}

	if ((ret = rename(dirpath, newpath)) !=0)
	{
		printf("\n rename error  ret:%d", ret);
		TSTDEF_ERRPRINT(errno);
		return -1;
	}

	sprintf(pathfile, "%s/%s", newpath, "testfile.bin");
	printf("\n%s", pathfile);

	if ((fp = fopen(pathfile, "w+")) ==NULL)
	{
		printf("\n fopen w+ error  ret:%d", ret);
		TSTDEF_ERRPRINT(errno);
		return -1;
	}

	if((ret = fwrite(str, 1, sizeof(str), fp)) != sizeof(str))
	{
		printf("\n fwrite size  ret:%d  size:%d ", ret, sizeof(str));
		TSTDEF_ERRPRINT(errno);
		return -1;
	}

	if ((ret = fclose(fp)) != 0)
	{
		printf("\n fclose error  ret:%d", ret);
		TSTDEF_ERRPRINT(errno);
		return -1;
	}

	sprintf(newfile, "%s/%s", newpath, "newtfile.bin");
	printf("\n %s", newfile);

	if ((ret = rename(pathfile, newfile)) !=0)
	{
		printf("\n rename error  ret:%d", ret);
		TSTDEF_ERRPRINT(errno);
		return -1;
	}

	if ((fp = fopen(newfile, "r")) ==NULL)
	{
		printf("\n fopen r error  ret:%d", ret);
		TSTDEF_ERRPRINT(errno);
		return -1;
	}

	if ((ret = fclose(fp)) != 0)
	{
		printf("\n fclose error  ret:%d", ret);
		TSTDEF_ERRPRINT(errno);
		return -1;
	}

	if ((ret = rename(newpath, dirpath)) !=0)
	{
		printf("\n rename error  ret:%d", ret);
		TSTDEF_ERRPRINT(errno);
		return -1;
	}

	sprintf(pathfile, "%s/%s", dirpath, "newtfile.bin");
	printf("\n %s", pathfile);

	if ((fp = fopen(pathfile, "r")) ==NULL)
	{
		printf("\n fopen r error  ret:%d", ret);
		TSTDEF_ERRPRINT(errno);
		return -1;
	}

	if ((ret = fclose(fp)) != 0)
	{
		printf("\n fclose error  ret:%d", ret);
		TSTDEF_ERRPRINT(errno);
		return -1;
	}

	
	if ((ret = mkdir(newpath, 777)) !=0)
	{
		printf("\n mkdir error  ret:%d", ret);
		TSTDEF_ERRPRINT(errno);
		return -1;
	}
	sprintf(newfile, "%s/%s", newpath, "testfile.bin");
	printf("\n %s", newfile);
	
	if ((ret = rename(pathfile, newfile)) !=0)
	{
		printf("\n rename error  ret:%d", ret);
		TSTDEF_ERRPRINT(errno);
		return -1;
	}

	if ((fp = fopen(newfile, "r")) ==NULL)
	{
		printf("\n fopen r error  ret:%d", ret);
		TSTDEF_ERRPRINT(errno);
		return -1;
	}

	if ((ret = fclose(fp)) != 0)
	{
		printf("\n fclose error  ret:%d", ret);
		TSTDEF_ERRPRINT(errno);
		return -1;
	}
	else
	{
		printf("\n  rename test OK !!\n");
		TEST_OKPRINT();
        remove(newfile);
        remove(pathfile);
        rmdir(newpath);
        rmdir(dirpath);
		return 0;

	}

}
