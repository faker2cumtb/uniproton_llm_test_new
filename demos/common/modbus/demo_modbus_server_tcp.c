/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-09-20     ShiAnhu      the first version
 */
#include <errno.h>
#include "print.h"
#include <string.h>
#include "lwip/etharp.h"
#include "lwip/sockets.h"
#include "lwip/tcpip.h"
#include "netif/ethernet.h"
#include <rtdevice.h>
#include <prt_task.h>
#include <prt_clk.h>
#include "modbus/modbus-tcp.h"

#define MAC0_IP4_ADDR "192.168.20.10"
#define MAC0_GATEWAY  "192.168.20.254"
#define MAC0_MASK     "255.255.255.0"

extern int usleep(unsigned sleep_time_in_us);

static struct netif test_netif0;
static const rt_uint8_t fxmac_hwaddr[ETH_HWADDR_LEN] = {0xff, 0x0e, 0x34, 0x00, 0x11, 0x55};
static rt_device_t xmac_dev;
static SemHandle xmac_tx_sem;
static SemHandle xmac_rx_sem;

static err_t default_send(struct netif *netif, struct pbuf *p)
{
    uint32_t bufferOffset = 0;
    uint32_t payloadLength = 0;
    struct pbuf *q;
    rt_size_t size;

    if (p->tot_len <= 0)
    {
        PRT_Printf("default_send err\n");
        return -1;
    }
    u8_t sendbuf[p->tot_len];

    LWIP_ASSERT("netif != NULL", (netif != NULL));
    LWIP_ASSERT("p != NULL", (p != NULL));

    for (q = p; q != NULL; q = q->next)
    {
        /* Get bytes in current lwIP buffer */
        payloadLength = q->len;

        /* copy data to Tx buffer */
        memcpy((u8_t*)(sendbuf + bufferOffset), (u8_t*)((u8_t*)q->payload), payloadLength);
        bufferOffset += payloadLength;
    }

    if (PRT_SemPend(xmac_tx_sem, OS_WAIT_FOREVER) == OS_OK)
    {
        size = rt_device_write(xmac_dev, 0, sendbuf, sizeof(sendbuf));
        if (size != sizeof(sendbuf))
        {
            PRT_Printf("xmac write err\n");
            return -1;
        }
    }

    return 0;
}
#define IFNAME0 's'
#define IFNAME1 't'

static void arp_timer(void *arg)
{
    etharp_tmr();
    sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);
}

/* 接收成功信号量 */
static rt_err_t xmac_rx_indicate(rt_device_t dev, rt_size_t size)
{
    PRT_SemPost(xmac_rx_sem);

    return RT_EOK;
}

/* 发送成功信号量 */
static rt_err_t xmac_tx_complete(rt_device_t dev, void *buffer)
{
    PRT_SemPost(xmac_tx_sem);

    return RT_EOK;
}

//飞腾派用了mac0，mac1，恒为e2000q板用了mac0,mac2
static void low_level_init(struct netif *netif)
{
    xmac_dev = rt_device_find("xmac0");
    if (xmac_dev == RT_NULL)
    {
        PRT_Printf("xmac device not found\n");
        return;
    }

    PRT_SemCreate(128, &xmac_tx_sem);
    PRT_SemCreate(0, &xmac_rx_sem);

    /* 设置接收通知回调 */
    rt_device_set_rx_indicate(xmac_dev, xmac_rx_indicate);

    /* 设置发送成功回调 */
    rt_device_set_tx_complete(xmac_dev, xmac_tx_complete);

    /* 设置mac地址 注意要在init之后，open之前设置*/
    rt_device_control(xmac_dev, RT_DEVICE_CTRL_NETIF_SETMAC, netif->hwaddr);

    static rt_uint8_t fxmacaddr[ETH_HWADDR_LEN] = {};
    rt_device_control(xmac_dev, RT_DEVICE_CTRL_NETIF_GETMAC, fxmacaddr);
    PRT_Printf("xmac mac addr is %02x:%02x:%02x:%02x:%02x:%02x\n", fxmacaddr[0], fxmacaddr[1],
               fxmacaddr[2], fxmacaddr[3], fxmacaddr[4], fxmacaddr[5]);

    /* 注意回调函数以及mac地址要在open之前设置好 */
    if (rt_device_open(xmac_dev, RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        PRT_Printf("xmac device open failed\n");
        return;
    }
}

static err_t ethernetif_init(struct netif *netif)
{
    LWIP_ASSERT("netif != NULL", (netif != NULL));

#if LWIP_NETIF_HOSTNAME
    /* Initialize interface hostname */
    netif->hostname = "lwip";
#endif

    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;
#if LWIP_IPV4 && LWIP_IPV6
#elif LWIP_IPV6
    netif->output_ip6 = ethip6_output;
#else
    netif->output = etharp_output;
#endif

    netif->linkoutput = default_send;
    /* set MAC hardware address length */
    netif->hwaddr_len = ETH_HWADDR_LEN;
    /* maximum transfer uint */
    netif->mtu = 1500;

    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

    memcpy(netif->hwaddr, fxmac_hwaddr, ETH_HWADDR_LEN);

    etharp_init();
    sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);
    low_level_init(netif);
    return ERR_OK;
}

static void ethernetif_input(struct netif *netif1)
{
    struct pbuf *q;
    rt_uint8_t rx_buf[1500];
    rt_size_t size;
    err_t err;

    /* move received packet into a new pbuf */
    do
    {
        SYS_ARCH_DECL_PROTECT(sr);

        SYS_ARCH_PROTECT(sr);
        size = rt_device_read(xmac_dev, 0, rx_buf, sizeof(rx_buf));
        SYS_ARCH_UNPROTECT(sr);

        if (size <= 0)
        {
            return;
        }
        /* netif_add func now use tcpip_input */
        q = pbuf_alloc(PBUF_RAW, size, PBUF_RAM);
        memcpy(q->payload, rx_buf, size);
        err = tcpip_input(q, netif1);
        if (err != ERR_OK)
        {
            LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input : IP input error\n"));
        }
    } while (1);
}

