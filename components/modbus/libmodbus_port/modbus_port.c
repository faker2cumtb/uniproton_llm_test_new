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
#include <netdb.h>
#include <stddef.h>
#include <stdlib.h>
#include <syscall.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <termios.h>

#include "print.h"
#include "prt_sem.h"
#include "prt_task.h"
#include "prt_tick.h"
#include "rtdevice.h"
#include "lwip/sockets.h"

#include "modbus_port.h"

#undef ioctl
#define ioctl(fd, req, ...)                            MB_ioctl(fd, req, ##__VA_ARGS__)

#undef cfsetospeed
#define cfsetospeed(tio, speed)                        MB_cfsetospeed(tio, speed)

static int g_uart_dev_no = -1;
static rt_device_t g_uart_dev_p = RT_NULL;
static SemHandle g_uart_sem;
struct termios g_uart_tios;

#define ASSERT_DEV_IS_NOT_SET() do { \
        if (!((g_uart_dev_no == -1) && (g_uart_dev_p == RT_NULL))) { \
            PRT_Printf("%s():%d: Assertion failed: Device is already set (%d, %p)\n", \
                       __FUNCTION__, __LINE__, g_uart_dev_no, g_uart_dev_p); \
            errno = EINVAL; \
            return -1; \
        } \
    } while (0)
#define ASSERT_DEV_IS_SET(fd) do { \
        if (!(g_uart_dev_no == fd) && (g_uart_dev_p != RT_NULL)) { \
            PRT_Printf("%s():%d: Assertion failed: Device is NOT set properly (%d, %p, %d)\n", \
                       __FUNCTION__, __LINE__, g_uart_dev_no, g_uart_dev_p, fd); \
            errno = EINVAL; \
            return -1; \
        } \
    } while (0)

static rt_uint32_t get_uart_baudrate(speed_t speed, bool debug)
{
    rt_uint32_t baudRate = BAUD_RATE_9600;

    switch (speed)
    {
        /*
        case B110:
            baudRate = BAUD_RATE_110;
            break;
        case B300:
            baudRate = BAUD_RATE_300;
            break;
        case B600:
            baudRate = BAUD_RATE_600;
            break;
        case B1200:
            baudRate = BAUD_RATE_1200;
            break;
        */
        case B2400:
            baudRate = BAUD_RATE_2400;
            break;
        case B4800:
            baudRate = BAUD_RATE_4800;
            break;
        case B9600:
            baudRate = BAUD_RATE_9600;
            break;
        case B19200:
            baudRate = BAUD_RATE_19200;
            break;
        case B38400:
            baudRate = BAUD_RATE_38400;
            break;
#ifdef B57600
        case B57600:
            baudRate = BAUD_RATE_57600;
            break;
#endif
#ifdef B115200
        case B115200:
            baudRate = BAUD_RATE_115200;
            break;
#endif
#ifdef B230400
        case B230400:
            baudRate = BAUD_RATE_230400;
            break;
#endif
#ifdef B460800
        case B460800:
            baudRate = BAUD_RATE_460800;
            break;
#endif
#ifdef B500000
            /*
            case B500000:
                baudRate = BAUD_RATE_500000;
                break;
            */
#endif
#ifdef B576000
            /*
            case B576000:
                baudRate = BAUD_RATE_576000;
                break;
            */
#endif
#ifdef B921600
        case B921600:
            baudRate = BAUD_RATE_921600;
            break;
#endif
#ifdef B1000000
            /*
            case B1000000:
                baudRate = BAUD_RATE_1000000;
                break;
            */
#endif
#ifdef B1152000
            /*
            case B1152000:
                baudRate = BAUD_RATE_1152000;
                break;
            */
#endif
#ifdef B1500000
            /*
            case B1500000:
                baudRate = BAUD_RATE_1500000;
                break;
            */
#endif
#ifdef B2000000
        case B2000000:
            baudRate = BAUD_RATE_2000000;
            break;
#endif
#ifdef B2500000
        case B2500000:
            baudRate = BAUD_RATE_2500000;
            break;
#endif
#ifdef B3000000
        case B3000000:
            baudRate = BAUD_RATE_3000000;
            break;
#endif
#ifdef B3500000
            /*
            case B3500000:
                baudRate = BAUD_RATE_3500000;
                break;
            */
#endif
#ifdef B4000000
            /*
            case B4000000:
                baudRate = BAUD_RATE_4000000;
                break;
            */
#endif
        default:
            baudRate = BAUD_RATE_9600;
            if (debug)
            {
                PRT_Printf("WARNING: Unsupported speed (0x%x), 9600 is used\n", speed);
            }
    }

    return baudRate;
}

