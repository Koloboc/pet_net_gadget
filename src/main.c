#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include "defines.h"
#include "sock.h"

void die(){
	printf("Exited!\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv){
	struct timeval tv;
	message msg;

	srand(time(NULL));
	myid = (rand() % 100) + 1;

	fd_set fds_r, fds_all;
	FD_ZERO(&fds_r);
	FD_ZERO(&fds_all);

	if(Socket())
		die();

	FD_SET(sock, &fds_all);
	int fdmax = 0;
	fdmax = max(fdmax, sock);
	msg.nom = 0;
	msg.id = myid;
	msg.type = IMREADY;
	msg.ds.temp = 100;
	msg.ds.brithness = 50;

	send_broadcast(sock, &msg, sizeof(msg));
	close(sock);
	return 0;
}
