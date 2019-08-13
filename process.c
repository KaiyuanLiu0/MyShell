#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include "process.h"
#include "internal.h"

const char* INTERNAL_COMMAND[_NOT_INTERNAL] = {
    "bg",
    "cd",
    "clr",
    "dir",
    "echo",
    "exec",
    "exit",
    "environ",
    "fg",
    "help",
    "jobs",
    "pwd",
    "quit",
    "set",
    "shift",
    "test",
    "time",
    "umask",
    "unset"    
};

enum CMD_ID is_internal_cmd(CMD command)
{
    enum CMD_ID i = 0;
    // compare with every internal command
    for (i = _BG; i < _NOT_INTERNAL; ++i)
    {
        if (!strcmp(command->cmd, INTERNAL_COMMAND[i]))
            return i;
    }
    // no internal command match
    return _NOT_INTERNAL;
}

int is_pipe(char *cmd, int cmdlen)
{
    return 0;
}

int is_io_redirect(char *cmd, int cmdlen)
{
    return 0;
}

int normal_cmd(char *cmd, int cmdlen, int infd, int out, int fork)
{
}

static int ExecuteCommand_Internal(CMD command, enum CMD_ID cmd_id)
{
    int flag;
    switch (cmd_id)
    {
    case _BG:
        flag = Internal_bg(command);
        break;
    case _CD:
        flag = Internal_cd(command);
        break;
    case _CLR:
        flag = Internal_clr(command);
        break;
    case _ECHO:
        flag = Internal_echo(command);
        break;
    case _EXEC:
        flag = Internal_exec(command);
        break;
    case _EXIT:
        flag = Internal_exit(command);
        break;
    case _ENVIRON:
        flag = Internal_environ(command);
        break;
    case _FG:
        flag = Internal_fg(command);
        break;
    case _HELP:
        flag = Internal_help(command);
        break;
    case _JOBS:
        flag = Internal_jobs(command);
        break;
    case _PWD:
        flag = Internal_pwd(command);
        break;
    case _QUIT:
        flag = Internal_quit(command);
        break;
    case _SET:
        flag = Internal_set(command);
        break;
    case _SHIFT:
        flag = Internal_shift(command);
        break;
    case _TEST:
        flag = Internal_test(command);
        break;
    case _TIME:
        flag = Internal_time(command);
        break;
    case _UMASK:
        flag = Internal_umask(command);
        break;
    case _UNSET:
        flag = Internal_unset(command);
        break;
    default:
        flag = -1;
    }
}

// execute external command
int ExecuteCommand_External(CMD command)
{
    pid_t pid;    // process id
    pid = fork(); // create a child process
    if (pid < 0)  // error
    {
        Error("Cannot create a child process");
        return -1;
    }
    else if (pid == 0) // child process
    {
        if (execvp(command->cmd, command->argv) == -1)
            Error(command->cmd);
        exit(EXIT_FAILURE);
    }
    else // parent process
    {
        waitpid(pid, NULL, 0);
        return 0;
    }
}

int ExecuteCommand(CMDL cmdl)
{
    int i = 0;
    enum CMD_ID cmd_id;
    // if no command
    if (cmdl == NULL)
        return 0;
    for (i = 0; i < cmdl->size; ++i)
    {
        // flush the stdin and stdout
        fflush(stdin);
        fflush(stdout);
        cmd_id = is_internal_cmd(cmdl->command[i]);
        // if it is a internal command
        if (cmd_id != _NOT_INTERNAL)
            ExecuteCommand_Internal(cmdl->command[i], cmd_id);
        else // external command
        ExecuteCommand_External(cmdl->command[i]);
    }
}