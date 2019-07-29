#include "server.h"
using namespace std;

int deal_connection(int,int);
int recv_cmd(int,int);

int
main(int argc, char** argv)
{
	int listen_fd;
	if ((listen_fd = socket(AF_INET,SOCK_STREAM,0)) == -1) {
		perror("socket() error");
		exit(-1);
	}

	struct sockaddr_in server_addr;
	bzero(&server_addr,ADDR_SIZE);
	/* clear structure */

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(SERVER_PORT);

	if (bind(listen_fd,(const struct sockaddr*)&server_addr,
		(socklen_t)sizeof(server_addr)) == -1) {
		perror("bind() error");
		exit(-1);
	}

	if (listen(listen_fd,LISTEN_QUE) == -1) {
		perror("listen() error");
		exit(-1);
	} else 
		cout << "listening to the port: " << SERVER_PORT << endl;
	int DATA_PORT = SERVER_PORT + 1;
	while (true) {
		struct sockaddr_in client_addr;
		bzero(&client_addr,ADDR_SIZE);
		int addr_size = sizeof(client_addr);
		int control_fd = accept(listen_fd,(struct sockaddr*)&client_addr,
							(socklen_t*)&addr_size);
		/* wait for the connection */
		if (control_fd == -1) {
			perror("accept() error");
			exit(-1);
		}
		pid_t child_pid;
		if ((child_pid = fork()) == -1) {
			perror("fork() error");
			exit(-1);
		}
		if (child_pid == 0) {    /* child process deals the connection */
			cout << "connecting..." << endl;
			int data_fd = creat_dsocket(control_fd,DATA_PORT);
			/* new socket to transfer data */
			while (true) {
				if (deal_connection(control_fd,data_fd) == 0)
					break;
			}
			close(data_fd);
			exit(0);
		}
		close(control_fd);
		DATA_PORT++;
	}	
	close(listen_fd);
}

int
deal_connection(int control_fd, int data_fd)
{
	int res;
	if ((res = recv_cmd(control_fd,data_fd)) == -1)
		return -1;
	else if (res == 0)
		return 0;
	return 1;
}

int
recv_cmd(int control_fd, int data_fd)
{
	struct packet* data = init_packet();
	struct packet* hostp = init_packet();
	if (recv_data(control_fd,data) == 0) {
		return -1;
	}
	cout << "receive a command from client" << endl;
	hostp = packet_ntoh(data,hostp);
	switch (hostp->cmd) 
	{
		case GET:
            command_get(data_fd);
            break;
        case PUT:
            command_put(data_fd);
            break;
        case LS:
            command_ls(data_fd);
            break;
        case CD:
            command_cd(data_fd,hostp->buffer);			
			break;
		case MKDIR:
			command_mkdir(data_fd,hostp->buffer);
			break;
		case EXIT:
			cerr << "client exit" << endl;
			return 0;
	}
	return 1;
}
