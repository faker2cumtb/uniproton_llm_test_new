#ifndef _CONFORMSNCE_RUN_TEST_H
#define _CONFORMSNCE_RUN_TEST_H

int tstdef_CreateFile(char* ch);
int tstdef_CreateAndWriteFile(char* ch,char* str);

extern int OS_FS_Chdir_001();
extern int OS_FS_Chdir_EINVAL();
extern int OS_FS_Chdir_ENOTDIR();
extern int OS_FS_Close_01();
extern int OS_FS_Close_02();
extern int OS_FS_Fcntl_001();
extern int OS_FS_Fcntl_002();
extern int OS_FS_Fgets_001();
extern int OS_FS_Fseek_001();
extern int OS_FS_Fseek_002();
extern int OS_FS_Fseek_003();
extern int OS_FS_Fseek_004();
extern int OS_FS_Fseek_005();
extern int OS_FS_Fseek_006();
extern int OS_FS_Fseek_007();
extern int OS_FS_Fseek_008();
extern int OS_FS_Fsetpos_001();
extern int OS_FS_Fsetpos_002();
extern int OS_FS_Fsetpos_003();
extern int OS_FS_Fsetpos_004();
extern int OS_FS_Fstat_001();
extern int OS_FS_Fstat_EBADF();
extern int OS_FS_Fstat_002();
extern int OS_FS_Fsync_001();
extern int OS_FS_Fsync_EBADF();
extern int OS_FS_Ftell_001();
extern int OS_FS_Ftell_002();
extern int OS_FS_Ftell_003();
extern int OS_FS_Ftruncate_001();
extern int OS_FS_Ftruncate_002();
extern int OS_FS_Ftruncate_EBADF();
extern int OS_FS_Ftruncate_EINVAL();
extern int OS_FS_Getcwd_001();
extern int OS_FS_Getcwd_EINVAL();
extern int OS_FS_Getcwd_EINVAL2();
extern int OS_FS_Lseek_EBADF();
extern int OS_FS_Lseek_EINVAL();
extern int OS_FS_Lseek_EOVERFLOW();
extern int OS_FS_Lseek_001();
extern int OS_FS_Lseek_002();
extern int OS_FS_Lseek_003();
extern int OS_FS_Lseek_004();
extern int OS_FS_Mkdir_01();
extern int OS_FS_Mkdir_EEXIST();
extern int OS_FS_Mkdir_ENAMETOOLONG();
extern int OS_FS_Mkdir_ENOENT();
extern int OS_FS_Mkdir_ENOTDIR();
extern int OS_FS_Open_001();
extern int OS_FS_Open_ENXIO();
extern int OS_FS_Open_EISDIR();
extern int OS_FS_Open_EEXIST();
extern int OS_FS_Open_ENAMETOOLONG();
extern int OS_FS_Open_ENOENT();
extern int OS_FS_Open_EINVAL();
extern int OS_FS_Open_ENOTDIR();
extern int OS_FS_Open_EROFS();
extern int OS_FS_Open_ENFILE();
extern int OS_FS_Read_001();
extern int OS_FS_Read_EBADF1();
extern int OS_FS_Read_EBADF2();
extern int OS_FS_Read_EISDIR();
extern int OS_FS_Rename_001();
extern int OS_FS_Rename_002();
extern int OS_FS_Rename_ENAMETOOLONG();
extern int OS_FS_Rename_ENOENT();
extern int OS_FS_Rename_ENOTDIR();
extern int OS_FS_Rename_EXDEV();
extern int OS_FS_Rmdir_01();
extern int OS_FS_Rmdir_EBUSY();
extern int OS_FS_Rmdir_ENAMETOOLONG();
extern int OS_FS_Rmdir_ENOENT();
extern int OS_FS_Rmdir_ENOENT2();
extern int OS_FS_Rmdir_ENOTEMPTY();
extern int OS_FS_Rmdir_ENOTDIR();
extern int OS_FS_Stat_01();
extern int OS_FS_Stat_ENAMETOOLONG();
extern int OS_FS_Stat_ENOTDIR();
extern int OS_FS_Truncate_001();
extern int OS_FS_Truncate_ENAMETOOLONG();
extern int OS_FS_Truncate_ENOENT();
extern int OS_FS_Truncate_ENOTDIR();
extern int OS_FS_Truncate_EISDIR();
extern int OS_FS_Truncate_EINVAL();
extern int OS_FS_Write_001();
extern int OS_FS_Write_EBADF1();
extern int OS_FS_Write_EBADF2();

