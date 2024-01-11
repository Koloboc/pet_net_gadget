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
#include <math.h>
#include "defines.h"
#include "sock.h"

int myid; 		// Ижентификатор гаджета (задается случайно)
char *lip;		// Listen IP адрес
int port_listen;// порт прослушивания
int sock;		// основной сокет
int bc_sock;		// основной сокет

void die(){
	close(sock);
	close(bc_sock);

	printf("Exited!\n");
	exit(EXIT_FAILURE);
}

void usage(const char *prog){
	printf("Usage: %s <listen ip address> [listen port] [id]\n", prog);
	return;
}
void change_id(){
	myid++;
}

// Получаем данные с датчиков и копируем в msg
void getdata(message *msg){
	msg->id = myid;
	msg->brithness = (rand() % 100) + 1;
	msg->temp = (rand() % 1000) + 1;;
	return;
}

void setdisplay(message *msg, message *mydata){
	printf("%d: brith=%d temp=%d\n", myid, mydata->brithness, mydata->temp);
	printf("**************************\n");
	printf("*  %d: DISPLAY\t\t *\n", myid);
	printf("*      brithness=%d\t *\n", msg->brithness);
	printf("*      temp=%d\t\t *\n", msg->temp);
	printf("**************************\n");
	return;
}

int main(int argc, char **argv){

	srand(time(NULL));

	if(argc < 2){
		usage(argv[0]);
		exit(EXIT_SUCCESS);
	}else if(argc < 3){
		port_listen = LISTENPORT;
		myid = (rand() % 100) + 1;
	}else if(argc < 4){
		port_listen = atoi(argv[2]);
		myid = (rand() % 100) + 1;
	}else{
		port_listen = atoi(argv[2]);
		myid = atoi(argv[3]);
	}

	lip = argv[1];

	int aver_brith = 0; // среднее значение яркости (датчик)
	int aver_temp = 0;  // среднее значение температуры (датчик)
	int clients = 0;	// количество пришедших покащаний (для расчета среднего)
	int bias_time = 0;  // смещение времени (для ументшения колизий) лиьо +1 либо -1

	struct timeval tv;	// время ожиданий в сек
	int fdmax;
	message msg, reciv_msg; 
	struct sockaddr_in sa_from; // алрес пришедших данных

	fd_set fds_r, fds_all;
	FD_ZERO(&fds_r);
	FD_ZERO(&fds_all);

	// Init 2 UDP socket (basic - sock, broadcast sock_bc)
	if(Socket())
		die();

	fdmax = 0;
	FD_SET(sock, &fds_all);
	fdmax = max(fdmax, sock);

	// получаем данные с датчиков
	getdata(&msg);

	// Посылаем первое сообщение всем
	send_broadcast(&msg, sizeof(msg));

	while(1){
		tv.tv_sec = bias_time + TIME_UNIT_WAITE; // секунд одилания
		tv.tv_usec = 0;
		fds_r = fds_all;
		int sel = select(fdmax + 1, &fds_r, NULL, NULL, &tv);
		if(sel == -1) {
			perror("unit select: ");
			break;
		}else if(sel == 0){ // time is gone
			getdata(&msg);
			message mydata = msg;
			if(clients){
				// считаем средние значения вместе со своими данными
				clients++;
				msg.brithness = roundf(1.0 * (aver_brith + msg.brithness) / clients);
				msg.temp = roundf(1.0 * (aver_temp + msg.temp) / clients);
			}
			send_broadcast(&msg, sizeof(msg));
			setdisplay(&msg, &mydata);
			aver_brith = aver_temp = clients = 0;
			bias_time = 0;
			continue;
		}
		// Пришли данные
		memset(&reciv_msg, 0, sizeof(reciv_msg));
		for(int i = 0; i <= fdmax; i++){
			if (FD_ISSET(i, &fds_r) && (i == sock)){
				socklen_t len_addr_from = sizeof(struct sockaddr_in);
				int read_bs = recvfrom(i, &reciv_msg, sizeof(reciv_msg), 0, (struct sockaddr*)&sa_from, &len_addr_from);

				if(read_bs > 0){
					if(reciv_msg.id < myid){				// запрос данных или установка дисплея
							getdata(&msg);
							send_msg(&msg, &sa_from);
							setdisplay(&reciv_msg, &msg);
							if(bias_time <= 0)
								bias_time++;			// Мы не master дадим возможность мастеру посылать запросы
					}else if(reciv_msg.id == myid){		// Один и тот же id и ip-address
						char buf[INET_ADDRSTRLEN];
						inet_ntop(sa_from.sin_family, &(sa_from.sin_addr), buf, INET_ADDRSTRLEN);

						if(strcmp(lip, buf) == 0){
							continue;
						}else{
							change_id();
						}
					}else if(reciv_msg.id > myid){		// Возможно мы мастер, подсчитаем сумму пришедших данных
						aver_brith += reciv_msg.brithness;
						aver_temp += reciv_msg.temp;
						clients++;
						if(bias_time >= 0)
							bias_time--;
					}
				}else{
					printf("unit %d: error recive broadcast read_bs=%d\n", myid, read_bs);
				}
			}
		}
	}
	close(sock);
	close(bc_sock);
	return 0;
}
