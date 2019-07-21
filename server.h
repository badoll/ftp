#ifndef SERVER_H
#define SERVER_H

#include "common.h"

/*maximun number of client connections*/
#define LISTEN_QUE 8

void command_ls();
void command_cd();
void command_get();
void command_put();

#endif





