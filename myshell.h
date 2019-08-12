#ifndef MYSHELL_H
#define MYSHELL_H_
#include <stdio.h>
#define MAX_LINE 80
#define MAX_TOKEN 80
void init();
void setpath(char* newpath);
int readcommand();
int is_internal_cmd(char* cmd, int cmdlen);
int is_pipe(char* cmd, int cmdlen);
int is_io_redirect(char* cmd, int cmdlen);
int normal_cmd(char* cmd, int cmdlen, int infd, int out, int fork);
// change later
void Error(char* errorMessage)
{
    perror(errorMessage);
}
#endif