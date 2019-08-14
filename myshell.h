#ifndef MYSHELL_H
#define MYSHELL_H
#include <stdio.h>
#define MAX_LINE 80
#define MAX_TOKEN 80
#define BUFFER_SIZE 80
#define COLOR_NONE "\033[m"
#define COLOR_RED "\033[1;37;41m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_CYAN "\033[0;36m"
#define COLOR_GREEN "\033[0;32;32m"
#define COLOR_GRAY "\033[1;30m"
void init();
void setpath(char* newpath);
// CMDL ReadCommand(); defined in prase.h
// int is_internal_cmd(char* cmd, int cmdlen); defined in process.h
// int is_pipe(char* cmd, int cmdlen); defined in process.h
// int is_io_redirect(char* cmd, int cmdlen); defined in process.h
// int normal_cmd(char* cmd, int cmdlen, int infd, int out, int fork); defined in process.h
void Error(char* errorMessage);
#endif
