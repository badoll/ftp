#include "server.h"

int deal_connection(int);
int recv_cmd(int);
int process_data(int);

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
	}

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
			while (true) {
				if (deal_connection(control_fd) == -1)
					continue;
			}
		}
		close(control_fd);
	}	
	close(listen_fd);
}

int
deal_connection(int control_fd)
{
	if (recv_cmd(control_fd) == -1)
		return -1;
	process_data(control_fd);
	return 0;
}

int
recv_cmd(int control_fd)
{
	struct packet* data = init_packet();
	struct packet* hostp = init_packet();
	if (recv(control_fd,data,BUFFERLEN,0) == -1) {
		cerr << "receive command failed, continue processing..." << endl;
		return -1;
	}
	hostp = packet_ntoh(data);
	switch (hostp->cmd) 
	{
		case GET:
            command_get();
            break;
        case PUT:
            command_put();
            break;
        case LS:
            command_ls();
            break;
        case CD:
            command_cd();			
	}
	return 0;
}

int
process_data(int control_fd)
{
	return 0;
}
