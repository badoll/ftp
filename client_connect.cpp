#include "client.h"
using namespace std;
static const vector<string> command_str = 
	{
		"ls",
		"cd",
		"get",
		"put",
		"mkdir",
		"!cd",
		"!ls",
		"exit"
	};
/* sequence must be the same as enum COMMAND */
struct command
input_to_command(string input)
{
	istringstream record(input);
	string cmd,str;
	vector<string> path;
	record >> cmd;
	while (record >> str) {
		path.push_back(str);	
	}
	struct command c;
	bool invalid = true;
	for (int i = 0; i < command_str.size(); i++)
		if (cmd == command_str[i]) {
			c.id = i;
			invalid = false;
			break;
		}
	if (invalid)
		c.id = -1;
	c.n_path = 0;
	if (c.id == LS || c.id == CLS || c.id == EXIT || c.id == -1)
		return c;
	/* add filename or path for command */
	else {
		c.n_path = path.size();
		c.path = (char**) malloc (sizeof(char*)*c.n_path);
		for (int i = 0; i < c.n_path; i++) {
			c.path[i] = (char*) malloc (sizeof(char)*path[i].size()+1);
			strcpy(c.path[i],path[i].c_str());
		}
	}
	return c;
}

void 
command_ls(int control_fd, int data_fd)
{
	struct packet* hostp = init_packet();
	struct packet* data = init_packet();
	hostp->type = REQU;
	hostp->cmd = LS;
	hostp->datalen = 0;
	data = packet_hton(hostp,data);	
	if (send_data(control_fd,data) == 0) {
		return;
	}
	set_zero(data);
	set_zero(hostp);
	while (true) {
		if (recv_data(data_fd,data) == 0) {
			break;
		}
		hostp = packet_ntoh(data,hostp);
		if (hostp->type != DATA) {
			break;
		}
		cout << hostp->buffer << endl;
		set_zero(data);
	}
}

void
command_cd(int control_fd, int data_fd, struct command cmd)
{
	struct packet* hostp = init_packet();
	struct packet* data = init_packet();
	hostp->type = REQU;
	hostp->cmd = CD;
	strcpy(hostp->buffer,cmd.path[0]);
	hostp->datalen = strlen(hostp->buffer);
	data = packet_hton(hostp,data);	
	if (send_data(control_fd,data) == 0) {
		return;
	}
	set_zero(data);
	set_zero(hostp);
	if (recv_data(data_fd,data) == 0) {
		return;
	}
	hostp = packet_ntoh(data,hostp);
	if (hostp->type == ERROR) {
		if (hostp->datalen) {
			cerr << hostp->buffer << endl;
		} else {
			cerr << "command cd error" << endl;
		}
	}
}

void
command_ccd(struct command cmd)
{
	char path[LINELEN];
	strcpy(path,cmd.path[0]);
	if (chdir(path) == -1) {
		perror("chdir() error");
	}
}

void
command_cls()
{
	DIR* dir_ptr;
    struct dirent* direntp;
    vector<string> filename;
    if ((dir_ptr = opendir(".")) == NULL) {
        perror("opendir() error");
        exit(-1);
    }
    while ((direntp = readdir(dir_ptr)) != NULL) {
        char *str = (char*) malloc (sizeof(char) * 2 * LINELEN);
        char *type = (char*) malloc (sizeof(char) * 10);
        if (direntp->d_type == DT_REG)
            strcpy(type,"FILE");
        else if (direntp->d_type == DT_DIR)
            strcpy(type,"DIRE");
        else
            strcpy(type,"UNKNOWN");
        sprintf(str,"\t>%s:\t%s", type, direntp->d_name);
        string s(str);
        filename.push_back(s);
        free(str);
        free(type);
    }
    sort(filename.begin(),filename.end());
	for (const auto& s : filename) {
		cout << s << endl;
	}
}

void
command_get(int control_fd, int data_fd, struct command cmd)
{ 
    struct packet* hostp = init_packet();
    struct packet* data = init_packet();
	hostp->type = REQU;
	hostp->cmd = GET;
	data = packet_hton(hostp,data);	
	if (send_data(control_fd,data) == 0) {
		return;
	}
    set_zero(hostp);
    set_zero(data);
	/* send the num of file */
    sprintf(hostp->buffer,"%d",cmd.n_path);
    hostp->type = INFO;
    data = packet_hton(hostp,data);
    if (send_data(data_fd,data) == 0) {
        return;
    }
	for (int i = 0; i < cmd.n_path; ++i) {
    	set_zero(hostp);
    	set_zero(data);
		hostp->type = DATA;
		strcpy(hostp->buffer,cmd.path[i]);
		hostp->datalen = strlen(hostp->buffer);
		data = packet_hton(hostp,data);	
		if (send_data(data_fd,data) == 0) {
			return;
		}
		
		set_zero(data);
		/* ensure file is enable to get */
		if (recv_data(data_fd,data) == 0) {
			return;
		}
		hostp = packet_ntoh(data,hostp);
		if (hostp->type == ERROR) {
			cerr << hostp->buffer << endl;
			continue;
		}
		char path[LINELEN];
		strcpy(path,cmd.path[i]);
		int x = 1;
		while (access(path,F_OK) == 0) {
			sprintf(path,"%s(%d)",cmd.path[i],x);
			x++;
		}
		FILE* file = fopen(path,"wb");
		if (!file) {
			perror("error: file can not be created");
			return;
		}
		recv_file(data_fd,file);
		cout << "get FILE: " << cmd.path[i] << " successfully" << endl;
	}
}

