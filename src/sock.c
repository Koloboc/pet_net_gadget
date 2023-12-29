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
extern int port_listen;
extern char* lip;

int Listen()
{
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_port = htons(port_listen);
	addr.sin_family = AF_INET;
	//inet_aton(lip, &addr.sin_addr.s_addr);

	if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) == -1){
		close(sock);
		perror("bind failure");
		exit(EXIT_FAILURE);
	}

    return 0;
}

int Socket(){
	int yes = 1;

	sock = socket(AF_INET, SOCK_DGRAM, getprotobyname("udp")->p_proto);
	if(sock == -1) {
		perror("socket: ");
		return 1;
	}

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("SO_REUSEADDR: ");
        close(sock);
		return 1;
    }

	if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &yes, sizeof (yes)) == -1) {
		perror("SO_BROADCAST");
		close(sock);
		return 1;
	}

	Listen();

	return 0;
}

int send_broadcast(int socket, message *msg, size_t len){
	int send_bytes;
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port_listen);
	addr.sin_addr.s_addr = inet_addr(BROADCAST);

	if((send_bytes = sendto(socket, msg, len, 0, (struct sockaddr*)&addr, sizeof(addr))) == -1)
	{
		perror("unit %d sendto:");
		return 0;
	}
	printf("unit %d send broadcast\n", msg->id);

	return 1;

}
