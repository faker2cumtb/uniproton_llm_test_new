#include <netdb.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <net/if.h>
#include <sys/uio.h>
#include <lwip/ip_addr.h>
#include <lwip/sockets.h>
#include <lwip/etharp.h>
#include <lwip/tcpip.h>
#include <rtdevice.h>
#include <prt_task.h>
#include <prt_clk.h>
#include <prt_buildef.h>
#include <demo.h>

#define MAC_PORT           "xmac3"
#define MAC_ADDR           "192.168.10.90" 
#define MAC_GATEWAY        "192.168.10.1"
#define MAC_MASK           "255.255.255.0"
#define PORTNUM             5002
#define MAX_CLIENTS         5

static rt_device_t xmac_dev;
static struct netif test_netif0;
static rt_uint8_t fxmac_hwaddr[ETH_HWADDR_LEN] = {0xff, 0x0e, 0x34, 0x00, 0x11, 0x55};
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
        memcpy((u8_t *)(sendbuf + bufferOffset), (u8_t *)((u8_t *)q->payload), payloadLength);
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

void low_level_init(struct netif *netif)
{
    xmac_dev = rt_device_find(MAC_PORT);
    if (xmac_dev == RT_NULL)
    {
        PRT_Printf("find device %s failed\n", MAC_PORT);
        return;
    }

    PRT_Printf("find device %s success\n", MAC_PORT);

    PRT_SemCreate(128, &xmac_tx_sem);
    PRT_SemCreate(0, &xmac_rx_sem);

    /* 设置接收通知回调 */
    rt_device_set_rx_indicate(xmac_dev, xmac_rx_indicate);

    /* 设置发送成功回调 */
    rt_device_set_tx_complete(xmac_dev, xmac_tx_complete);

    /* 设置mac地址 注意要在init之后，open之前设置 */
    rt_device_control(xmac_dev, RT_DEVICE_CTRL_NETIF_SETMAC, netif->hwaddr);

    static rt_uint8_t fxmacaddr[ETH_HWADDR_LEN] = {};
    rt_device_control(xmac_dev, RT_DEVICE_CTRL_NETIF_GETMAC, fxmacaddr);
    PRT_Printf("xmac mac addr is %02x:%02x:%02x:%02x:%02x:%02x\n", fxmacaddr[0], fxmacaddr[1], fxmacaddr[2],
        fxmacaddr[3], fxmacaddr[4], fxmacaddr[5]);

    /* 注意回调函数以及mac地址要在open之前设置好 */
    if (rt_device_open(xmac_dev, RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        PRT_Printf("xmac device open failed\n");
        return;
    }

    netif->state = (void *)xmac_dev;
}

err_t ethernetif_init(struct netif *netif)
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

void ethernetif_input(struct netif *netif)
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
        size = rt_device_read((rt_device_t)(netif->state), 0, rx_buf, sizeof(rx_buf));
        SYS_ARCH_UNPROTECT(sr);

        if (size <= 0)
        {
            return;
        }
        /* netif_add func now use tcpip_input */
        q = pbuf_alloc(PBUF_RAW, size, PBUF_RAM);
        memcpy(q->payload, rx_buf, size);
        err = tcpip_input(q, netif);
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
        if (PRT_SemPend(xmac_rx_sem, OS_WAIT_FOREVER) == OS_OK)
        { // wait recv sem forever
            ethernetif_input(&test_netif0);
        }
    }
}

void link_detect_thread(void *p)
{
    RT_UNUSED(p);
    struct netif *netif = &test_netif0;
    RT_ASSERT(netif->state != NULL);
    int link_status = 0;


    while (1)
    {
        /* detect link status every second */
        PRT_TaskDelay(1000);
        rt_device_control((rt_device_t)netif->state, RT_DEVICE_CTRL_NETIF_LINKSTATE, &link_status);
        if (link_status)
        {
            if (netif_is_link_up(netif) == 0)
            {
                PRT_Printf("link up\n");
                netif_set_link_up(netif);
            }
        } else {
            if (netif_is_link_up(netif) == 1)
            {
                PRT_Printf("link down\n");
                netif_set_link_down(netif);
            }
        }
    }
}

