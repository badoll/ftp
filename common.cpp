#include "common.h"
using namespace std;

struct packet*
init_packet()
{
	struct packet* new_packet;
	new_packet = (struct packet*) malloc (PACKET_SIZE);
	bzero(new_packet,PACKET_SIZE);
	return new_packet;
}

void
set_zero(struct packet* p)
{
	bzero(p,PACKET_SIZE);
}

struct packet* 
packet_ntoh(struct packet* data,struct packet* hostp)
{
	free(hostp);	
	struct packet* new_packet;
	new_packet = (struct packet*) malloc (PACKET_SIZE);
	bzero(new_packet,PACKET_SIZE);
	new_packet->type = ntohs(data->type);
	new_packet->cmd = ntohs(data->cmd);
	new_packet->datalen = ntohs(data->datalen);
	strcpy(new_packet->buffer,data->buffer);
	return new_packet;
	/*
	* char* need not to transform ?
	*/
}
struct packet* 
packet_hton(struct packet* hostp,struct packet* data)
{
	free(data);
	struct packet* new_packet;
	new_packet = (struct packet*) malloc (PACKET_SIZE);
	bzero(new_packet,PACKET_SIZE);
	new_packet->type = htons(hostp->type);
	new_packet->cmd = htons(hostp->cmd);
	new_packet->datalen = htons(hostp->datalen);
	strcpy(new_packet->buffer,hostp->buffer);
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

int
recv_data(int data_fd,struct packet* data)
{
	int recvlen;
	if ((recvlen = recv(data_fd,data,PACKET_SIZE,0)) <= 0) {
		if (recvlen  == -1) {
			perror("receive error");
			exit(-1);
		}
		if (recvlen == 0) {
			cerr << "error: receive data length = 0" << endl;
			return 0;
		}
	}
	//cerr << "receive data length = " << recvlen << endl;
	/* deal incomplete data */
	if (recvlen < PACKET_SIZE) {
        //cerr << "receive incompletely, continue receiving" << endl;
        if ((recvlen = recv(data_fd,data+recvlen,PACKET_SIZE-recvlen,0)) <= 0) {
			perror("receive error");
            exit(-1);
        }
        //cerr << "receive remaining data length = " << recvlen << endl;
    }
	return 1;
}

int
send_data(int data_fd,struct packet* data)
{
	int sendlen;
	if ((sendlen = send(data_fd,data,PACKET_SIZE,0)) <= 0) {
		if (sendlen  == -1) {
            perror("send error");
            exit(-1);
        }
        if (sendlen == 0) {
			cerr << "error: send data length = 0" << endl;
            return 0;
        }
	}
	//cerr << "send data length = " << sendlen << endl;
	/* deal incomplete data */
	if (sendlen < PACKET_SIZE) {
		//cerr << "send incompletely, continue sending..." << endl;
		if ((sendlen = send(data_fd,data+sendlen,PACKET_SIZE-sendlen,0)) <= 0) {
			perror("send error");
			exit(-1);
		}
		//cerr << "send remaining data length = " << sendlen << endl;
	}
	return 1;
}

void
send_file(int data_fd,FILE* file)
{
	struct packet* hostp = init_packet();
	struct packet* data = init_packet();
	while (!feof(file)) {
		hostp->type = DATA;
		hostp->datalen = fread(hostp->buffer,1,BUFFERLEN,file);
		data = packet_hton(hostp,data);
		if (send_data(data_fd,data) == 0) {
			exit(-1);
		}
		set_zero(hostp);
		set_zero(data);
	}
	hostp->type = DONE;
	data = packet_hton(hostp,data);
	if (send_data(data_fd,data) == 0) {
		cerr << "send done signal failed" << endl;
		exit(-1);
	}
}

void
recv_file(int data_fd,FILE* file)
{
	struct packet* hostp = init_packet();
	struct packet* data = init_packet();
	while (true) {
		if (recv_data(data_fd,data) == 0) {
			exit(-1);
		}
		hostp = packet_ntoh(data,hostp);
		if (hostp->type == DONE) {
			break;
		}
		fwrite(hostp->buffer,1,hostp->datalen,file);
	}

}
