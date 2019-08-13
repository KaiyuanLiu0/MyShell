#ifndef INTERNAL_H_
#define INTERNAL_H_
#include "prase.h"
int Internal_bg(CMD command, int fd[2]);
int Internal_cd(CMD command, int fd[2]);
int Internal_clr(CMD command, int fd[2]);
int Internal_dir(CMD command, int fd[2]);
int Internal_echo(CMD command, int fd[2]);
int Internal_exec(CMD command, int fd[2]);
int Internal_exit(CMD command, int fd[2]);
int Internal_environ(CMD command, int fd[2]);
int Internal_fg(CMD command, int fd[2]);
int Internal_help(CMD command, int fd[2]);
int Internal_jobs(CMD command, int fd[2]);
int Internal_pwd(CMD command, int fd[2]);
int Internal_quit(CMD command, int fd[2]);
int Internal_set(CMD command, int fd[2]);
int Internal_shift(CMD command, int fd[2]);
int Internal_test(CMD command, int fd[2]);
int Internal_time(CMD command, int fd[2]);
int Internal_umask(CMD command, int fd[2]);
int Internal_unset(CMD command, int fd[2]);
#endif