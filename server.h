#ifndef SERVER_H
#define SERVER_H

#include "common.h"

#define SERVER_PORT 2048

/*maximun number of client connections*/
#define LISTEN_QUE 8

int creat_dsocket(int,int);
void command_ls(int);
void command_cd(int,char[]);
void command_get(int);
void command_put(int);
void command_mkdir(int,char[]);

#endif





