#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
/*
	scoket();
	bind();
	recv();
	send();
	inet_aton();
	inet_ntoa();
	AF_INET
	SOCK_STREAM
*/

#include <arpa/inet.h>
/*
	htons();
	ntonl();
	ntohs();
	inet_aton();
	inet_ntoa();
*/

#include <netinet/in.h>
/*
	inet_aton();
	inet)ntoa();
*/

#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#define BUFFERLEN 	500 
/* in struct packet, 500 + 12 = 512 */
#define LINELEN  	256
#define PACKET_SIZE sizeof(struct packet)
#define ADDR_SIZE sizeof(struct sockaddr_in)

enum TYPE
{
	INFO,
	REQU,
	DATA,
	DONE,
	ERROR
};

enum COMMAND
{
	LS,
	CD,
	GET,
	PUT,
	MKDIR,
	CCD,
	CLS,
	EXIT,
	NONE
};
#define N_COMMAND 6
struct packet
{
	int type;     /* enum TYPE */
	int cmd;      /* enum COMMAND */ 
	int datalen;
	char buffer[BUFFERLEN];
};

struct packet* packet_ntoh(struct packet*,struct packet*);
struct packet* packet_hton(struct packet*,struct packet*);
struct packet* init_packet();
void set_zero(struct packet*);
void print_packet(struct packet*);
int recv_data(int,struct packet*);
int send_data(int,struct packet*);
void send_file(int,FILE*);
void recv_file(int,FILE*);



#endif
