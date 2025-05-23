/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * 	http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2023-11-20
 * Description: openamp backend
 */

#include "openamp/open_amp.h"
#include "openamp_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RPMSG_VIRTIO_CONSOLE_CONFIG        \
    (&(const struct rpmsg_virtio_config) { \
        .h2r_buf_size = 2048,  \
        .r2h_buf_size = 2048,  \
        .split_shpool = false,\
})

void reset_vq(void);
int create_rpmsg_device(void);
void rpmsg_backend_remove(void);
struct rpmsg_device *get_rpmsg_device();
void deal_rpmsg_msg();

#ifdef __cplusplus
}
#endif
