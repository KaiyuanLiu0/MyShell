#ifndef MYSHELL_H
#define MYSHELL_H_
#include <stdio.h>
#define MAX_LINE 80
#define MAX_TOKEN 80
void init();
void setpath(char* newpath);
// CMDL ReadCommand(); defined in prase.h
// int is_internal_cmd(char* cmd, int cmdlen); defined in process.h
// int is_pipe(char* cmd, int cmdlen); defined in process.h
// int is_io_redirect(char* cmd, int cmdlen); defined in process.h
// int normal_cmd(char* cmd, int cmdlen, int infd, int out, int fork); defined in process.h
void Error(char* errorMessage);
#endif