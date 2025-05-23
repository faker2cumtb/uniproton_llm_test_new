#include "prt_sem.h"
#include "print.h"
#include "prt_cache_external.h"
#include "openamp/rpmsg.h"

static uint8_t* g_buffer_addr = NULL;
static uint8_t* g_data_addr = NULL;

static uint32_t g_data_len = 0;
static uint32_t g_left_len = 0;

static SemHandle mutex;
SemHandle recv_powerdown_sem;

static struct rpmsg_endpoint powerdown_prot_ept;

static void (*g_powerdown_prot_ipi_handler)(void);

extern struct rpmsg_device *rpdev;
extern uintptr_t OsGetPowerDownProtBuffer();

#define RPMSG_POWERDOWN_PROT_EPT_NAME    "rpmsg-powerdown_protect"

static void sync_powerdown_prot_data(void);
static void *powerdown_prot_listen_task(void *arg)
{
    while(1)
    {
        if(PRT_SemPend(recv_powerdown_sem, OS_WAIT_FOREVER) == OS_OK)
            break;
    }
    if(g_powerdown_prot_ipi_handler)
        g_powerdown_prot_ipi_handler();

    sync_powerdown_prot_data();
    
    return NULL;
}

void notify_powerdown_msg(void)
{
    PRT_SemPost(recv_powerdown_sem);
}

uint32_t powerdown_prot_service_init(void)
{
    U32 ret = 0;
    g_data_len = 0;
    g_data_addr = NULL;
    pthread_attr_t attr;
    pthread_t listen_thread;

    ret = PRT_SemMutexCreate(&mutex);
    if(ret != OS_OK)
    {
        PRT_Printf("PRT_SemMutexCreate failed. ret = %d\n", ret);
        goto err;
    }

    ret = PRT_SemCreate(0, &recv_powerdown_sem);
    if(ret != OS_OK)
    {
        PRT_Printf("PRT_SemCreate recv_powerdown_sem failed. ret = %d\n", ret);
        goto err;
    }

    g_buffer_addr = OsGetPowerDownProtBuffer();
    if(g_buffer_addr == NULL)
    {
        PRT_Printf("OsGetPowerOffProtBuffer failed.\n");
        ret = -1;
        goto err;
    }

    g_data_addr = g_buffer_addr + sizeof(uint32_t);
    g_left_len = 64 * 1024;
    g_data_len = 0;

    if (pthread_attr_init(&attr) != 0) {
        PRT_Printf("poweroff_prot failed to init pthread_attr\n");
        goto err;
    }

    if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
        pthread_attr_destroy(&attr);
        PRT_Printf("poweroff_prot failed to set pthread_attr\n");
        goto err;
    }

    ret = pthread_create(&listen_thread, &attr, powerdown_prot_listen_task, NULL);
    pthread_attr_destroy(&attr);
    if (ret) {
        /* If no rpmsg tasks, release the backend. */
        PRT_Printf("create poweroff_prot_listen_task task fail, %d\n", ret);
        goto err;
    }

    return OS_OK;
err:
    PRT_SemDelete(mutex);
    PRT_SemDelete(recv_powerdown_sem);
    return ret;
}

void set_powerdown_prot_ipi_handler(void (*handler)(void))
{
    g_powerdown_prot_ipi_handler = handler;
}

uint32_t write_powerdown_prot_data(uint8_t *data, uint32_t len)
{
    uint32_t min_len = 0;

    PRT_SemPend(mutex, OS_WAIT_FOREVER);
    if(g_left_len == 0)
    {
        PRT_Printf("poweroff protect buffer no empty space\n");
        return 0;
    }

    min_len = len > g_left_len ? g_left_len : len;
    memcpy(g_data_addr, data, min_len);
    g_data_addr += min_len;
    g_data_len += min_len;
    g_left_len -= min_len;

    PRT_SemPost(mutex);

    return min_len;
}

int powerdown_prot_client_cb(struct rpmsg_endpoint *ept,
                    void *data, size_t len,
                    uint32_t src, void *priv)
{
    return RPMSG_SUCCESS;
}


static void sync_powerdown_prot_data(void)
{
    if(g_buffer_addr == NULL)
    {
        PRT_Printf("poweroff protect service not init.\n");
        return;
    }

    *(uint32_t *)g_buffer_addr = g_data_len;
    cache_flush_by_range(g_buffer_addr, sizeof(uint32_t) + g_data_len);

    rpmsg_create_ept(&powerdown_prot_ept, rpdev, RPMSG_POWERDOWN_PROT_EPT_NAME,
                   RPMSG_ADDR_ANY, RPMSG_ADDR_ANY, powerdown_prot_client_cb, NULL);

    PRT_SemDelete(mutex);
    PRT_SemDelete(recv_powerdown_sem);
}