static rt_uint32_t get_uart_data_bits(int data_bit, bool debug)
{
    rt_uint32_t dataBit = DATA_BITS_8;

    switch (data_bit)
    {
        case CS5:
            dataBit = DATA_BITS_5;
            break;
        case CS6:
            dataBit = DATA_BITS_6;
            break;
        case CS7:
            dataBit = DATA_BITS_7;
            break;
        case CS8:
            dataBit = DATA_BITS_8;
            break;
        default:
            dataBit = DATA_BITS_8;
            if (debug)
            {
                PRT_Printf("WARNING: Unsupported data_bit (%d), 8B is used\n", data_bit);
            }
    }

    return dataBit;
}

static rt_uint32_t get_uart_stop_bits(int stop_bit, bool debug)
{
    RT_UNUSED(debug);
    rt_uint32_t stopBit = DATA_BITS_8;

    if (stop_bit == 0)
    {
        stopBit = STOP_BITS_1;
    }
    else
    {
        stopBit = STOP_BITS_2;
    }

    return stopBit;
}

static rt_uint32_t get_uart_parity(int parity_enable, int parity_odd, bool debug)
{
    RT_UNUSED(debug);
    rt_uint32_t parity = PARITY_NONE;

    if (parity_enable == 0)
    {
        // None
        parity = PARITY_NONE;
    }
    else if ((parity_enable == 1) && (parity_odd == 0))
    {
        // Even
        parity = PARITY_EVEN;
    }
    else
    {
        // Odd
        parity = PARITY_ODD;
    }

    return parity;
}

static rt_err_t uart_rx_indicate(rt_device_t dev, rt_size_t size)
{
    U32 prt_rc = PRT_SemPost(g_uart_sem);
    if (prt_rc != OS_OK)
    {
        return RT_ERROR;
    }

    return RT_EOK;
}

