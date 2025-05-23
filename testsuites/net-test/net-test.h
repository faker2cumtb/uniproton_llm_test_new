#ifndef _CONFORMSNCE_RUN_TEST_H
#define _CONFORMSNCE_RUN_TEST_H

extern int NET_Accept_001();
extern int NET_Accept_EBADF();
extern int NET_Accept_ENOTSOCK();
extern int NET_Accept_EOPNOTSUPP();
extern int NET_Socket_001();
extern int NET_Socket_EMFILE();
extern int NET_Socket_EPROTONOSUPPORT();
extern int NET_Bind_001();
extern int NET_Bind_EADDRINUSE();
extern int NET_Bind_EBADF();
extern int NET_Bind_ENOTSOCK();
extern int NET_Connect_001();
extern int NET_Connect_EBADF();
extern int NET_Connect_EISCONN();
extern int NET_Connect_ENOTSOCK();
extern int NET_Listen_001();
extern int NET_Listen_EBADF();
extern int NET_Listen_ENOTSOCK();
extern int NET_Recv_001();
extern int NET_Recv_EBADF();
extern int NET_Recv_ENOTSOCK();
extern int NET_Recvfrom_001();
extern int NET_Recvfrom_EBADF();
extern int NET_Recvfrom_ENOTSOCK();
extern int NET_Send_001();
extern int NET_Send_ENOTSOCK();
extern int NET_Sendto_001();
extern int NET_Setsockopt_001();
extern int NET_Setsockopt_ENOTSOCK();
extern int NET_Shutdown_001();
extern int NET_Shutdown_ENOTSOCK();
extern int NET_Getsockname_001();
extern int NET_Getsockopt_001();
extern int NET_Getsockopt_EBADF();
extern int NET_Getsockopt_ENOPROTOOPT();
extern int NET_Getsockopt_ENOTSOCK();
extern int NET_Getpeername_001();
extern int NET_Getpeername_EBADF();
extern int NET_Getpeername_ENOTCONN();
extern int NET_Getpeername_ENOTSOCK();
extern int NET_Addrinfo();
extern int NET_Gethost_addr();
extern int NET_Gethost_name();
extern int NET_Gethost_byname();
extern int NET_IF_Nameindex();
extern int NET_Gai_strerror();
extern int NET_Accept4();

typedef int test_run_main();

test_run_main *run_test_arry_1[] = {
    NET_Accept_001,
    NET_Accept_EBADF,
    NET_Accept_ENOTSOCK,
    NET_Accept_EOPNOTSUPP,
    NET_Socket_001,
    NET_Socket_EPROTONOSUPPORT,
    NET_Bind_001,
    NET_Bind_EADDRINUSE,
    NET_Bind_EBADF,
    NET_Bind_ENOTSOCK,
    NET_Connect_001,
    NET_Connect_EBADF,
    NET_Connect_EISCONN,
    NET_Connect_ENOTSOCK,
    NET_Listen_001,
    NET_Listen_EBADF,
    NET_Listen_ENOTSOCK,
    NET_Recv_001,
    NET_Recv_EBADF,
    NET_Recv_ENOTSOCK,
    NET_Recvfrom_001,
    NET_Recvfrom_EBADF,
    NET_Recvfrom_ENOTSOCK,
    NET_Send_001,
    NET_Send_ENOTSOCK,
    NET_Sendto_001,
    NET_Setsockopt_001,
    NET_Setsockopt_ENOTSOCK,
    NET_Shutdown_001,
    NET_Shutdown_ENOTSOCK,
    NET_Getsockname_001,
    NET_Getsockopt_001,
    NET_Getsockopt_EBADF,
    NET_Getsockopt_ENOPROTOOPT,
    NET_Getsockopt_ENOTSOCK,
    NET_Getpeername_001,
    NET_Getpeername_EBADF,
    NET_Getpeername_ENOTCONN,
    NET_Getpeername_ENOTSOCK,
    NET_Addrinfo,
    NET_Gethost_addr,
    NET_Gethost_name,
    NET_Gethost_byname,
    NET_IF_Nameindex,
    NET_Gai_strerror,
    NET_Accept4,
    NET_Socket_EMFILE,
};

char run_test_name_1[][50] = {
    "NET_Accept_001",
    "NET_Accept_EBADF",
    "NET_Accept_ENOTSOCK",
    "NET_Accept_EOPNOTSUPP",
    "NET_Socket_001",
    "NET_Socket_EPROTONOSUPPORT",
    "NET_Bind_001",
    "NET_Bind_EADDRINUSE",
    "NET_Bind_EBADF",
    "NET_Bind_ENOTSOCK",
    "NET_Connect_001",
    "NET_Connect_EBADF",
    "NET_Connect_EISCONN",
    "NET_Connect_ENOTSOCK",
    "NET_Listen_001",
    "NET_Listen_EBADF",
    "NET_Listen_ENOTSOCK",
    "NET_Recv_001",
    "NET_Recv_EBADF",
    "NET_Recv_ENOTSOCK",
    "NET_Recvfrom_001",
    "NET_Recvfrom_EBADF",
    "NET_Recvfrom_ENOTSOCK",
    "NET_Send_001",
    "NET_Send_ENOTSOCK",
    "NET_Sendto_001",
    "NET_Setsockopt_001",
    "NET_Setsockopt_ENOTSOCK",
    "NET_Shutdown_001",
    "NET_Shutdown_ENOTSOCK",
    "NET_Getsockname_001",
    "NET_Getsockopt_001",
    "NET_Getsockopt_EBADF",
    "NET_Getsockopt_ENOPROTOOPT",
    "NET_Getsockopt_ENOTSOCK",
    "NET_Getpeername_001",
    "NET_Getpeername_EBADF",
    "NET_Getpeername_ENOTCONN",
    "NET_Getpeername_ENOTSOCK",
    "NET_Addrinfo",
    "NET_Gethost_addr",
    "NET_Gethost_name",
    "NET_Gethost_byname",
    "NET_IF_Nameindex",
    "NET_Gai_strerror",
    "NET_Accept4",
    "NET_Socket_EMFILE",
};

#endif
