#include "prt_typedef.h"
#include "pthread.h"
#include "prt_sem.h"
#include "openamp/rpmsg.h"

extern struct rpmsg_device *get_rpmsg_device();

#define RPMSG_TTY_EPT_NAME "rpmsg-tty"
static char *g_s1 = "Hello, UniProton! \r\n";

struct rpmsg_rcv_msg {
    void *data;
    size_t len;
};

static SemHandle tty_sem;
static struct rpmsg_endpoint tty_ept;
static struct rpmsg_rcv_msg tty_msg;
static volatile int tty_exit = 0;
static pthread_t tty_thread = NULL;

#if defined(OS_OPTION_SMP)
static struct PrtSpinLock g_ttyLock;
#endif

static int rpmsg_rx_tty_callback(struct rpmsg_endpoint *ept, void *data,
                   size_t len, uint32_t src, void *priv)
{
    struct rpmsg_rcv_msg *tty_msg = priv;

    rpmsg_hold_rx_buffer(ept, data);
    tty_msg->data = data;
    tty_msg->len = len;
    PRT_SemPost(tty_sem);

    return 0;
}

int send_message(unsigned char *message, int len)
{
    int ret;

    if (!is_rpmsg_ept_ready(&tty_ept)) {
        return 0;
    }
#if defined(OS_OPTION_SMP)
    uintptr_t intSave = PRT_SplIrqLock(&g_ttyLock);
#endif
    ret = rpmsg_send(&tty_ept, message, len);
#if defined(OS_OPTION_SMP)
    PRT_SplIrqUnlock(&g_ttyLock, intSave);
#endif
    return ret;
}

static void *rpmsg_tty_task(void *arg)
{
    int ret;
#ifndef LOSCFG_SHELL_MICA_INPUT
    char tx_buff[512];
#endif
    char *tty_data;
    struct rpmsg_device * rpdev;

    rpdev = get_rpmsg_device();
    if(!rpdev)
    {
        PRT_Printf("[tty] get_rpmsg_device NULL\n");
        goto err;
    }

    PRT_Printf("[openamp] tty task started\n");

    tty_ept.priv = &tty_msg;
    ret = rpmsg_create_ept(&tty_ept, rpdev, RPMSG_TTY_EPT_NAME,
                   RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
                   rpmsg_rx_tty_callback, NULL);
    if (ret != 0) {
        PRT_Printf("[tty] endpoint ret:%d \n", ret);
        goto err;
    }

    while (!is_rpmsg_ept_ready(&tty_ept)) {
        PRT_TaskDelay(100);
    }

    tty_exit = 0;
    while (!tty_exit) {
        PRT_SemPend(tty_sem, OS_WAIT_FOREVER);
        if (tty_msg.len) {
            tty_data = (char *)tty_msg.data;
            tty_data[tty_msg.len] = '\0';
            #ifdef LOSCFG_SHELL_MICA_INPUT
                ShellCB *shellCb = OsGetShellCB();
                if (shellCb == NULL) {
                    send_message((void *)g_s1, strlen(g_s1) * sizeof(char));
                } else {
                    for(int i = 0; i < tty_msg.len; i++){
                        char c = tty_data[i];
                        ShellCmdLineParse(c, (pf_OUTPUT)printf, shellCb);
                    }
                }
            #else
                ret = snprintf(tx_buff, 512, "Hello, UniProton! Recv: %s\r\n", tty_data);
                rpmsg_send(&tty_ept, tx_buff, ret);
            #endif
                rpmsg_release_rx_buffer(&tty_ept, tty_msg.data);

        }
        tty_msg.len = 0;
        tty_msg.data = NULL;

        /* TODO: add lifecycle */
    }
    rpmsg_destroy_ept(&tty_ept);
err:
    pthread_exit(NULL);
}

int create_tty_ept()
{
    int ret;

    ret = PRT_SemCreate(0, &tty_sem);
    if (ret != OS_OK) {
        PRT_Printf("[tty] failed to create tty sem\n");
        return OS_ERROR;
    }

#if defined(OS_OPTION_SMP)
    ret = PRT_SplLockInit(&g_ttyLock);
    if (ret) {
        PRT_SemDelete(tty_sem);
        PRT_Printf("[tty] spin lock init fail\n");
        return OS_ERROR;
    }
#endif

    ret = pthread_create(&tty_thread, NULL, rpmsg_tty_task, NULL);
    if(ret != 0)
    {
        PRT_Printf("[tty] create rpmsg_tty_task fail, %d\n", ret);
        goto final;
    }

    return 0;

final:
    PRT_SemDelete(tty_sem);

    return -1;
}

void remove_tty_ept()
{
    if(tty_thread)
    {
        tty_exit = 1;
        pthread_join(tty_thread, NULL);
    }
    
    PRT_SemDelete(tty_sem);
    tty_thread = NULL;
}