static void EthThread(void *arg)
{
    while (1)
    {
        if (PRT_SemPend(xmac_rx_sem, OS_WAIT_FOREVER) == OS_OK) // wait recv sem forever
        {
            ethernetif_input(&test_netif0);
        }
    }
}

static void Net_Lwip_Init(void)
{
    ip4_addr_t test_gw1, test_ipaddr1, test_netmask1;

    // 协议栈初始化
    tcpip_init(NULL, NULL);

    if (inet_aton(MAC0_IP4_ADDR, &test_ipaddr1) == 0)
    {
        PRT_Printf("The addr of ipaddr is wrong\r\n");
    }
    if (inet_aton(MAC0_GATEWAY, &test_gw1) == 0)
    {
        PRT_Printf("The addr of gw is wrong\r\n");
    }
    if (inet_aton(MAC0_MASK, &test_netmask1) == 0)
    {
        PRT_Printf("The addr of netmask is wrong\r\n");
    }

    // 设置netif的ip，gateway，mask
    netif_add(&test_netif0, &test_ipaddr1, &test_netmask1,
              &test_gw1, NULL, &ethernetif_init, &tcpip_input);

    netif_set_default(&test_netif0);
    netif_set_up(&test_netif0);
    netif_set_link_up(&test_netif0);
    sys_thread_new((char*)"Eth_if", EthThread, &test_netif0, 0x1000, 2);
}

int Modbus_Server_TCP_Demo()
{
    const char *ip_or_device = MAC0_IP4_ADDR;
    const int srv_port = 1502;
    const bool debug_flag = false;
    const int sleep_time_us = debug_flag ? 500 * 1000 : 1 * 1000;

    PRT_Printf("Hello world! - IDE: Visual Studio Code, OS: UniProton, "
               "Demo: Modbus Server TCP, Built: %s %s\n", __DATE__, __TIME__);
    PRT_Printf("Local IP: %s, Server IP: %s, Server Port: %d\n",
               MAC0_IP4_ADDR, ip_or_device, srv_port);

    Net_Lwip_Init(); // init LWIP and MAC

    modbus_t *ctx = modbus_new_tcp(ip_or_device, srv_port);
    if (ctx == NULL)
    {
        PRT_Printf("ERROR: Unable to allocate libmodbus context\n");
        return -1;
    }

    if (modbus_set_debug(ctx, debug_flag) == -1)
    {
        PRT_Printf("ERROR: Set debug failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    modbus_mapping_t *map = modbus_mapping_new(500, 500, 500, 500);
    if (map == NULL)
    {
        PRT_Printf("ERROR: Mapping failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    const int num_reg = 10;

    for (int i = 0; i < num_reg; i++)
    {
        map->tab_registers[i] = i;
    }

    int sockfd = modbus_tcp_listen(ctx, 1);
    if (sockfd == -1)
    {
        PRT_Printf("ERROR: Listen failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    if (modbus_tcp_accept(ctx, &sockfd) == -1)
    {
        PRT_Printf("ERROR: Accept failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];

    int req_length = 0;
    int data_index = 0;

    while (1)
    {
        PRT_Printf("---------------- [MODBUS SERVER TCP %03d] ----------------\n", ++data_index);

        memset(query, 0, sizeof(query));

        for (int i = 0; i < num_reg; i++)
        {
            map->tab_registers[i]++;
        }

        do
        {
            req_length = modbus_receive(ctx, query);
            if ((req_length != -1) || ((req_length == -1) && (errno != ETIMEDOUT)))
            {
                break;
            }

            usleep(sleep_time_us);
        } while ((req_length == 0) || ((req_length == -1) && (errno == ETIMEDOUT)));

        if ((req_length == -1) && (errno != EMBBADCRC))
        {
            modbus_close(ctx);
            if (modbus_tcp_accept(ctx, &sockfd) == -1)
            {
                PRT_Printf("ERROR: Accept failed: %s\n", modbus_strerror(errno));
                modbus_free(ctx);
                return -1;
            }
        }
        else
        {
            uint16_t *tab_reg = map->tab_registers;
            PRT_Printf("SEND REG[%d]<dec(hex)>: "
                       "%02d(0x%02X) %02d(0x%02X) %02d(0x%02X) %02d(0x%02X) %02d(0x%02X) "
                       "%02d(0x%02X) %02d(0x%02X) %02d(0x%02X) %02d(0x%02X) %02d(0x%02X)\n",
                       num_reg,
                       tab_reg[0], tab_reg[0], tab_reg[1], tab_reg[1],
                       tab_reg[2], tab_reg[2], tab_reg[3], tab_reg[3],
                       tab_reg[4], tab_reg[4], tab_reg[5], tab_reg[5],
                       tab_reg[6], tab_reg[6], tab_reg[7], tab_reg[7],
                       tab_reg[8], tab_reg[8], tab_reg[9], tab_reg[9]);
            int rc = modbus_reply(ctx, query, req_length, map);
            if (rc == -1)
            {
                PRT_Printf("ERROR: Reply failed: %s\n", modbus_strerror(errno));
                break;
            }
        }
    }

    PRT_Printf("ERROR: Quit the loop: %s\n", modbus_strerror(errno));

    modbus_mapping_free(map);

    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
