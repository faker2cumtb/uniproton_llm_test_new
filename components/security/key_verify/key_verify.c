#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "algorithm.h"
#include "key_verify.h"

static const char *directory = "/auth";
static const char *filename = "/.auth/.auth.bin";  // 保存结果的文件名

int key_verify() 
{
    char calculated_result[1024];  // 假设组合算法的结果不会超过1024字节
    char saved_result[1024];  // 存储文件中的结果
    size_t saved_result_len;

    // 调用组合算法
    int result = combination_algorithm(calculated_result, sizeof(calculated_result));

    // 检查组合算法的返回值
    if (result < 0) {
        printf("Combination algorithm failed.\n");
        return -1;
    }

    // 打开/auth/auth.bin文件以读取保存的结果
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to open file for reading\n");
        return -1;
    }

    // 读取保存的结果
    saved_result_len = fread(saved_result, sizeof(char), sizeof(saved_result), file);
    fclose(file);
    // 比较计算结果和保存的结果
    if (saved_result_len == strlen(calculated_result) && 
        memcmp(calculated_result, saved_result, saved_result_len) == 0) {
        return 0;
    } else {
        // printf("calculated value: %s\n", calculated_result);
        // printf("save value      : %s\n", saved_result);
        return -1;
    }
}