typedef int test_run_main();

test_run_main *run_test_arry_1[] = {
    OS_FS_Chdir_001,
    OS_FS_Chdir_EINVAL,
    OS_FS_Chdir_ENOTDIR,
    OS_FS_Close_01,
    OS_FS_Close_02,
    OS_FS_Fcntl_001,
    OS_FS_Fcntl_002,
    OS_FS_Fgets_001,
    OS_FS_Fseek_001,
    OS_FS_Fseek_002,
    OS_FS_Fseek_003,
    OS_FS_Fseek_004,
    OS_FS_Fseek_005,
    OS_FS_Fseek_006,
    OS_FS_Fseek_007,
    OS_FS_Fseek_008,
    OS_FS_Fstat_001,
    OS_FS_Fstat_002,
    OS_FS_Fstat_EBADF,
    OS_FS_Fsync_001,
    OS_FS_Fsync_EBADF,
    OS_FS_Ftell_001,
    OS_FS_Ftell_002,
    OS_FS_Ftell_003,
    OS_FS_Ftruncate_001,
    OS_FS_Ftruncate_002,
    OS_FS_Ftruncate_EBADF,
    OS_FS_Ftruncate_EINVAL,
    OS_FS_Getcwd_001,
    OS_FS_Getcwd_EINVAL,
    OS_FS_Getcwd_EINVAL2,
    OS_FS_Lseek_EBADF,
    OS_FS_Lseek_EINVAL,
    OS_FS_Lseek_EOVERFLOW,
    OS_FS_Lseek_001,
    OS_FS_Lseek_002,
    OS_FS_Lseek_003,
    OS_FS_Lseek_004,
    OS_FS_Mkdir_01,
    OS_FS_Mkdir_EEXIST,
    OS_FS_Mkdir_ENAMETOOLONG,
    OS_FS_Mkdir_ENOENT,
    OS_FS_Mkdir_ENOTDIR,
    OS_FS_Open_001,
    OS_FS_Open_ENXIO,
    OS_FS_Open_EISDIR,
    OS_FS_Open_EEXIST,
    OS_FS_Open_ENAMETOOLONG,
    OS_FS_Open_ENOENT,
    OS_FS_Open_EINVAL,
    OS_FS_Open_ENOTDIR,
    OS_FS_Open_EROFS,
    OS_FS_Open_ENFILE,
    OS_FS_Read_001,
    OS_FS_Read_EBADF1,
    OS_FS_Read_EBADF2,
    OS_FS_Read_EISDIR,
    OS_FS_Rename_001,
    OS_FS_Rename_002,
    OS_FS_Rename_ENAMETOOLONG,
    OS_FS_Rename_ENOENT,
    OS_FS_Rename_ENOTDIR,
    OS_FS_Rename_EXDEV,
    OS_FS_Rmdir_01,
    OS_FS_Rmdir_EBUSY,
    OS_FS_Rmdir_ENAMETOOLONG,
    OS_FS_Rmdir_ENOENT,
    OS_FS_Rmdir_ENOENT2,
    OS_FS_Rmdir_ENOTEMPTY,
    OS_FS_Rmdir_ENOTDIR,
    OS_FS_Stat_01,
    OS_FS_Stat_ENAMETOOLONG,
    OS_FS_Stat_ENOTDIR,
    OS_FS_Truncate_001,
    OS_FS_Truncate_ENAMETOOLONG,
    OS_FS_Truncate_ENOENT,
    OS_FS_Truncate_ENOTDIR,
    OS_FS_Truncate_EISDIR,
    OS_FS_Truncate_EINVAL,
    OS_FS_Write_001,
    OS_FS_Write_EBADF1,
    OS_FS_Write_EBADF2,
};


