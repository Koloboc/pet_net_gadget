#ifndef __SOCKERR_H__
#define __SOCKERR_H__


int sock;
int Socket();
int Listen();
int send_broadcast(int socket, message *msg, size_t len);

#endif
