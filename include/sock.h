#ifndef __SOCK_H__
#define __SOCK_H__


int Socket();
int send_broadcast(int socket, message *msg, size_t len);
int send_msg(int idreq, message *msg, struct sockaddr_in *sa);

#endif
