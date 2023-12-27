#ifndef __SOCK_H__
#define __SOCK_H__


int sock;
int Socket();
int Listen();
int send_broadcast(int socket, message *msg, size_t len);

#endif
