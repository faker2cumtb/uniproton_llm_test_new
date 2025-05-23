#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "key_verify.h"

#if !defined(SECURITY_MBEDTLS)
#error "SECURITY_MBEDTLS is not defined\n"
#endif

#include <mbedtls/sha256.h>
#include <mbedtls/md5.h>
#include <mbedtls/version.h>

#include "boardid.h"
#include "algorithm.h"
// 函数声明
#define SHA256_DIGEST_LENGTH   32
// XOR 加密函数实现
void xor_encrypt(char *input, const char *key) {
    size_t key_length = strlen(key);
    for (size_t i = 0; i < strlen(input); i++) {
        input[i] ^= key[i % key_length];
    }
}

// 哈希函数实现
void hash_string(const char *input, unsigned char output[SHA256_DIGEST_LENGTH]) {
    mbedtls_sha256_context sha256_ctx;

    mbedtls_sha256_init(&sha256_ctx);
    mbedtls_sha256_starts(&sha256_ctx, 0);
    mbedtls_sha256_update(&sha256_ctx, input, strlen(input));
    mbedtls_sha256_finish(&sha256_ctx, output);
}

// 将哈希值转换为十六进制字符串
void hash_to_hex_string(const unsigned char *hash, char *output) {
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
}

// 组合算法为 mac0.hash()+mmc0.xor("sdfsdf")
int combination_algorithm(char* buf, unsigned int buf_len) {
    // 示例输入
    char mac0[UUID_BUFFER_LEN];
    char mmc0[UUID_BUFFER_LEN];
    const char *xor_key = "sdfsdf";
    int ret;

    ret = get_mac_uuid(mac0, UUID_BUFFER_LEN, 0);
    if (ret) {
        printf("Error getting MAC UUID: %d\n", ret);
        return -1;
    }
    // printf("MAC UUID: %s\n", mac0);  // 打印 mac0 的值

    ret = get_mmc_uuid(mmc0, UUID_BUFFER_LEN, 0);
    if (ret) {
        printf("Error getting MMC UUID: %d\n", ret);
        return -1;
    }
    // printf("MMC UUID: %s\n", mmc0);  // 打印 mmc0 的值

    // 处理 mac0
    unsigned char hash_output[SHA256_DIGEST_LENGTH];
    hash_string(mac0, hash_output);

    // 将哈希值转换为十六进制字符串
    char hash_string_hex[SHA256_DIGEST_LENGTH * 2 + 1];
    hash_to_hex_string(hash_output, hash_string_hex);
    hash_string_hex[SHA256_DIGEST_LENGTH * 2] = '\0';  // 确保以 null 结尾
    // printf("Hash of MAC UUID: %s\n", hash_string_hex);  // 打印哈希值

    // 处理 mmc0
    char mmc0_xor[50];  // 确保缓冲区足够大
    strcpy(mmc0_xor, mmc0);
    xor_encrypt(mmc0_xor, xor_key);
    // printf("XOR Encrypted MMC UUID: %s\n", mmc0_xor);  // 打印 XOR 加密后的结果

    // 拼接结果
    if (buf_len < strlen(hash_string_hex) + strlen(mmc0_xor) + 2) { // +2 for "+\0"
        printf("Buffer length is insufficient\n");
        return -1;
    }

    snprintf(buf, buf_len, "%s-%s", hash_string_hex, mmc0_xor);

    // 打印最终结果
    // printf("Final Result: %s\n", buf);

    return 0;
}