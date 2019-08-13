#ifndef INTERNAL_H_
#define INTERNAL_H_
#include "prase.h"
int Internal_bg(CMD command);
int Internal_cd(CMD command);
int Internal_clr(CMD command);
int Internal_dir(CMD command);
int Internal_echo(CMD command);
int Internal_exec(CMD command);
int Internal_exit(CMD command);
int Internal_environ(CMD command);
int Internal_fg(CMD command);
int Internal_help(CMD command);
int Internal_jobs(CMD command);
int Internal_pwd(CMD command);
int Internal_quit(CMD command);
int Internal_set(CMD command);
int Internal_shift(CMD command);
int Internal_test(CMD command);
int Internal_time(CMD command);
int Internal_umask(CMD command);
int Internal_unset(CMD command);
#endif