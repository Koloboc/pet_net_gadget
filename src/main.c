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
#include <unistd.h>
#include "defines.h"
#include "sock.h"

int myid = 0; 						// Ижентификатор гаджета (задается случайно)
char *lip = ADDRESS;			// Listen IP адрес
char *bip = BROADCAST;
int port_listen = LISTENPORT;	// порт прослушивания
int sock;						// сокет
int bc_sock;					// широковещательный сокет

void die(){
	close(sock);
	close(bc_sock);

	printf("Exited!\n");
	exit(EXIT_FAILURE);
}

void usage(const char *prog){
	printf("Usage: %s -a [listen ip address] -p [listen port] -b [broadcast address] -i [id]\n", prog);
	return;
}

// Получаем данные с датчиков и копируем в msg
void getdata(message *msg){
	msg->id = myid;
	msg->brithness = (rand() % 100) + 1;
	msg->temp = (rand() % 1000) + 1;;
	return;
}

void setdisplay(message *rec_msg, message *my_data){
	printf("************************************\n");
	printf("ID\tTEMP\t\tBRITHNESS*\n");
	printf("____________________________________\n");
	if(my_data->brithness != rec_msg->brithness && my_data->temp == rec_msg->temp){
		printf("%d:\tAVERRAGE\t\t \n", myid);
	}
		printf("%d:\t%.2f\t\t%.2f \n", my_data->id, my_data->temp, my_data->brithness);
		printf("%d:\t%.2f\t\t%.2f \n", rec_msg->id, rec_msg->temp, rec_msg->brithness);
		printf("************************************\n");
	return;
}

static char const short_options[] = "a:p:i:b:";

int main(int argc, char **argv){

	srand(time(NULL));
	int argRes = 0;

	while((argRes = getopt(argc, argv, short_options)) != -1)
	{
		switch(argRes)
		{
			case 'a':
				lip = optarg;
				break;
			case 'p':
				port_listen = atoi(optarg);
				break;
			case 'i':
				myid = atoi(optarg);
				break;
			case 'b':
				bip = optarg;
				break;
			case '?':
			default:
				usage(argv[0]);
				exit(EXIT_SUCCESS);
		}
	}
	if(!myid)
		myid = rand() % 100 + 1;

	int avg_br = 0; // среднее значение яркости (датчик)
	int avg_te = 0;  // среднее значение температуры (датчик)
	int clients = 0;	// количество пришедших показаний (для расчета среднего)
	int bias_time = 0;  // смещение времени (для ументшения колизий) лиьо +1 либо -1
	int fdmax = 0;

	struct timeval tv;	// время ожиданий в сек
	message msg, reciv_msg; 
	struct sockaddr_in sa_from; // алрес пришедших данных

	fd_set fds_r, fds_all;
	FD_ZERO(&fds_r);
	FD_ZERO(&fds_all);

	// Init 2 UDP socket (basic - sock, broadcast sock_bc)
	if(Socket())
		die();

	FD_SET(sock, &fds_all);
	fdmax = max(fdmax, sock);
	FD_SET(bc_sock, &fds_all);
	fdmax = max(fdmax, bc_sock);

	// получаем данные с датчиков
	getdata(&msg);

	// Посылаем первое сообщение всем
	send_broadcast(&msg);

	while(1){
		tv.tv_sec = bias_time + TIME_UNIT_WAITE; // секунд ожидания
		tv.tv_usec = 0;
		fds_r = fds_all;

		int sel = select(fdmax + 1, &fds_r, NULL, NULL, &tv);
		if(sel == -1) {
			perror("unit select: ");
			break;
		}else if(sel == 0){ // time is gone
			getdata(&msg);
			message avg;
			if(clients){
				// считаем средние значения вместе со своими данными
				clients++;
				avg.brithness = roundf(1.0 * (avg_br + msg.brithness) / clients);
				avg.temp = roundf(1.0 * (avg_te + msg.temp) / clients);
			}else{
				avg = msg;
			}
			printf("============================\n");
			printf("=======time is gone=========\n");
			setdisplay(&avg, &msg);
			send_broadcast(&avg);
			avg_br = avg_te = clients = 0;
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
						/* send_broadcast(&msg, sizeof(msg)); */
						setdisplay(&reciv_msg, &msg);
						if(bias_time <= 0)
							bias_time++;			// Мы не master дадим возможность мастеру посылать запросы
						
					}else if(reciv_msg.id == myid){		// Один и тот же id и ip-address
						char buf[INET_ADDRSTRLEN];
						inet_ntop(sa_from.sin_family, &(sa_from.sin_addr), buf, INET_ADDRSTRLEN);

						if(strcmp(lip, buf) == 0){ // мой ip addr
							continue;
						}else{
							myid++;
						}
					}else if(reciv_msg.id > myid){		// Возможно мы мастер, подсчитаем сумму пришедших данных
						avg_br += reciv_msg.brithness;
						avg_te += reciv_msg.temp;
						clients++;
						if(bias_time > 0)
							bias_time = 0;
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
