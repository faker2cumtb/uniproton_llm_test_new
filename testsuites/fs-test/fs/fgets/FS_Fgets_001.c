/*
【测试目的】
fgets从指定文件中读数据，数据正确

【测试类型】
功能测试

【测试步骤】
1、打开文件
2、fgets从指定文件中读数据，判断数据正确。

【预期结果】
fgets从指定文件中读数据，数据正确

【评价准则】
与预期结果一致

*/
/**************************** 头文件部分 ***************************************/
#include <stdio.h>
#include <stdlib.h>
#include <test.h>
#include <print.h>

/************************全局变量******************************/
#define num_test 2050

/**************************** 实现部分 ***************************************/
int OS_FS_Fgets_001(void)
{
	FILE *pfile=NULL;
	char filename[100] = FS_ROOT;
	char buf[128] = "0123456789aaaaaa0123456789aaaaaa0123456789aaaaaa0123456789aaaaa\n";
	char buf_fread[128] = {0};
	char * flag = NULL;
	char * str = NULL;
	char f_str[256] = {0};
	int i =  0;
	int ret=0;
	size_t iRet = 0;
	size_t file_len = 0;
	size_t fread_len = 0;

	strcat(filename, "/test_fgets.txt");
	str = calloc(num_test, 1);
	if(str == NULL)
	{
		printf("test failed\n");
		TSTDEF_ERRPRINT(errno);
		return -1;
	}

	flag = calloc(num_test, 1);
	if(flag == NULL)
	{
		printf("test failed\n");
		TSTDEF_ERRPRINT(errno);
		return -1;
	}
	else {
		for(i = 0; i < 32; i++)
		{
			strcat(flag, buf);
		}
	}

	pfile = fopen(filename, "a+");
	if(pfile == NULL)
	{
		TSTDEF_ERRPRINT(errno);
	     return -1;
	}

	for(i = 0; i < 32; i++)
	{
		iRet = fwrite(buf, 1, strlen(buf), pfile);
		if(iRet != strlen(buf))
		{
			printf("fwrite errno:%d iRet: %d \n",errno,iRet);
			fclose(pfile);
			remove(filename);
			free(str);
			free(flag);
			TSTDEF_ERRPRINT(errno);
			return -1;
		}
	}

	file_len = ftell(pfile);
	if(file_len == -1)
	{
		printf("ftell errno:%d \n",errno);
		fclose(pfile);
		remove(filename);
		free(str);
		free(flag);
		TSTDEF_ERRPRINT(errno);
		return -1;
	}

	iRet = fseek(pfile, 0, SEEK_SET);
	if(iRet != 0)
	{
		printf("fwrite errno:%d iRet: %d \n",errno,iRet);
		fclose(pfile);
		remove(filename);
		free(str);
		free(flag);
		TSTDEF_ERRPRINT(errno);
		return -1;
	}

	while (fread_len < file_len)
	{
		iRet = fread(str, 1, strlen(flag), pfile);
		fread_len += iRet;
	}

	ret = strcmp(flag, str);
	if(ret != 0)
	{
		printf("strcmp errno:%d\n str: %s \n",errno,str);
		fclose(pfile);
		remove(filename);
		free(str);
		free(flag);
		TSTDEF_ERRPRINT(errno);
		return -1;
	}

	memset(str, 0, sizeof(str));
	iRet=strcmp(flag,str);
	if(iRet == 0)
	{
		printf("str: %s \n",str);
		remove(filename);
		free(str);
		free(flag);
		TSTDEF_ERRPRINT(errno);
		return -1;
	}

	fclose(pfile);

	pfile = fopen(filename, "rb");
	while(fgets(f_str, 256, (FILE*)pfile))
	{
		strcat(str, f_str);
		printf("%s\n-----\n",f_str);
	}
	fclose(pfile);

	iRet = strcmp(flag, str);
	if(iRet == 0)
	{
		TEST_OKPRINT();
		remove(filename);
		free(str);
		free(flag);
		return 0;
	}
	else
	{
		printf("fgets errno:%d\n iRet: %s \n",errno,str);
		remove(filename);
		free(str);
		free(flag);
		TSTDEF_ERRPRINT(errno);
		return -1;
	}

	return 0;
}
