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
	printf("Usage: %s <listen ip address> <listen port>\n", prog);
	//printf("Usage: %s <listen ip> <listen port>\n", prog);
	return;
}

void proc(message *request, message *in_msg, struct sockaddr_in *sf){
	message msg;
	printf("%d: Пришли данные от %d\n", myid, in_msg->id);
	printf("\t%d recive req=%d res=%d type=%d\n", in_msg->id, in_msg->id_req, in_msg->id_res, in_msg->type);
	if(in_msg->type == TIMEOUT){
		msg.id = myid;
		msg.id_req = 0;
		msg.id_res = in_msg->id_req;
		msg.type = MYDATA;
		msg.ds.brithness = (rand() % 100) + 1;
		msg.ds.temp = (rand() % 1000) + 1;;
		printf("\t%d отправка данных %d brithness=%d temp=%d\n",myid, in_msg->id, msg.ds.brithness, msg.ds.temp);
		send_msg(&msg, sf);
	//}else if(in_msg->type == SETDISPLAY){
		printf("\t%d SET DISPLAY brithness=%d temp=%d\n",myid, in_msg->ds.brithness, in_msg->ds.temp);
	}
	return;
}

int main(int argc, char **argv){

	if(argc < 3){
		usage(argv[0]);
		exit(EXIT_SUCCESS);
	}
	lip = argv[1];
	port_listen = atoi(argv[2]);

	int aver_brith = 0;
	int aver_temp = 0;
	int clients = 0;
	int bias_time = 0;

	struct timeval tv;
	int fdmax;
	message bc_msg, reciv_msg;
	struct sockaddr_in sa_from;

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

	bc_msg.id_req = 0;
	bc_msg.id_res = 0;
	bc_msg.id = myid;
	bc_msg.type = IMREADY;
	bc_msg.ds.temp = 0;
	bc_msg.ds.brithness = 0;

	send_broadcast(&bc_msg, sizeof(bc_msg));

	while(1){
		tv.tv_sec = bias_time + TIME_UNIT_WAITE;
		tv.tv_usec = 0;
		fds_r = fds_all;
		int sel = select(fdmax + 1, &fds_r, NULL, NULL, &tv);
		if(sel == -1) {
			printf("unit %d error!\n", myid);
			perror("unit select: ");
			break;
		}else if(sel == 0){ // time is gone
			printf("unit %d: time is gone!\n", myid);
			bc_msg.id_req++;
			bc_msg.id_res = 0;
			bc_msg.id = myid;
			bc_msg.type = TIMEOUT;
			if(clients){
				bc_msg.ds.brithness = aver_brith / clients;
				bc_msg.ds.temp = aver_temp / clients;
			}
			send_broadcast(&bc_msg, sizeof(bc_msg));
			aver_brith = aver_temp = clients = 0;
			bias_time = 0;
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
				   if(reciv_msg.id < myid){				// запрос данных или установка дисплея
						proc(&bc_msg, &reciv_msg, &sa_from);
						if(!bias_time)
							bias_time++;
				   }else if(reciv_msg.id == myid){		// Один и тот же id 
					   if(reciv_msg.id_req == bc_msg.id_req) { 	// получили свой же broadcast
						   continue;
					   }else{
						   if(myid){					//  не "мастер"
							   printf("change id\n");
							   printf("\tid=%d req=%d res=%d type=%d brith=%d temp=%d\n", reciv_msg.id, reciv_msg.id_req,reciv_msg.id_res, reciv_msg.type, reciv_msg.ds.brithness, reciv_msg.ds.temp);
							   myid++;
						   }
					   }
					   // END CHANGE ID
					}else if(reciv_msg.id > myid){
						if(reciv_msg.type == MYDATA){
							aver_brith += reciv_msg.ds.brithness;
							aver_temp += reciv_msg.ds.temp;
							clients++;
							printf("%d считаем дату brith=%d temp=%d cliemts=%d\n", myid, aver_brith, aver_temp, clients);
						}
					}
				}else{
					printf("unit %d: error recive broadcast read_bs=%d\n", myid, read_bs);
				}
			}
		}
	}
	close(sock);
	return 0;
}