char run_test_name_1[][50] = {
    "OS_FS_Chdir_001",
    "OS_FS_Chdir_EINVAL",
    "OS_FS_Chdir_ENOTDIR",
    "OS_FS_Close_01",
    "OS_FS_Close_02",
    "OS_FS_Fcntl_001",
    "OS_FS_Fcntl_002",
    "OS_FS_Fgets_001",
    "OS_FS_Fseek_001",
    "OS_FS_Fseek_002",
    "OS_FS_Fseek_003",
    "OS_FS_Fseek_004",
    "OS_FS_Fseek_005",
    "OS_FS_Fseek_006",
    "OS_FS_Fseek_007",
    "OS_FS_Fseek_008",
    "OS_FS_Fstat_001",
    "OS_FS_Fstat_002",
    "OS_FS_Fstat_EBADF",
    "OS_FS_Fsync_001",
    "OS_FS_Fsync_EBADF",
    "OS_FS_Ftell_001",
    "OS_FS_Ftell_002",
    "OS_FS_Ftell_003",
    "OS_FS_Ftruncate_001",
    "OS_FS_Ftruncate_002",
    "OS_FS_Ftruncate_EBADF",
    "OS_FS_Ftruncate_EINVAL",
    "OS_FS_Getcwd_001",
    "OS_FS_Getcwd_EINVAL",
    "OS_FS_Getcwd_EINVAL2",
    "OS_FS_Lseek_EBADF",
    "OS_FS_Lseek_EINVAL",
    "OS_FS_Lseek_EOVERFLOW",
    "OS_FS_Lseek_001",
    "OS_FS_Lseek_002",
    "OS_FS_Lseek_003",
    "OS_FS_Lseek_004",
    "OS_FS_Mkdir_01",
    "OS_FS_Mkdir_EEXIST",
    "OS_FS_Mkdir_ENAMETOOLONG",
    "OS_FS_Mkdir_ENOENT",
    "OS_FS_Mkdir_ENOTDIR",
    "OS_FS_Open_001",
    "OS_FS_Open_ENXIO",
    "OS_FS_Open_EISDIR",
    "OS_FS_Open_EEXIST",
    "OS_FS_Open_ENAMETOOLONG",
    "OS_FS_Open_ENOENT",
    "OS_FS_Open_EINVAL",
    "OS_FS_Open_ENOTDIR",
    "OS_FS_Open_EROFS",
    "OS_FS_Open_ENFILE",
    "OS_FS_Read_001",
    "OS_FS_Read_EBADF1",
    "OS_FS_Read_EBADF2",
    "OS_FS_Read_EISDIR",
    "OS_FS_Rename_001",
    "OS_FS_Rename_002",
    "OS_FS_Rename_ENAMETOOLONG",
    "OS_FS_Rename_ENOENT",
    "OS_FS_Rename_ENOTDIR",
    "OS_FS_Rename_EXDEV",
    "OS_FS_Rmdir_01",
    "OS_FS_Rmdir_EBUSY",
    "OS_FS_Rmdir_ENAMETOOLONG",
    "OS_FS_Rmdir_ENOENT",
    "OS_FS_Rmdir_ENOENT2",
    "OS_FS_Rmdir_ENOTEMPTY",
    "OS_FS_Rmdir_ENOTDIR",
    "OS_FS_Stat_01",
    "OS_FS_Stat_ENAMETOOLONG",
    "OS_FS_Stat_ENOTDIR",
    "OS_FS_Truncate_001",
    "OS_FS_Truncate_ENAMETOOLONG",
    "OS_FS_Truncate_ENOENT",
    "OS_FS_Truncate_ENOTDIR",
    "OS_FS_Truncate_EISDIR",
    "OS_FS_Truncate_EINVAL",
    "OS_FS_Write_001",
    "OS_FS_Write_EBADF1",
    "OS_FS_Write_EBADF2",
};

#endif
