#include "client.h"
void send_cmd(struct command);

int
main(int argc, char** argv)
{
	int control_fd = socket(AF_INET,SOCK_STREAM,0);
	if (control_fd == -1) {
		perror("socket() error");
		exit(-1);
	}
	struct sockaddr_in server_addr;
	bzero(&server_addr,ADDR_SIZE);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(atoi(argv[2]));
	
	if (connect(control_fd,(const struct sockaddr*)&server_addr,
				(socklen_t)ADDR_SIZE) == -1) {
		perror("connect() error");
		exit(-1);
	}

	while (true) {
		/* send command and transfer data */
		char input[BUFFERLEN];
		cin >> input;
		struct command cmd = input_to_command(input);
		if (cmd.id == EXIT)
			break;
		send_cmd(cmd);		
	}
	close(control_fd);
}

void
send_cmd(struct command cmd)
{
	switch(cmd.id)
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
}