int MB_ioctl(int fd, int req, ...)
{
    void *arg;
    va_list ap;
    va_start(ap, req);
    arg = va_arg(ap, void*);
    va_end(ap);

    if (req == TCGETS)
    {
        *((struct termios*)arg) = g_uart_tios;
        return 0;
    }
    else if (req == TCFLSH)
    {
        PRT_Printf("ERROR: Unsupported req(0x%08X)\n", req);
        errno = ENOTSUP;
        return -1;
    }
    else if ((req >= TCSETS) && (req < TCFLSH))
    {
        const int act = req - TCSETS;
        if (act != TCSANOW)
        {
            PRT_Printf("ERROR: Unsupported act(%d)\n", act);
            errno = ENOTSUP;
            return -1;
        }

        ASSERT_DEV_IS_SET(fd);

        struct termios *tio = arg;
        if (tio->c_cflag == 0)
        {
            // this should be happened when closing the device, just return ok
            return 0;
        }

        bool ioctl_debug = true;

        struct serial_configure uart_config = RT_SERIAL_CONFIG_DEFAULT;

        speed_t speed = tio->c_cflag & CBAUD;
        uart_config.baud_rate = get_uart_baudrate(speed, ioctl_debug);

        int data_bit = tio->c_cflag & CSIZE;
        uart_config.data_bits = get_uart_data_bits(data_bit, ioctl_debug);

        int stop_bit = tio->c_cflag & CSTOPB;
        uart_config.stop_bits = get_uart_stop_bits(stop_bit, ioctl_debug);

        int parity_enable = tio->c_cflag & PARENB;
        int parity_odd = tio->c_cflag & PARODD;
        uart_config.parity = get_uart_parity(parity_enable, parity_odd, ioctl_debug);

        U32 prt_rc = PRT_SemDelete(g_uart_sem);
        if (prt_rc != OS_OK)
        {
            errno = EINVAL;
            return -1;
        }

        rt_err_t rt_rc = rt_device_close(g_uart_dev_p);
        if (rt_rc != RT_EOK)
        {
            errno = EINVAL;
            return -1;
        }

        rt_rc = rt_device_control(g_uart_dev_p, RT_DEVICE_CTRL_CONFIG, &uart_config);
        if (rt_rc != RT_EOK)
        {
            errno = EINVAL;
            return -1;
        }

        rt_rc = rt_device_open(g_uart_dev_p,
                               RT_DEVICE_FLAG_RX_NON_BLOCKING | RT_DEVICE_FLAG_TX_BLOCKING);
        if (rt_rc != RT_EOK)
        {
            errno = EINVAL;
            return -1;
        }

        prt_rc = PRT_SemCreate(0, &g_uart_sem);
        if (prt_rc != OS_OK)
        {
            errno = EINVAL;
            return -1;
        }

        rt_rc = rt_device_set_rx_indicate(g_uart_dev_p, uart_rx_indicate);
        if (rt_rc != RT_EOK)
        {
            errno = EINVAL;
            return -1;
        }

        g_uart_tios = *tio;
    }
    else
    {
        PRT_Printf("ERROR: Unsupported req(0x%08X)\n", req);
        errno = ENOTSUP;
        return -1;
    }

    return 0;
}

int MB_open(const char *filename, int flags, ...)
{
    ASSERT_DEV_IS_SET(-1);

    g_uart_dev_p = rt_device_find(filename);

    g_uart_dev_no = 0;
    ASSERT_DEV_IS_SET(g_uart_dev_no);

    rt_err_t rt_rc = rt_device_open(g_uart_dev_p,
                                    RT_DEVICE_FLAG_RX_NON_BLOCKING | RT_DEVICE_FLAG_TX_BLOCKING);
    if (rt_rc != RT_EOK)
    {
        errno = EINVAL;
        return -1;
    }

    U32 prt_rc = PRT_SemCreate(0, &g_uart_sem);
    if (prt_rc != OS_OK)
    {
        errno = EINVAL;
        return -1;
    }

    rt_rc = rt_device_set_rx_indicate(g_uart_dev_p, uart_rx_indicate);
    if (rt_rc != RT_EOK)
    {
        errno = EINVAL;
        return -1;
    }

    return g_uart_dev_no;
}

ssize_t MB_read(int fd, void *buf, size_t count)
{
    ASSERT_DEV_IS_SET(fd);

    int num_read = 0;

    for (size_t i = 0; i < count; i++)
    {
        rt_size_t rt_rv = rt_device_read(g_uart_dev_p, 0, &((char*)buf)[i], 1);
        if (rt_rv == 1)
        {
            num_read++;
        }
        else
        {
            break;
        }
    }

    return num_read;
}

ssize_t MB_write(int fd, const void *buf, size_t count)
{
    ASSERT_DEV_IS_SET(fd);

    rt_size_t rt_rv = rt_device_write(g_uart_dev_p, 0, buf, count);
    return rt_rv;
}

int MB_cfsetispeed(struct termios *tio, speed_t speed)
{
    return speed ? cfsetospeed(tio, speed) : 0;
}

int MB_cfsetospeed(struct termios *tio, speed_t speed)
{
    if (speed & ~CBAUD)
    {
        errno = EINVAL;
        return -1;
    }

    tio->c_cflag &= ~CBAUD;
    tio->c_cflag |= speed;

    return 0;
}

int MB_tcgetattr(int fd, struct termios *tio)
{
    int rc = ioctl(fd, TCGETS, tio);
    if (rc != 0)
    {
        errno = EINVAL;
        return -1;
    }

    return 0;
}

