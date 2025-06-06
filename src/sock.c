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
extern int bc_sock;
extern int port_listen;
extern char* lip;
extern char* bip;

// Печатает ip адрес в читаемом виде
void print_ip(struct sockaddr_in *addr){
	char buf[INET_ADDRSTRLEN];
	int port = ntohs(addr->sin_port);
	inet_ntop(addr->sin_family, &(addr->sin_addr), buf, INET_ADDRSTRLEN);
	printf("%s:%d\n", buf, port);
}

/* Создаем два сокета */
/* один для broadcast сообщений */
/* второй для отправки сообщений в ответ */
int Socket(){

	struct sockaddr_in addr;
	int yes = 1;

	// BROADCAST SOCKET
	bc_sock = socket(AF_INET, SOCK_DGRAM, getprotobyname("udp")->p_proto);
	if(bc_sock == -1) {
		perror("broadcast socket: ");
		exit(EXIT_FAILURE);
	}

    if (setsockopt(bc_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("SO_REUSEADDR: ");
		exit(EXIT_FAILURE);
    }

	if(setsockopt(bc_sock, SOL_SOCKET, SO_BROADCAST, &yes, sizeof (yes)) == -1) {
		perror("SO_BROADCAST");
		exit(EXIT_FAILURE);
	}

	// REGULAR SOCKET
	sock = socket(AF_INET, SOCK_DGRAM, getprotobyname("udp")->p_proto);
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("SO_REUSEADDR: ");
		exit(EXIT_FAILURE);
    }
	memset(&addr, 0, sizeof(addr)); // обнуляем
	addr.sin_family = AF_INET;		// семейство
	addr.sin_port = htons(port_listen);// Порт для привязки
	addr.sin_addr.s_addr = INADDR_ANY;


	if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) == -1){
		close(bc_sock);
		close(sock);
		perror("bind failure");
		exit(EXIT_FAILURE);
	}

    return 0;
}

// Посылаем msg по broadcast адресу (указанному в define)
int send_broadcast(message *msg){
	int send_bytes;
	struct sockaddr_in addr;
	socklen_t lenaddr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port_listen);
	if(inet_pton(AF_INET, bip, &(addr.sin_addr)) <= 0){
		perror("inet_pton");
		return 0;
	}
	lenaddr = sizeof(addr);

	if((send_bytes = sendto(bc_sock, msg, sizeof(message), 0, (struct sockaddr*)&addr, lenaddr)) == -1) {
		perror("broadcast sendto");
		return 0;
	}
	LOG("send broad\n");

	return 1;

}

// Посылаем ответный msg по адресу *sa на порт укащанный в port_listen
// Возвращает количество посланыж байт 
int send_msg(message *msg, struct sockaddr_in *sa){
	int bytes;
   	int len_msg = sizeof(message);;
	struct sockaddr_in addr = *sa;
	addr.sin_port = htons(port_listen);
	socklen_t lenaddr = sizeof(addr);

	if((bytes = sendto(bc_sock, msg, len_msg, 0, (struct sockaddr*)&addr, lenaddr))== -1)
		perror("Failure send udp mesg");
	LOG("send mess to: ");
	LOG(&addr);
	return bytes;
}

