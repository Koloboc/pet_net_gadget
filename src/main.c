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

extern int port_listen;

void die(){
	printf("Exited!\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv){
	if(argc > 1){ 
		port_listen = atoi(argv[1]);
	}
	struct timeval tv;
	int fdmax = 0;
	message msg, reciv_msg;

	srand(time(NULL));
	myid = (rand() % 100) + 1;

	fd_set fds_r, fds_all;
	FD_ZERO(&fds_r);
	FD_ZERO(&fds_all);

	if(Socket())
		die();

	FD_SET(sock, &fds_all);
	fdmax = max(fdmax, sock);
	msg.nom = 0;
	msg.id = myid;
	msg.type = IMREADY;
	msg.ds.temp = 100;
	msg.ds.brithness = 50;

	send_broadcast(sock, &msg, sizeof(msg));

	while(1){
		tv.tv_sec = TIME_UNIT_WAITE;
		tv.tv_usec = 0;
		fds_r = fds_all;
		int sel = select(fdmax + 1, &fds_r, NULL, NULL, &tv);
		if(sel == -1) {
			printf("unit %d error!\n", myid);
			perror("unit select: ");
			break;
		}else if(sel == 0){ // time is gone
			printf("unit %d: time is gone!\n", myid);
			msg.nom++;
			msg.id = myid;
			msg.type = IMREADY;
			send_broadcast(sock, &msg, sizeof(msg));
			continue;
		}
		// Пришли даееые
		memset(&reciv_msg, 0, sizeof(reciv_msg));
		for(int i = 0; i <= fdmax; i++){
			if (FD_ISSET(i, &fds_r) && (i == sock)){
				int read_bs = recvfrom(i, &reciv_msg, sizeof(reciv_msg), 0, NULL, NULL);
				if((read_bs > 0) && (read_bs == sizeof(reciv_msg))) {
					
					printf("unit %d: recive id=%d num=%d\n", myid, reciv_msg.id, reciv_msg.nom);
				}else{
					printf("unit %d: error recive broadcast\n", myid);
				}
			}
		}
	}
	close(sock);
	return 0;
}
