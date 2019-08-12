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

enum CMD_ID is_internal_cmd(CMD command)
{
    return _NOT_INTERNAL;
}

int is_pipe(char* cmd, int cmdlen)
{
    return 0;
}

int is_io_redirect(char* cmd, int cmdlen)
{
    return 0;
}

int normal_cmd(char* cmd, int cmdlen, int infd, int out, int fork)
{

}

// execute external command
int ExecuteCommand_External(CMD command)
{
    pid_t pid; // process id
    pid = fork(); // create a child process
    if (pid < 0) // error
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
        // if (cmd_id != NOT_INTERNAL)
        //     ExecuteCommand_Internal(cmdl->command[i], cmd_id);
        
        // external command
        ExecuteCommand_External(cmdl->command[i]);
    }
}