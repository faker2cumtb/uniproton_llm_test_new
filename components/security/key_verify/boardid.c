#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "boardid.h"

#define MMC_INFO_DIR "/sys/class/block/mmcblk0/device/serial"
#define MAC_INFO_DIR "/sys/class/net/eth0/address"
#define CPU_INFO_DIR "/proc/cpuinfo"

#define BUFFER_SIZE 256
#define FILE_NAME_LEN 100


int get_mmc_uuid(char* uuid_buf, unsigned int buf_len, unsigned int index) {
    // 打开指定的文件
    char file_name[FILE_NAME_LEN];
    char temp_uuid_buf[UUID_BUFFER_LEN]={0};

    sprintf(file_name, MMC_INFO_DIR, index);
    FILE *file = fopen(file_name, "r");
    if (!file) {
        printf("Cannot find the device index: %d.\n", index);
        return -1; // 返回错误
    }

    // 读取内容到缓冲区
    if (fgets(temp_uuid_buf, UUID_BUFFER_LEN, file) == NULL) {
        printf("can not read uuid info.\n");
        fclose(file);
        return -1; // 返回错误
    }

    // 关闭文件
    fclose(file);

    // 去掉末尾的换行符
    temp_uuid_buf[strcspn(temp_uuid_buf, "\n")] = '\0';

    // 去掉前缀 "0x"，如果存在
    if (strncmp(temp_uuid_buf, "0x", 2) == 0) {
        memmove(temp_uuid_buf, temp_uuid_buf + 2, strlen(temp_uuid_buf) - 1); // 移动字符
    }

    if (strlen(temp_uuid_buf) >= buf_len)
    {
        printf("input buffer len: %d less than uuid len: %ld.\n", buf_len, strlen(temp_uuid_buf));
        return -1;
    }
    strcpy(uuid_buf, temp_uuid_buf);

    return 0; // 成功
}

int get_mac_uuid(char* uuid_buf, unsigned int buf_len, unsigned int index) {
    // 打开指定的文件
    char file_name[FILE_NAME_LEN];
    char temp_uuid_buf[UUID_BUFFER_LEN] = {0};

    sprintf(file_name, MAC_INFO_DIR, index);
    FILE *file = fopen(file_name, "r");
    if (!file) {
        printf("Cannot find the device index: %d.\n", index);
        return -1; // 返回错误
    }

    // 读取内容到缓冲区
    if (fgets(temp_uuid_buf, UUID_BUFFER_LEN, file) == NULL) {
        printf("can not read uuid info.\n");
        fclose(file);
        return -1; // 返回错误
    }

    // 关闭文件
    fclose(file);

    // 去掉末尾的换行符
    temp_uuid_buf[strcspn(temp_uuid_buf, "\n")] = '\0';

    // 去掉冒号符号
    char temp_buf[UUID_BUFFER_LEN] = {0}; // MAC 地址的最大长度为 17（包括 6 个 : 和结束符）
    int j = 0;

    for (int i = 0; temp_uuid_buf[i] != '\0'; i++) {
        if (temp_uuid_buf[i] != ':') {
                temp_buf[j++] = temp_uuid_buf[i];
        }
    }
    temp_buf[j] = '\0'; // 添加字符串结束符

    // 复制处理后的 MAC 地址到 uuid_buf
    if (strlen(temp_buf) >= buf_len)
    {
        printf("input buffer len: %d less than uuid len: %ld.\n", buf_len, strlen(temp_buf));
        return -1;
    }
    strcpy(uuid_buf, temp_buf);

    return 0; // 成功
}

int get_cpu_uuid(char* uuid_buf, unsigned int buf_len, unsigned int index) {
    // 打开指定的文件
    char serial_number[UUID_BUFFER_LEN] = {0};
    char file_name[FILE_NAME_LEN];
    
    sprintf(file_name, CPU_INFO_DIR);

    FILE *file = fopen(file_name, "r");
    if (!file) {
        printf("Cannot find the device index: %d.\n", index);
        return -1; // 返回错误
    }

    char line[BUFFER_SIZE];

    while (fgets(line, sizeof(line), file)) {
        // 查找包含 "Serial" 的行
        if (strncmp(line, "Serial", 6) == 0) {
            // 提取序列号
            sscanf(line, "Serial : %s", serial_number);
            fclose(file);
            if (strlen(serial_number) >= buf_len)
            {
                printf("input buffer len: %d less than uuid len: %ld.\n", buf_len, strlen(serial_number));
                return -1;
            }
            strcpy(uuid_buf, serial_number);
            return 0;
        }
    }

    fclose(file);

    return -1; // 找不到序列号
}

void format_uuid(const char *uuid_in, char *uuid_out, unsigned int format_len) {
    int uuid_len = strlen(uuid_in);

    if (uuid_len > format_len) {
        // 如果输入 UUID 超过目标长度，则截取后面长度为 format_len 的字符
        strncpy(uuid_out, uuid_in + (uuid_len - format_len), format_len);
        uuid_out[format_len] = '\0';  // 确保字符串以 null 结尾
    } else {
        // 前面填充字符
        int i = 0;
        for (; i < format_len - uuid_len; i++) {
            uuid_out[i] = '0';  // 填充字符
        }
        uuid_out[i] = '\0';  // 确保字符串以 null 结尾
        // 如果输入 UUID 不足目标长度，则填充
        strcat(uuid_out, uuid_in);
    }
}
