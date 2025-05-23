/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-12-31     ShiAnhu      the first version
 */
#ifndef MODBUS_PORT_TCP_H
#define MODBUS_PORT_TCP_H

#include "modbus_port.h"

#if !defined(OS_OPTION_PROXY)

#undef getaddrinfo
#define getaddrinfo(host, serv, hint, res)             MB_getaddrinfo(host, serv, hint, res)

#undef gai_strerror
#define gai_strerror(ecode)                            MB_gai_strerror(ecode)
#undef freeaddrinfo
#define freeaddrinfo(p)                                MB_freeaddrinfo(p)
#undef socket
#define socket(domain, type, protocol)                 MB_socket(domain, type, protocol)
#undef getsockopt
#define getsockopt(fd, level, optname, optval, optlen) MB_getsockopt(fd, level, optname, optval, optlen)
#undef setsockopt
#define setsockopt(fd, level, optname, optval, optlen) MB_setsockopt(fd, level, optname, optval, optlen)
#undef bind
#define bind(fd, addr, len)                            MB_bind(fd, addr, len)
#undef listen
#define listen(fd, backlog)                            MB_listen(fd, backlog)
#undef accept
#define accept(fd, addr, len)                          MB_accept(fd, addr, len)
#undef connect
#define connect(fd, addr, len)                         MB_connect(fd, addr, len)
#undef sendto
#define sendto(fd, buf, len, flags, addr, alen)        MB_sendto(fd, buf, len, flags, addr, alen)
#undef send
#define send(fd, buf, len, flags)                      MB_send(fd, buf, len, flags)
#undef recvfrom
#define recvfrom(fd, buf, len, flags, addr, alen)      MB_recvfrom(fd, buf, len, flags, addr, alen)
#undef recv
#define recv(fd, buf, len, flags)                      MB_recv(fd, buf, len, flags)
#undef shutdown
#define shutdown(fd, how)                              MB_shutdown(fd, how)

#undef select
#define select(n, rfds, wfds, efds, tv)                MB_select(n, rfds, wfds, efds, tv)

#undef close
#define close(fd)                                      MB_close(fd)

#endif // OS_OPTION_PROXY

#endif // MODBUS_PORT_TCP_H
