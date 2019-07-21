#ifndef CLIENT_H
#define CLIENT_H

#include "common.h"

struct command {
	int id;
	char buf[BUFFERLEN];
};

struct command input_to_command(char*);
void send_cmd(struct command);
void command_ls();
void command_cd();
void command_get();
void command_put();


#endif 