void Net_Lwip_Init(void)
{
    ip4_addr_t test_gw1, test_ipaddr1, test_netmask1;

    // 协议栈初始化
    tcpip_init(NULL, NULL);

    if (inet_aton(MAC_ADDR, &test_ipaddr1) == 0)
    {
        PRT_Printf("The addr of ipaddr is wrong\r\n");
    }
    if (inet_aton(MAC_GATEWAY, &test_gw1) == 0)
    {
        PRT_Printf("The addr of gw is wrong\r\n");
    }
    if (inet_aton(MAC_MASK, &test_netmask1) == 0)
    {
        PRT_Printf("The addr of netmask is wrong\r\n");
    }

    // 设置netif的ip，gateway，mask
    netif_add(&test_netif0, &test_ipaddr1, &test_netmask1, &test_gw1, NULL, &ethernetif_init, &tcpip_input);

    netif_set_default(&test_netif0);
    netif_set_up(&test_netif0);
    netif_set_link_up(&test_netif0);
    sys_thread_new((char *)"Eth_if", EthThread, &test_netif0, 0x1000, 2);
    sys_thread_new((char *)"Eth_if", link_detect_thread, &test_netif0, 0x1000, 2);
}

void Lwip_Udp_Demo(void)
{
    Net_Lwip_Init(); // init lwip and mac
    PRT_Printf("Udp Client Demo\n");
    int socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server_addr;
    char pkt[] = "hello e2000q lwip";
    ip4_addr_t serverip;
    /* server init */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORTNUM);
    IP4_ADDR(&serverip, 192, 168, 10, 100); // c0 a8 1 63
    server_addr.sin_addr.s_addr = serverip.addr;
    memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    // 如果存在代理网络，需要绑定lwip网口
	#ifdef OS_OPTION_PROXY
    const char *device_name = "ethxxxyyy";
	int ret = setsockopt(socketfd, SOL_SOCKET, SO_BINDTODEVICE, device_name, strlen(device_name) + 1);
    #endif

    while (1)
    {
        sendto(socketfd, pkt, sizeof(pkt), 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
        PRT_TaskDelay(1000); // send msg every seconds
    }
}


int Lwip_Tcp_Demo(void)
{
    Net_Lwip_Init(); // init lwip and mac
    PRT_Printf("Tcp Client Demo ip %s:%d\n", MAC_ADDR, PORTNUM);
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[1024] = {0};
    fd_set fds;
    char pkt[] = "hello e2000q lwip tcp";

    int so_keepalive_val = 1;
    int tcp_keepalive_idle = 1;
    int tcp_keepalive_intvl = 1;
    int tcp_keepalive_cnt = 1;
    int tcp_nodelay = 1;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        PRT_Printf("Socket creation failed\n");
        return -1;
    }

    // 如果存在代理网络，需要绑定lwip网口
	#ifdef OS_OPTION_PROXY
	const char *device_name = "ethxxxyyy";
	int ret = setsockopt(server_socket, SOL_SOCKET, SO_BINDTODEVICE, device_name, strlen(device_name) + 1);
    #endif

    /* server init */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORTNUM);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        PRT_Printf("Binding failed\n");
        close(server_socket);
        return -1;
    }

    if (listen(server_socket, MAX_CLIENTS) < 0)
    {
        PRT_Printf("Listen failed\n");
        close(server_socket);
        return -1;
    }

    while (1)
    {
        FD_ZERO(&fds);
        FD_SET(server_socket, &fds);
        if (select(MAX_CLIENTS + 1, &fds, NULL, NULL, NULL) < 0)
        {
            PRT_Printf("Select failed\n");
            close(server_socket);
            return -1;
        }

        if (FD_ISSET(server_socket, &fds))
        {
            client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
            if (client_socket < 0)
            {
                PRT_Printf("Accepting client failed\n");
                close(server_socket);
                return -1;
            }
            else
            {
                // 使能socket层的心跳检测
                setsockopt(client_socket, SOL_SOCKET, SO_KEEPALIVE, &so_keepalive_val, sizeof(int));
                // 配置心跳检测参数   默认参数时间很长
                setsockopt(client_socket, IPPROTO_TCP, TCP_KEEPIDLE, &tcp_keepalive_idle, sizeof(int));
                setsockopt(client_socket, IPPROTO_TCP, TCP_KEEPINTVL, &tcp_keepalive_intvl, sizeof(int));
                setsockopt(client_socket, IPPROTO_TCP, TCP_KEEPCNT, &tcp_keepalive_cnt, sizeof(int));
                setsockopt(client_socket, IPPROTO_TCP, TCP_NODELAY, &tcp_nodelay, sizeof(int));
                PRT_Printf("Client connected from %s\n", inet_ntoa(client_addr.sin_addr));
                int len = recv(client_socket, buffer, sizeof(buffer), 0);
                while (len > 0)
                {
                    buffer[len] = '\0';
                    PRT_Printf("read len %d : %s \n", len, buffer);
                    if (send(client_socket, pkt, strlen(pkt), 0) < 0)
                    {
                        PRT_Printf("Sending data failed\n");
                        len = 0;
                    }
                    len = recv(client_socket, buffer, sizeof(buffer), 0);
                }
                PRT_Printf("Client Disconnected\n");
                close(client_socket);
            }
        }
    }

    close(server_socket);
    return 0;
}
