#include "client.h"
using namespace std;
int send_cmd(int,int,struct command);
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
	cout << "connecting to the server"<< endl;
	int data_fd = creat_dsocket(control_fd,argv[1]);
	while (true) {
		/* send command and transfer data */
		string input;
		getline(cin,input);
		struct command cmd = input_to_command(input);
		if (send_cmd(control_fd,data_fd,cmd) == 0) {
			break;
		}
		cout << endl;
	}
	close(control_fd);
}

int
send_cmd(int control_fd, int data_fd, struct command cmd)
{
	switch(cmd.id)
	{
		case GET:
			command_get(control_fd,data_fd,cmd);
			break;
		case PUT:
			command_put(control_fd,data_fd,cmd);
			break;
		case LS:
			command_ls(control_fd,data_fd);
			break;
		case CD:
			command_cd(control_fd,data_fd,cmd);
			break;
		case MKDIR:
			command_mkdir(control_fd,data_fd,cmd);
			break;
		case CCD:
			command_ccd(cmd);
			break;
		case CLS:
			command_cls();
			break;
		case EXIT:
			command_exit(control_fd);
			return 0;
		case -1:
			cerr << "invalid input" << endl;
			return -1;	
	}
	return 1;
}