int MB_tcsetattr(int fd, int act, const struct termios *tio)
{
    if (act < 0 || act > 2)
    {
        errno = EINVAL;
        return -1;
    }

    int rc = ioctl(fd, TCSETS + act, tio);
    if (rc != 0)
    {
        errno = EINVAL;
        return -1;
    }

    return 0;
}

int MB_tcflush(int fd, int queue)
{
    int rc = ioctl(fd, TCFLSH, queue);
    if (rc != 0)
    {
        errno = EINVAL;
        return -1;
    }

    return 0;
}


extern int lwip_getaddrinfo(const char *nodename,
                            const char *servname,
                            const struct addrinfo *hints,
                            struct addrinfo **res);
extern void lwip_freeaddrinfo(struct addrinfo *ai);

extern struct TickModInfo g_tickModInfo;

static inline U32 OsSysGetTickPerSecond(void)
{
    return g_tickModInfo.tickPerSecond;
}

int MB_getaddrinfo(const char *restrict host, const char *restrict serv,
                   const struct addrinfo *restrict hint, struct addrinfo **restrict res)
{
    return lwip_getaddrinfo(host, serv, hint, res);
}

static const char msgs[] =
    "Invalid flags\0"
    "Name does not resolve\0"
    "Try again\0"
    "Non-recoverable error\0"
    "Unknown error\0"
    "Unrecognized address family or invalid length\0"
    "Unrecognized socket type\0"
    "Unrecognized service\0"
    "Unknown error\0"
    "Out of memory\0"
    "System error\0"
    "Overflow\0"
    "\0Unknown error";
const char *MB_gai_strerror(int ecode)
{
    const char *s;
    for (s = msgs, ecode++; ecode && *s; ecode++, s++)
    {
        for (; *s; s++);
    }
    if (!*s)
    {
        s++;
    }
    return s;
}

void MB_freeaddrinfo(struct addrinfo *p)
{
    return lwip_freeaddrinfo(p);
}

int MB_socket(int domain, int type, int protocol)
{
    // remove these flag firstly as lwip doesn't support them right now
    type &= ~(SOCK_CLOEXEC | SOCK_NONBLOCK);
    int rc = lwip_socket(domain, type, protocol);
    return rc;
}

int MB_getsockopt(int fd, int level, int optname, void *restrict optval,
                  socklen_t *restrict optlen)
{
    int rc = lwip_getsockopt(fd, level, optname, optval, optlen);
    return rc;
}

int MB_setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
    int rc = lwip_setsockopt(fd, level, optname, optval, optlen);
    return rc;
}

int MB_bind(int fd, const struct sockaddr *addr, socklen_t len)
{
    int rc = lwip_bind(fd, addr, len);
    return rc;
}

int MB_listen(int fd, int backlog)
{
    int rc = lwip_listen(fd, backlog);
    return rc;
}

int MB_accept(int fd, struct sockaddr *restrict addr, socklen_t *restrict len)
{
    int rc = lwip_accept(fd, addr, len);
    return rc;
}

int MB_connect(int fd, const struct sockaddr *addr, socklen_t len)
{
    int rc = lwip_connect(fd, addr, len);
    return rc;
}

ssize_t MB_sendto(int fd, const void *buf, size_t len, int flags, // static
                  const struct sockaddr *addr, socklen_t alen)
{
    ssize_t rc = lwip_sendto(fd, buf, len, flags, addr, alen);
    return rc;
}

ssize_t MB_send(int fd, const void *buf, size_t len, int flags)
{
    ssize_t rc = lwip_send(fd, buf, len, flags);
    return rc;
}

ssize_t MB_recvfrom(int fd, void *restrict buf, size_t len, int flags, // static
                    struct sockaddr *restrict addr, socklen_t *restrict alen)
{
    ssize_t rc = lwip_recvfrom(fd, buf, len, flags, addr, alen);
    return rc;
}

