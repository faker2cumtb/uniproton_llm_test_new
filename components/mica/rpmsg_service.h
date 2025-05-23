#ifndef _RPMSG_SERVICE_H_
#define _RPMSG_SERVICE_H_

void active_rpmsg_task();
int create_rpmsg_listen_task();
void remove_rpmsg_listen_task();

int create_rpc_ept();
int create_tty_ept();

void remove_rpc_ept();
void remove_tty_ept();

#endif /* _RPMSG_SERVICE_H_ */