void
command_put(int control_fd, int data_fd, struct command cmd)
{
	struct packet* hostp = init_packet();
	struct packet* data = init_packet();
	hostp->type = REQU;
	hostp->cmd = PUT;
	data = packet_hton(hostp,data);	
	if (send_data(control_fd,data) == 0) {
		return;
	}
	set_zero(hostp);
	set_zero(data);
	/* send the num of file */
	sprintf(hostp->buffer,"%d",cmd.n_path);
	hostp->type = INFO;
	data = packet_hton(hostp,data);
	if (send_data(data_fd,data) == 0) {
		return;
	}
	for (int i = 0; i < cmd.n_path; ++i) {
		set_zero(hostp);
		set_zero(data);
		hostp->type = DATA;
		strcpy(hostp->buffer,cmd.path[i]);
		hostp->datalen = strlen(hostp->buffer);
		data = packet_hton(hostp,data);	
		if (send_data(data_fd,data) == 0) {
			return;
		}
		set_zero(data);
		/* ensure file is enable to get */
		if (recv_data(data_fd,data) == 0) {
			return;
		}
		hostp = packet_ntoh(data,hostp);
		if (hostp->type == ERROR) {
			cerr << hostp->buffer << endl;
			continue;
		}
		if (access(cmd.path[i],F_OK) == -1) {
			cerr << "error: file does not exist" << endl;
			return;
		}
		FILE* file = fopen(cmd.path[i],"rb");
		if (!file) {
			perror("error: file can not open");
			return;
		}
		send_file(data_fd,file);
		cout << "put FILE: " << cmd.path[i] << " successfully" << endl;
	}
}

void
command_mkdir(int control_fd, int data_fd, struct command cmd)
{
	struct packet* hostp = init_packet();
	struct packet* data = init_packet();
	hostp->type = REQU;
	hostp->cmd = MKDIR;
	strcpy(hostp->buffer,cmd.path[0]);
	data = packet_hton(hostp,data);	
	if (send_data(control_fd,data) == 0) {
		return;
	}
	set_zero(data);
	set_zero(hostp);
	if (recv_data(data_fd,data) == 0) {
		return;
	}
	hostp = packet_ntoh(data,hostp);
	if (hostp->type == ERROR) {
		if (hostp->datalen) {
			cerr << hostp->buffer << endl;
		} else {
			cerr << "command mkdir error" << endl;
		}
	}
}

void
command_exit(int control_fd)
{
	struct packet* hostp = init_packet();
	struct packet* data = init_packet();
	hostp->type = REQU;
	hostp->cmd = EXIT;
	hostp->datalen = 0;
	data = packet_hton(hostp,data);	
	if (send_data(control_fd,data) == 0) {
		exit(-1);
	}
}

int
cnet_serv(int DATA_PORT, char* SERVER_IP)
{
	int data_fd = socket(AF_INET,SOCK_STREAM,0);
    if (data_fd == -1) {
        perror("socket() error");
        exit(-1);
    }
    struct sockaddr_in server_addr;
    bzero(&server_addr,ADDR_SIZE);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(DATA_PORT);
	cout << "server ip: " << SERVER_IP << endl;
	cout << "data port: " << DATA_PORT << endl;

    if (connect(data_fd,(const struct sockaddr*)&server_addr,
                (socklen_t)ADDR_SIZE) == -1) {
        perror("connect() error");
        exit(-1);
    }
	return data_fd;
}

int
creat_dsocket(int control_fd, char* SERVER_IP)
{
	struct packet* data = init_packet();
	struct packet* hostp = init_packet();
	if (recv_data(control_fd,data) == 0) {
		cerr << "receive data port error" << endl;
		exit(-1);
	}
	hostp = packet_ntoh(data,hostp);
	if (hostp->type != INFO) {
		cerr << "recv error packet" << endl;
		exit(-1);
	}
	int DATA_PORT = atoi(hostp->buffer);
	int data_fd = cnet_serv(DATA_PORT,SERVER_IP);
	return data_fd;
}