ssize_t MB_recv(int fd, void *buf, size_t len, int flags)
{
    ssize_t rc = lwip_recv(fd, buf, len, flags);
    return rc;
}

int MB_shutdown(int fd, int how)
{
    int rc = lwip_shutdown(fd, how);
    return rc;
}


int MB_select(int n, fd_set *restrict rfds, fd_set *restrict wfds, fd_set *restrict efds,
              struct timeval *restrict tv)
{
    time_t s = tv ? tv->tv_sec : 0;
    suseconds_t us = tv ? tv->tv_usec : 0;
    long ns = 0;
    const time_t max_time = (1ULL << (8 * sizeof(time_t) - 1)) - 1;

    if (s < 0 || us < 0)
    {
        return __syscall_ret(-EINVAL);
    }

    if (us / 1000000 > max_time - s)
    {
        s = max_time;
        us = 999999;
        ns = 999999999;
    }
    else
    {
        s += us / 1000000;
        us %= 1000000;
        ns = us * 1000;
    }

    if ((wfds != NULL) || (efds != NULL))
    {
        PRT_Printf("ERROR: Unsupported wfds & efds\n");
        errno = EINVAL;
        return -1;
    }

    int fd = -1;
    int num_fd = 0;

    for (int i = 0; i < n; i++)
    {
        if (rfds && FD_ISSET(i, rfds))
        {
            fd = i;
            num_fd++;
        }
    }

    if (num_fd != 1)
    {
        PRT_Printf("ERROR: Unsupported num_fd(%d)\n", num_fd);
        errno = EINVAL;
        return -1;
    }

    if (g_uart_dev_no == fd)
    {
        ASSERT_DEV_IS_SET(fd);

        rt_err_t rt_rc = RT_ERROR;
        rt_size_t rx_data_len = 0;
        rt_rc = rt_device_control(g_uart_dev_p, RT_DEVICE_GET_DATA_LEN_OF_RX_BUFF, &rx_data_len);
        if (rt_rc != RT_EOK)
        {
            PRT_Printf("ERROR: rt_device_control() failed, rt_rc=%d\n", rt_rc);
            errno = EINVAL;
            return -1;
        }
        else if (rx_data_len != 0)
        {
            return n;
        }
        else
        {
            U32 semCnt = 0;

            U32 prt_rc = PRT_SemGetCount(g_uart_sem, &semCnt);
            if (prt_rc != OS_OK)
            {
                errno = EINVAL;
                return -1;
            }

            while (semCnt--)
            {
                prt_rc = PRT_SemPend(g_uart_sem, 0);
                if (prt_rc != OS_OK)
                {
                    errno = EINVAL;
                    return -1;
                }
            }

            U32 timeout = OS_WAIT_FOREVER;
            if ((s != 0) || (ns != 0))
            {
                U32 os_tick_sec = OsSysGetTickPerSecond();
                timeout = s * os_tick_sec + ns / (1000 * 1000 * 1000 / os_tick_sec);
            }

            prt_rc = PRT_SemPend(g_uart_sem, timeout);
            if (prt_rc != OS_OK)
            {
                PRT_Printf("ERROR: PRT_SemPend(%d) failed, prt_rc=0x%X\n", timeout, prt_rc);
                if (prt_rc == OS_ERRNO_SEM_TIMEOUT)
                {
                    errno = ETIMEDOUT;
                    return -1;
                }
                else
                {
                    errno = EINVAL;
                    return -1;
                }
            }
            return n;
        }
    }
    else
    {
        int rc = lwip_select(n, rfds, wfds, efds, tv);
        return rc;
    }
}

int MB_close(int fd)
{
    if (g_uart_dev_no == fd)
    {
        ASSERT_DEV_IS_SET(fd);

        rt_err_t rt_rc = rt_device_close(g_uart_dev_p);
        if (rt_rc != RT_EOK)
        {
            errno = EINVAL;
            return -1;
        }

        g_uart_dev_p = NULL;
        g_uart_dev_no = -1;
        return 0;
    }
    else
    {
        int rc = lwip_close(fd);
        return rc;
    }
}
