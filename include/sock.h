#ifndef __SOCK_H__
#define __SOCK_H__


int Socket();
int send_broadcast(message *msg);
int send_msg(message *msg, struct sockaddr_in *sa);
void print_ip(struct sockaddr_in *addr);

#endif
