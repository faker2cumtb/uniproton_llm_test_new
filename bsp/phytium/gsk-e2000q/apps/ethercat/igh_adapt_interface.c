#include "prt_typedef.h"
#include <linux/netdevice.h>
#include "print.h"
#include <prt_clk.h>
#include "prt_sem.h"
#include <rtdevice.h>

static int gmacInitOk = 0;
static rt_device_t xmac_dev;
static SemHandle xmac_tx_sem;

static rt_err_t xmac_tx_complete(rt_device_t dev, void *buffer)
{
    PRT_SemPost(xmac_tx_sem);

    return RT_EOK;
}

void igh_nic_init()
{
    xmac_dev = rt_device_find("xmac3");
    if (xmac_dev == RT_NULL)
    {
        PRT_Printf("xmac device not found\n");
        return -1;
    }

    PRT_SemCreate(16, &xmac_tx_sem);

    /* 设置发送成功回调 */
    rt_device_set_tx_complete(xmac_dev, xmac_tx_complete);

    if (rt_device_open(xmac_dev, RT_DEVICE_FLAG_RDWR) != RT_EOK) {
        PRT_Printf("xmac device open failed with \n");
        return -1;
    }

    gmacInitOk = 1;
}

int igh_nic_get_mac_address(unsigned char *buf, int buf_len)
{
    buf[0] = 0x00;
    buf[1] = 0x00;
    buf[2] = 0x00;
    buf[3] = 0x00;
    buf[4] = 0x20;
    buf[5] = 0x00;

    return 0;
}
int igh_packet_send(unsigned char *packet, int length)
{
    size_t size;
    if (PRT_SemPend(xmac_tx_sem, OS_WAIT_FOREVER) == OS_OK)
    {
        size = rt_device_write(xmac_dev, 0, packet, length);
        return size;
    }

    return -1;
}

int igh_packet_recv(unsigned char *packet, int size)
{
    size_t recv_len;
    recv_len = rt_device_read(xmac_dev, 0, packet, size);
    if(recv_len > 0)
    {
        return recv_len;
    }
    return -1;
}

bool igh_nic_carrier_ok(const struct net_device *dev)
{
    return gmacInitOk;
}

U64 get_cycles()
{
    return PRT_ClkGetCycleCount64();
}
