#ifndef PROCESS_H_
#define PROCESS_H_
#include <stdio.h>
#include "prase.h"

// job id, for background process
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

// if jid is 0, then print the whole job list
// if jid is not 0, then print the corresponding job
int PrintJobList(jid_t jid);

// create a new initialized job
J NewJob();

// add the job to the job list
int AddJob(J job);

// delete the job from the job list
J DeleteJob(pid_t pid);

// whether it is pipe
int is_pipe(CMD command);

// used for bg
int Background(jid_t jid);

// used for fg
int Foreground(jid_t jid);

// dup stdin and stdout
// this is used for both redirection and pipe
int Dup(CMD command, int fd[2]);

// close the pipe if not closed
int ClosePipe(int *fd);

// wait the child for a short time
// this is used to let the parent process
// wait for some very quick background commands 
// then the shell output will be more fancy
int WaitChild();

// check if any zombie process
int CheckZombie();

// Execute the command
// no matter whether it's internal or external or even wrong
int ExecuteCommand(CMDL);
#endif