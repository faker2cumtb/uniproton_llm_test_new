/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *     http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2024-04-08
 * Description: openamp rsctable backend
 */

#include <metal/device.h>
#include <metal/cache.h>
#include "rpmsg_backend.h"
#include "resource_table.h"
#include "prt_hwi.h"
#include "prt_sem.h"
#include "prt_sys.h"
#include "openamp_common.h"
#include "print.h"
/*
 * Use resource tables's  reserved[0] to carry some extra information.
 * The following IDs come from PSCI definition
 */


static struct virtio_device *vdev;
static struct rpmsg_virtio_device rvdev;
struct rpmsg_device *rpdev;

static metal_phys_addr_t shm_physmap[] = { VDEV_START_ADDR };
static struct metal_device shm_device = {
    .name = SHM_DEVICE_NAME,
    .num_regions = 2,
    .regions = {
        /*
         * shared memory io, only the addr in [share mem start + share mem size]
         * can be accessed and guaranteed by metal_io_read/write
         */
        {.virt = NULL},
        /*
         * resource table io, only the addr in [resource table start + table size]
         * can be accessed and guaranteed by metal_io_read/write
         */
        {.virt = NULL},
    },
    .node = { NULL },
    .irq_num = 0,
    .irq_info = NULL
};

static inline struct fw_rsc_vdev *rsc_table_to_vdev(void *rsc_table)
{
    return &((struct fw_resource_table *)rsc_table)->vdev;
}

static inline struct fw_rsc_vdev_vring *rsc_table_get_vring0(void *rsc_table)
{
    return &((struct fw_resource_table *)rsc_table)->vring0;
}

static inline struct fw_rsc_vdev_vring *rsc_table_get_vring1(void *rsc_table)
{
    return &((struct fw_resource_table *)rsc_table)->vring1;
}

#ifdef GUEST_OS_JAILHOUSE
#define JAILHOUSE_CELL_COMM 10
static inline long hvc_send_sgi(unsigned long cpu_id, unsigned long sgi_id)
{
    register long num_result asm("x0") = JAILHOUSE_CELL_COMM;
    register long __arg1 asm("x1") = cpu_id;
    register long __arg2 asm("x2") = sgi_id;

    asm volatile(
        "hvc #0x4a48"
        : "+r"(num_result), "+r"(__arg1), "+r"(__arg2)
        : : "memory", "x3");
    return num_result;
}
#endif

static int virtio_notify(void *priv, uint32_t id)
{
#ifdef GUEST_OS_JAILHOUSE
    hvc_send_sgi(0, OS_HWI_IPI_NO_07);
#else
    uint16_t coreMask = 1;
#if defined(OS_OPTION_SMP)
    OsHwiMcTrigger(0, coreMask, OS_HWI_IPI_NO_07);
#else
    OsHwiMcTrigger(coreMask, OS_HWI_IPI_NO_07);
#endif
#endif
    return 0;
}

void reset_vq(void)
{
    if (rvdev.svq != NULL) {
        /*
         * For svq:
         * vq_free_cnt: Set to vq_nentries, all descriptors in the svq are available.
         * vq_queued_cnt: Set to 0, no descriptors waiting to be processed in the svq.
         * vq_desc_head_idx: Set to 0, the next available descriptor is at the beginning
         *                   of the descriptor table.
         * vq_available_idx: Set to 0, No descriptors have been added to the available ring.
         * vq_used_cons_idx: No descriptors have been added to the used ring.
         * vq_ring.avail->idx and vq_ring.used->idx will be set at host.
         */
        rvdev.svq->vq_free_cnt = rvdev.svq->vq_nentries;
        rvdev.svq->vq_queued_cnt = 0;
        rvdev.svq->vq_desc_head_idx = 0;
        rvdev.svq->vq_available_idx = 0;
        rvdev.svq->vq_used_cons_idx = 0;
    }

    if (rvdev.rvq != NULL) {
        /*
         * For rvq:
         * Because host resets its tx vq, on the remote side,
         * it also needs to reset the rx rq.
         */
        rvdev.rvq->vq_available_idx = 0;
        rvdev.rvq->vq_used_cons_idx = 0;
        rvdev.rvq->vq_ring.used->idx = 0;
        rvdev.rvq->vq_ring.avail->idx = 0;
        metal_cache_flush(&(rvdev.rvq->vq_ring.used->idx),
                          sizeof(rvdev.rvq->vq_ring.used->idx));
        metal_cache_flush(&(rvdev.rvq->vq_ring.avail->idx),
                          sizeof(rvdev.rvq->vq_ring.avail->idx));
    }
}


