#include "prt_typedef.h"
#include "pthread.h"

#include "openamp/rpmsg.h"
#include "prt_proxy_ext.h"

#define RPMSG_RPC_EPT_NAME "rpmsg-rpc"

extern char *g_printf_buffer;
extern struct rpmsg_device *get_rpmsg_device();
extern int rpmsg_client_cb(struct rpmsg_endpoint *ept,
               void *data, size_t len,
               uint32_t src, void *priv);
extern void rpmsg_set_default_ept(struct rpmsg_endpoint *ept);

static struct rpmsg_endpoint rpc_ept;

static void rpmsg_service_unbind(struct rpmsg_endpoint *ep)
{
    rpmsg_destroy_ept(ep);
}

static void *rpmsg_rpc_task(void *arg)
{
    int ret;
    struct rpmsg_device * rpdev;

    rpdev = get_rpmsg_device();
    if(!rpdev)
    {
        PRT_Printf("[rpc] get_rpmsg_device NULL\n");
        goto err;
    }
    ret = rpmsg_create_ept(&rpc_ept, rpdev, RPMSG_RPC_EPT_NAME,
                   RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
                   rpmsg_client_cb, rpmsg_service_unbind);
    if (ret != 0) {
        PRT_Printf("[openamp] rpc endpoint ret:%d \n", ret);
        goto err;
    }

err:
    pthread_exit(NULL);
}

int create_rpc_ept()
{
    int ret = 0;
    pthread_attr_t attr;
    pthread_t rpc_thread;

    if (pthread_attr_init(&attr) != 0) {
        PRT_Printf("[proxy] failed to init pthread_attr\n");
        return OS_ERROR;
    }

    if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
        PRT_Printf("[proxy] failed to set pthread_attr\n");
        goto final;
    }

    /* create rpmsg task */
    ret = pthread_create(&rpc_thread, &attr, rpmsg_rpc_task, NULL);
    if(ret != 0)
    {
        PRT_Printf("[proxy] create task fail, %d\n", ret);
        goto final;
    }

    while (!is_rpmsg_ept_ready(&rpc_ept)) {
        PRT_TaskDelay(100);
    }
    PRT_Printf("[proxy] ept ready\n");

    rpmsg_set_default_ept(&rpc_ept);
    g_printf_buffer = (char *)malloc(PRINTF_BUFFER_LEN);

final:
    pthread_attr_destroy(&attr);
    return ret;
}

void remove_rpc_ept()
{
    rpmsg_destroy_ept(&rpc_ept);
    if(g_printf_buffer)
    {
        free(g_printf_buffer);
    }
}