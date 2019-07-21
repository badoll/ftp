#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
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

#define SERVER_PORT 2048
#define BUFFERLEN 4096
#define PACKET_SIZE sizeof(struct packet)
#define ADDR_SIZE sizeof(struct sockaddr_in)
using namespace std;

enum TYPE
{
	INFO,
	REQU,
	DATA
};

enum COMMAND
{
	NONE,
	LS,
	CD,
	GET,
	PUT,
	EXIT
};

struct packet
{
	int type;     /* enum TYPE */
	int cmd;      /* enum COMMAND */ 
	int datalen;
	char buffer[BUFFERLEN];
};

struct packet* packet_ntoh(struct packet*);
struct packet* packet_hton(struct packet*);
struct packet* init_packet();
void print_packet(struct packet*);

#endif
