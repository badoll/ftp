#include "common.h"

struct packet*
init_packet()
{
	struct packet* new_packet;
	new_packet = (struct packet*) malloc (PACKET_SIZE);
	bzero(new_packet,PACKET_SIZE);
	return new_packet;
}

struct packet* 
packet_ntoh(struct packet* data)
{
	struct packet* new_packet;
	new_packet = (struct packet*) malloc (PACKET_SIZE);
	bzero(new_packet,PACKET_SIZE);
	new_packet->type = ntohs(data->type);
	new_packet->cmd = ntohs(data->cmd);
	new_packet->datalen = ntohs(data->datalen);
	return new_packet;
	/*
	* char* need not to transform ?
	*/
}
struct packet* 
packet_hton(struct packet* data)
{
	struct packet* new_packet;
	new_packet = (struct packet*) malloc (PACKET_SIZE);
	bzero(new_packet,PACKET_SIZE);
	new_packet->type = htons(data->type);
	new_packet->cmd = htons(data->cmd);
	new_packet->datalen = htons(data->datalen);
	return new_packet;
}

void
print_packet(struct packet* p)
{
	cout << "Command:  " << p->cmd << endl;
	cout << "Type:     " << p->type << endl;
	cout << "Data len: " << p->datalen << endl;
	cout << "Data:     " << p->buffer << endl;
}
