#ifndef PROCESS_H_
#define PROCESS_H_
#include <stdio.h>
#include "prase.h"

typedef unsigned int jid_t;
// store background jobs
typedef struct Job {
    pid_t pid;
    jid_t jid;
    CMD command;
    int size;
    struct Job *next;
} *J;

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
int is_pipe(CMD command);
int is_io_redirect(CMD command);
int normal_cmd(char *cmd, int cmdlen, int infd, int out, int fork);
int ExecuteCommand(CMDL);
int Dup(CMD command, int fd[2]);
int PrintJobList(jid_t jid);
#endif