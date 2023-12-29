#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include "defines.h"
#include "sock.h"

extern int sock;
extern int sock_bc;
extern int port_listen;
extern char* lip;

int Socket(){
	int yes = 1;

	// BROADCAST SOCKET
	sock_bc = socket(AF_INET, SOCK_DGRAM, getprotobyname("udp")->p_proto);
	if(sock_bc == -1) {
		perror("broadcast socket: ");
		return 1;
	}

    if (setsockopt(sock_bc, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("SO_REUSEADDR: ");
		return 1;
    }

	if(setsockopt(sock_bc, SOL_SOCKET, SO_BROADCAST, &yes, sizeof (yes)) == -1) {
		perror("SO_BROADCAST");
		return 1;
	}

	// BASIC SOCKET
	sock = socket(AF_INET, SOCK_DGRAM, getprotobyname("udp")->p_proto);
	if(sock == -1) {
		perror("basic socket: ");
		return 1;
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_port = htons(port_listen);
	addr.sin_family = AF_INET;

	if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) == -1){
		close(sock);
		perror("bind failure");
		exit(EXIT_FAILURE);
	}

    return 0;
}

int send_broadcast(int socket, message *msg, size_t len){
	int send_bytes;
	struct sockaddr_in addr;
	socklen_t lenaddr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port_listen);
	addr.sin_addr.s_addr = inet_addr(BROADCAST);
	lenaddr = sizeof(addr);

	if((send_bytes = sendto(socket, msg, len, 0, (struct sockaddr*)&addr, lenaddr)) == -1)
	{
		perror("sendto:");
		return 0;
	}
	printf("unit %d send broadcast\n", msg->id);

	return 1;

}

int send_msg(int idreq, message *msg, struct sockaddr_in *sa){
	int bytes;
   	int len_msg = sizeof(struct _message);;
	struct sockaddr_in addr;
	socklen_t lenaddr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port_listen);
	addr.sin_addr.s_addr = sa->sin_addr.s_addr;
	lenaddr = sizeof(addr);

	if((bytes = sendto(sock, msg, len_msg, 0, (struct sockaddr*)&addr, lenaddr))== -1){
		perror("Failure send udp mesg");
	}
	return bytes;
}

