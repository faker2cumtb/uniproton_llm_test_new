/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-11-21     LuoYuncong   the first version
 */
#include <rtdevice.h>
#include <prt_task.h>
#include <prt_clk.h>
#include <print.h>
#include <string.h>
#include <demo.h>

#if !defined(SECURITY_MBEDTLS)
#error "SECURITY_MBEDTLS is not defined\n"
#endif

#include <mbedtls/sha256.h>
#include <mbedtls/md5.h>
#include <mbedtls/version.h>

static uint8_t test_data[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static uint8_t test_result[32];

void mbedtls_sha256_calc()
{
    mbedtls_sha256_context sha256_ctx;

    mbedtls_sha256_init(&sha256_ctx);
    mbedtls_sha256_starts(&sha256_ctx, 0);
    mbedtls_sha256_update(&sha256_ctx, test_data, strlen((char *)test_data));
    mbedtls_sha256_finish(&sha256_ctx, test_result);

    /* Result: D6EC6898DE87DDAC6E5B3611708A7AA1C2D298293349CC1A6C299A1DB7149D38 */
    PRT_Printf("SHA256: ");
    for (int i = 0; i < 32; i++)
    {
        PRT_Printf("%02X", test_result[i]);
    }
    PRT_Printf("\n");
}

void mbedtls_md5_calc()
{
    mbedtls_md5_context md5_ctx;

    mbedtls_md5_init(&md5_ctx);
    mbedtls_md5_starts(&md5_ctx);
    mbedtls_md5_update(&md5_ctx, test_data, strlen((char *)test_data));
    mbedtls_md5_finish(&md5_ctx, test_result);

    /* Result: 437BBA8E0BF58337674F4539E75186AC */
    PRT_Printf("MD5: ");
    for (int i = 0; i < 16; i++)
    {
        PRT_Printf("%02X", test_result[i]);
    }
    PRT_Printf("\n");
}

void mbedtls_demo()
{
    char version[16];

    mbedtls_version_get_string(version);
    PRT_Printf("mbed-tls version: %s\n", version);

    PRT_Printf("Data: ");
    for (int i = 0; i < sizeof(test_data); i++)
    {
        PRT_Printf("%c", test_data[i]);
    }
    PRT_Printf("\n");

    mbedtls_sha256_calc();

    mbedtls_md5_calc();
}
