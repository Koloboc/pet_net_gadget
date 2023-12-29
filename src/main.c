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

int myid; 		// Ижентификатор гаджета (задается случайно)
char *lip;		// Listen IP адрес
int port_listen;// порт прослушивания
int sock;		// основной сокет
int sock_bc;	// broadcast сокет
int master;		// Истинный Master or not

void die(){
	close(sock);
	close(sock_bc);

	printf("Exited!\n");
	exit(EXIT_FAILURE);
}

void usage(const char *prog){
	printf("Usage: %s <listen port>\n", prog);
	//printf("Usage: %s <listen ip> <listen port>\n", prog);
	return;
}

void proc(message *req, message *res, struct sockaddr_in *sf, socklen_t len_addr_from){
	message msg;
	if(res->id != myid){
		printf("%d: Пришли данные от %d\n", myid, res->id);
		printf("\trecive id=%d req=%d res=%d type=%d\n", res->id, res->id_req, res->id_res, res->type);
		if(res->id < myid){
			msg.id = myid;
			msg.id_req = 0;
			msg.id_res = res->id_req;
			msg.type = MYDATA;
			msg.ds.brithness = 11;
			msg.ds.temp = 20;

			send_msg(res->id_req, &msg, sf);
		}else if(res->type == MYDATA){
			printf("\t\tDATA id=%d br=%d t=%d\n", res->id, res->ds.brithness, res->ds.temp);
		}
	}
	return;
}

int main(int argc, char **argv){

	//lip = argv[1];
	port_listen = atoi(argv[1]);

	struct timeval tv;
	int fdmax;
	message msg, reciv_msg;
	struct sockaddr_in sa_from;

	if(argc < 2){
		usage(argv[0]);
		exit(EXIT_SUCCESS);
	}

	srand(time(NULL));
	myid = (rand() % 100) + 1;

	fd_set fds_r, fds_all;
	FD_ZERO(&fds_r);
	FD_ZERO(&fds_all);

	// Init 2 UDP socket (basic - sock, broadcast sock_bc)
	if(Socket())
		die();

	fdmax = 0;
	FD_SET(sock, &fds_all);
	fdmax = max(fdmax, sock);
	FD_SET(sock_bc, &fds_all);
	fdmax = max(fdmax, sock_bc);

	msg.id_req = 0;
	msg.id_res = 0;
	msg.id = myid;
	msg.type = IMREADY;
	msg.ds.temp = 100;
	msg.ds.brithness = 50;

	send_broadcast(sock_bc, &msg, sizeof(msg));

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
			msg.id_req++;
			msg.id_res = 0;
			msg.id = myid;
			msg.type = TIMEOUT;
			send_broadcast(sock_bc, &msg, sizeof(msg));
			continue;
		}
		// Пришли данные
		memset(&reciv_msg, 0, sizeof(reciv_msg));
		for(int i = 0; i <= fdmax; i++){
			if (FD_ISSET(i, &fds_r) && (i == sock)){
				socklen_t len_addr_from = 0;
				int read_bs = recvfrom(i, &reciv_msg, sizeof(reciv_msg), 0, (struct sockaddr*)&sa_from, &len_addr_from);
				if(read_bs > 0){
				   if(read_bs != sizeof(reciv_msg)) {
					   printf("recive bytes != size struct!");
				   }
					//printf("unit %d: recive id=%d req=%d res=%d\n", myid, reciv_msg.id, reciv_msg.id_req, reciv_msg.id_res);
					proc(&msg, &reciv_msg, &sa_from, len_addr_from);
				}else{
					printf("unit %d: error recive broadcast read_bs=%d\n", myid, read_bs);
				}
			}
		}
	}
	close(sock);
	return 0;
}
