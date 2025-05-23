/*
 * Copyright (c) 2024, NCTI Technologies Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-09-20     ShiAnhu      the first version
 */
#ifndef MODBUS_PORT_H
#define MODBUS_PORT_H

#include <netdb.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <termios.h>
#include <prt_buildef.h>

#include "print.h"
#define printf(msg, ...)      PRT_Printf(msg, ##__VA_ARGS__)
#define fprintf(fd, msg, ...) PRT_Printf(msg, ##__VA_ARGS__)

/***** RTU NON-POSIX APIs *****/
int MB_ioctl(int fd, int req, ...);


/***** RTU POSIX APIs *****/
int MB_open(const char *filename, int flags, ...);
ssize_t MB_read(int fd, void *buf, size_t count);
ssize_t MB_write(int fd, const void *buf, size_t count);
int MB_cfsetispeed(struct termios *tio, speed_t speed);
int MB_cfsetospeed(struct termios *tio, speed_t speed);
int MB_tcgetattr(int fd, struct termios *tio);
int MB_tcsetattr(int fd, int act, const struct termios *tio);
int MB_tcflush(int fd, int queue);


/***** TCP NON-POSIX APIs *****/
int MB_getaddrinfo(const char *restrict host, const char *restrict serv,
                   const struct addrinfo *restrict hint, struct addrinfo **restrict res);


/***** TCP POSIX APIs *****/
const char *MB_gai_strerror(int ecode);
void MB_freeaddrinfo(struct addrinfo *p);
int MB_socket(int domain, int type, int protocol);
int MB_getsockopt(int fd, int level, int optname, void *restrict optval,
                  socklen_t *restrict optlen);
int MB_setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen);
int MB_bind(int fd, const struct sockaddr *addr, socklen_t len);
int MB_listen(int fd, int backlog);
int MB_accept(int fd, struct sockaddr *restrict addr, socklen_t *restrict len);
int MB_connect(int fd, const struct sockaddr *addr, socklen_t len);
ssize_t MB_sendto(int fd, const void *buf, size_t len, int flags,
                  const struct sockaddr *addr, socklen_t alen);
ssize_t MB_send(int fd, const void *buf, size_t len, int flags);
ssize_t MB_recvfrom(int fd, void *restrict buf, size_t len, int flags,
                    struct sockaddr *restrict addr, socklen_t *restrict alen);
ssize_t MB_recv(int fd, void *buf, size_t len, int flags);
int MB_shutdown(int fd, int how);


/***** SHARED NON-POSIX APIs *****/
int MB_select(int n, fd_set *restrict rfds, fd_set *restrict wfds, fd_set *restrict efds,
              struct timeval *restrict tv);


/***** SHARED POSIX APIs *****/
int MB_close(int fd);


#endif // MODBUS_PORT_H
