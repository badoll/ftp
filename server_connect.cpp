#include "server.h"
using namespace std;

void
command_ls(int data_fd)
{
	struct packet* hostp = init_packet();
	struct packet* data = init_packet();
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
	/* send each file's name one by one */
	for (const auto& f : filename) {
		set_zero(hostp);
		hostp->type = DATA;
		hostp->cmd  = LS;
		hostp->datalen = f.size() + 1;
		strcpy(hostp->buffer,f.c_str());
		data = packet_hton(hostp,data);
		if (send_data(data_fd,data) == 0) {
			break;
		}
	}
	/* send done signal to client */
	set_zero(hostp);
	hostp->type = DONE;
	data = packet_hton(hostp,data);
	if (send_data(data_fd,data) == 0) {
		cerr << "DONE signal is not sended" << endl;
		exit(-1);
	}
}

void 
command_cd(int data_fd, char path[])
{
	struct packet* hostp = init_packet();
	struct packet* data = init_packet();
	int res;
	hostp->type = DONE;
	if ((res = chdir(path)) == -1) {
		hostp->type = ERROR;
		if (errno == ENOENT) {
			strcpy(hostp->buffer,"error: directory does not exist");
			hostp->datalen = strlen(hostp->buffer);
		} else {
			perror("chdir() error");
		}
	}
	data = packet_hton(hostp,data);
	if (send_data(data_fd,data) == 0) {
		cerr << "DONE signal is not sended" << endl;
		exit(-1);
	}
}

void 
command_get(int data_fd)
{
	struct packet* hostp = init_packet();
    struct packet* data = init_packet();
	if (recv_data(data_fd,data) == 0) {
		return;
	}
	hostp = packet_ntoh(data,hostp);
	int n_path = atoi(hostp->buffer);
	for (int i = 0; i < n_path; ++i) {
		set_zero(hostp);
		set_zero(data);
		if (recv_data(data_fd,data) == 0) {
			return;
		}
		hostp = packet_ntoh(data,hostp);
		/* if file does not exist */
		if (access(hostp->buffer,F_OK) == -1) {
			set_zero(hostp);
			hostp->type = ERROR;
			strcpy(hostp->buffer,"error: file does not exist");
			hostp->datalen = strlen(hostp->buffer);
			data = packet_hton(hostp,data);
			if (send_data(data_fd,data) == 0) {
				cerr << "send error signal failed" << endl;
				exit(-1);
			}
			continue;
        }
        FILE* file = fopen(hostp->buffer,"rb");
        if (!file) {
			set_zero(hostp);
			hostp->type = ERROR;
			strcpy(hostp->buffer,"error: file can not open");
			hostp->datalen = strlen(hostp->buffer);
			data = packet_hton(hostp,data);
			if (send_data(data_fd,data) == 0) {
				cerr << "send error signal failed" << endl;
				exit(-1);
			}
			continue;
        }
		set_zero(hostp);
		hostp->type = INFO;
		data = packet_hton(hostp,data);
		if (send_data(data_fd,data) == 0) {
			cerr << "send success signal failed" << endl;
			exit(-1);
		}
        send_file(data_fd,file);
	}
}

void
command_put(int data_fd)
{
	struct packet* hostp = init_packet();
    struct packet* data = init_packet();
	if (recv_data(data_fd,data) == 0) {
		return;
	}
	hostp = packet_ntoh(data,hostp);
	int n_path = atoi(hostp->buffer);
	for (int i = 0; i < n_path; ++i) {
		set_zero(hostp);
		set_zero(data);
		if (recv_data(data_fd,data) == 0) {
			break;
		}
		hostp = packet_ntoh(data,hostp);
		char path[BUFFERLEN+4];
        strcpy(path,hostp->buffer);
        int x = 1;
        while (access(path,F_OK) == 0) {
            sprintf(path,"%s(%d)",hostp->buffer,x);
            x++;
        }
        FILE* file = fopen(path,"wb");
        if (!file) {
			set_zero(hostp);
			hostp->type = ERROR;
			strcpy(hostp->buffer,"error: file can not be created");
			hostp->datalen = strlen(hostp->buffer);
			data = packet_hton(hostp,data);
			if (send_data(data_fd,data) == 0) {
				cerr << "send error signal failed" << endl;
				exit(-1);
			}
			continue;
        }
		set_zero(hostp);
		hostp->type = INFO;
		data = packet_hton(hostp,data);
		if (send_data(data_fd,data) == 0) {
			cerr << "send success signal failed" << endl;
			exit(-1);
		}
		recv_file(data_fd,file);
	}

}

void
command_mkdir(int data_fd, char path[])
{
	struct packet* hostp = init_packet();
	struct packet* data = init_packet();
	int res;
	hostp->type = DONE;

	if ((res = mkdir(path,0777)) == -1) {
		hostp->type = ERROR;
		if (errno == EEXIST) {
			strcpy(hostp->buffer,"error: Name exists");
			hostp->datalen = strlen(hostp->buffer);
		} else {	
			perror("mkdir() error");
		}
	}
	data = packet_hton(hostp,data);
	if (send_data(data_fd,data) == 0) {
		cerr << "DONE signal is not sended" << endl;
		exit(-1);
	}
}

int
bind_addr(int DATA_PORT)
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
    server_addr.sin_port = htons(DATA_PORT);

    if (bind(listen_fd,(const struct sockaddr*)&server_addr,
        (socklen_t)sizeof(server_addr)) == -1) {
        perror("bind() error");
        exit(-1);
    }

    if (listen(listen_fd,LISTEN_QUE) == -1) {
        perror("listen() error");
        exit(-1);
    }
	return listen_fd;
}

int
acpt_clet(int listen_fd)
{
	struct sockaddr_in client_addr;
    bzero(&client_addr,ADDR_SIZE);
    int addr_size = sizeof(client_addr);
    int data_fd = accept(listen_fd,(struct sockaddr*)&client_addr,
                            (socklen_t*)&addr_size);
    /* wait for the connection */
    if (data_fd == -1) {
        perror("accept() error");
        exit(-1);
    }
	cout << "data socket connect successfully" << endl;
	return data_fd;
}

int
creat_dsocket(int control_fd, int DATA_PORT)
{
	cout << "creating data port" << endl;
	int listen_fd = bind_addr(DATA_PORT);
	struct packet* hostp = init_packet();
	struct packet* data  = init_packet();
	hostp->type = INFO;
	hostp->cmd = NONE;
	sprintf(hostp->buffer,"%d",DATA_PORT++);
	cout << "data port: " << hostp->buffer << endl;
	data = packet_hton(hostp,data);
	if (send_data(control_fd,data) == 0) {
		cerr << "data port is not sended" << endl;
		return 0;
	}
	cout << "send data port successfully" << endl;
	int data_fd = acpt_clet(listen_fd);
	return data_fd;
}
