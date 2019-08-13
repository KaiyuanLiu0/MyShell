#ifndef PROCESS_H_
#define PROCESS_H_
#include <stdio.h>
#include "prase.h"

// store background jobs
typedef struct Job {
    pid_t pid;
    CMD cmd;
} *J;

typedef struct JobList {
    J job;
    int size;
} *JL;


// itnernal command id
enum CMD_ID
{
    _BG, // 0
    _CD,
    _CLR,
    _DIR,
    _ECHO,
    _EXEC,
    _EXIT,
    _ENVIRON,
    _FG,
    _HELP,
    _JOBS,
    _PWD,
    _QUIT,
    _SET,
    _SHIFT,
    _TEST,
    _TIME,
    _UMASK,
    _UNSET,
    _NOT_INTERNAL
};

enum CMD_ID is_internal_cmd(CMD);
int is_pipe(char *cmd, int cmdlen);
int is_io_redirect(char *cmd, int cmdlen);
int normal_cmd(char *cmd, int cmdlen, int infd, int out, int fork);
int ExecuteCommand(CMDL);
#endif