struct virtio_device* platform_create_vdev(void *rsc_table, struct metal_io_region *rsc_io)
{
    struct fw_rsc_vdev_vring *vring_rsc;
    struct virtio_device *vdev;
    int ret;

    vdev = rproc_virtio_create_vdev(VIRTIO_DEV_DEVICE, VDEV_ID,
                    rsc_table_to_vdev(rsc_table),
                    rsc_io, NULL, virtio_notify, NULL);
    if (!vdev)
        return NULL;

    /* wait master rpmsg init completion */
    rproc_virtio_wait_remote_ready(vdev);

    vring_rsc = rsc_table_get_vring0(rsc_table);
    PRT_Printf("[openamp]: get vring0: da %lx\n", vring_rsc->da);

    ret = rproc_virtio_init_vring(vdev, 0, vring_rsc->notifyid,
                      (void *)(uintptr_t)vring_rsc->da, rsc_io,
                      vring_rsc->num, vring_rsc->align);
    if (ret)
        goto failed;

    vring_rsc = rsc_table_get_vring1(rsc_table);
    PRT_Printf("[openamp]: get vring1: da %lx\n", vring_rsc->da);

    ret = rproc_virtio_init_vring(vdev, 1, vring_rsc->notifyid,
                      (void *)(uintptr_t)vring_rsc->da, rsc_io,
                      vring_rsc->num, vring_rsc->align);
    if (ret)
        goto failed;

    return vdev;

failed:
    rproc_virtio_remove_vdev(vdev);
    return NULL;
}

/**
 * rpmsg_backend_init - register rpmsg-virtio-device.
 *
 * Init a rpmsg backend based on the the integration of
 * a resource table in the elf file.
 *
 * Return: pointer of rpmsg_device(rpdev) on success, NULL on failure.
 */
int create_rpmsg_device(void)
{
    void *rsc_table;
    struct metal_io_region *shm_io;
    int rsc_size;
    int err;
    struct metal_init_params metal_params = METAL_INIT_DEFAULTS;
    struct metal_device *device;

    /* Libmetal setup */
    err = metal_init(&metal_params);
    if (err) {
        PRT_Printf("[rpmsg] metal_init failed %d\n", err);
        return err;
    }

    err = metal_register_generic_device(&shm_device);
    if (err) {
        PRT_Printf("[rpmsg] Couldn't register shared memory device %d\n", err);
        goto cleanup_metal;
    }

    err = metal_device_open("generic", SHM_DEVICE_NAME, &device);
    if (err) {
        PRT_Printf("[rpmsg] metal_device_open failed %d\n", err);
        goto cleanup_metal;
    }

    metal_io_init(&device->regions[0], (void *)VDEV_START_ADDR, &shm_physmap[0],
              SHM_SIZE, -1, 0, NULL);

    shm_io = metal_device_io_region(device, 0);
    if (!shm_io) {
        PRT_Printf("[rpmsg] get shared memory io region failed %d\n", err);
        goto cleanup_metal;
    }

    rsc_table_get(&rsc_table, &rsc_size);
    PRT_Printf("[rpmsg] get rsctable %lx, size %lx \n", rsc_table, rsc_size);

    /* virtio device setup */
    vdev = platform_create_vdev(rsc_table, shm_io);
    if (!vdev) {
        PRT_Printf("[rpmsg] create virtio device failed %d\n", err);
        goto cleanup_metal;
    }

    /* setup rvdev */
    err = rpmsg_init_vdev_with_config(&rvdev, vdev, NULL, shm_io, NULL, RPMSG_VIRTIO_CONSOLE_CONFIG);
    if (err) {
        PRT_Printf("[rpmsg] rpmsg_init_vdev_with_config failed %d\n", err);
        goto cleanup_vdev;
    }

    rpdev = rpmsg_virtio_get_rpmsg_device(&rvdev);
    return 0;

cleanup_vdev:
    rproc_virtio_remove_vdev(vdev);
cleanup_metal:
    metal_finish();

    return -1;
}

void rpmsg_backend_remove(void)
{
    rpmsg_deinit_vdev(&rvdev);
    rproc_virtio_remove_vdev(vdev);
    rpdev = NULL;
    metal_finish();
    /* TODO: disable openamp ipi */
}

struct rpmsg_device *get_rpmsg_device()
{
    return rpdev;
}

void deal_rpmsg_msg()
{
    (void)rproc_virtio_notified(vdev, VRING1_ID);
}

