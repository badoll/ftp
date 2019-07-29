#ifndef CLIENT_H
#define CLIENT_H

#include "common.h"

struct command {
	int id;
	int n_path;
	char **path;
};

int creat_dsocket(int,char*);
struct command input_to_command(std::string);
int send_cmd(int,int,struct command);
void command_ls(int,int);
void command_cd(int,int,struct command);
void command_get(int,int,struct command);
void command_put(int,int,struct command);
void command_mkdir(int,int,struct command);
void command_ccd(struct command);
void command_cls();
void command_exit(int);



#endif 
