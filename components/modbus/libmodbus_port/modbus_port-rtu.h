/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-12-31     ShiAnhu      the first version
 */
#ifndef MODBUS_PORT_RTU_H
#define MODBUS_PORT_RTU_H

#include "modbus_port.h"

extern int usleep(unsigned sleep_time_in_us);

#undef ioctl
#define ioctl(fd, req, ...)                            MB_ioctl(fd, req, ##__VA_ARGS__)

#undef open
#define open(filename, flags, ...)                     MB_open(filename, flags, ##__VA_ARGS__)
#undef read
#define read(fd, buf, count)                           MB_read(fd, buf, count)
#undef write
#define write(fd, buf, count)                          MB_write(fd, buf, count)
#undef cfsetispeed
#define cfsetispeed(tio, speed)                        MB_cfsetispeed(tio, speed)
#undef cfsetospeed
#define cfsetospeed(tio, speed)                        MB_cfsetospeed(tio, speed)
#undef tcgetattr
#define tcgetattr(fd, tio)                             MB_tcgetattr(fd, tio)
#undef tcsetattr
#define tcsetattr(fd, act, tio)                        MB_tcsetattr(fd, act, tio)
#undef tcflush
#define tcflush(fd, queue)                             MB_tcflush(fd, queue)

#undef select
#define select(n, rfds, wfds, efds, tv)                MB_select(n, rfds, wfds, efds, tv)

#undef close
#define close(fd)                                      MB_close(fd)


#endif // MODBUS_PORT_RTU_H
