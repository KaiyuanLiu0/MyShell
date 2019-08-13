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

// the head of all jobs
// the head of foregound jobs
static J jobList = NULL;
static J fregroundJobList = NULL;

// find the last job
static J LastJob(J job)
{
    J ptr_job;
    while (ptr_job)
    {
        if (ptr_job->next == job)
            return ptr_job;
        else
            ptr_job = ptr_job->next;
    }
    return ptr_job;
}

static J FindJob(jid_t jid)
{
    J ptr_job = jobList;
    while (ptr_job)
    {
        if (ptr_job->jid == jid)
            return ptr_job;
        else
            ptr_job = ptr_job->next;
    }
    return ptr_job;
}

static int PrintJobList(jid_t jid)
{
    J ptr_job;

    // print a certain job info
    if (jid)
    {
        // first, find it 
        ptr_job = FindJob(jid);
        if (ptr_job) // if found
            printf("[%u] %d %s\n", ptr_job->jid, ptr_job->pgid, ptr_job->command->cmd);
        else
            Error("no such job\n");  
    }
    else // print all jobs
    {
        ptr_job = jobList;
        while (ptr_job)
        {
            printf("[%u] %d %s\n", ptr_job->jid, ptr_job->pgid, ptr_job->command->cmd);
            ptr_job = ptr_job->next;
        }
    }
}


static int AddJob(J job)
{
    J ptr_job = job;
    jid_t jid = 0;
    // the job with greatest jid
    while (ptr_job)
    {
        if (jid < ptr_job->jid)
            jid = ptr_job->jid;
        ptr_job = ptr_job->next;
    }
    job->jid = jid + 1;
    ptr_job = LastJob(job);
    if (!ptr_job) // the job list is empty, set the job head
        jobList = job;
    else
        ptr_job->next = job;
    job->next = NULL;
    PrintJobList(job->jid);
    return 0;
}

static J NewJob()
{
    J job = (J)malloc(sizeof(struct Job));
    job->pgid = 0;
    job->jid = 0;
    job->command=NULL;
    job->size = 0;
    job->next = NULL;
    return job;
}
static int ProcessDone(J job)
{
    J ptr_job;
    if (!--job->size)
    {
        ptr_job = LastJob(job); // find the last job
        // delete it from the linked list
        if (ptr_job == NULL) // if it's the head
            jobList  = job->next;
        else
            ptr_job->next = job->next;
        printf("[%d] %d done\t%s\n", job->jid, job->pgid, job->command->cmd);
        free(job);
    }
}

// static int ProcessDone(J job)
// {
//     if (!--job->size)
//         JobDone(job);
// }

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

int is_pipe(CMD command)
{
    return 0;
}

int is_io_redirect(CMD command)
{
    return 0;
}

int normal_cmd(char *cmd, int cmdlen, int infd, int out, int fork)
{
    return 0;
}

static int ExecuteCommand_Internal(CMD command, enum CMD_ID cmd_id, int fd[2])
{
    int flag;
    switch (cmd_id)
    {
    case _BG:
        flag = Internal_bg(command, fd);
        break;
    case _CD:
        flag = Internal_cd(command, fd);
        break;
    case _CLR:
        flag = Internal_clr(command, fd);
        break;
    case _ECHO:
        flag = Internal_echo(command, fd);
        break;
    case _EXEC:
        flag = Internal_exec(command, fd);
        break;
    case _EXIT:
        flag = Internal_exit(command, fd);
        break;
    case _ENVIRON:
        flag = Internal_environ(command, fd);
        break;
    case _FG:
        flag = Internal_fg(command, fd);
        break;
    case _HELP:
        flag = Internal_help(command, fd);
        break;
    case _JOBS:
        flag = Internal_jobs(command, fd);
        break;
    case _PWD:
        flag = Internal_pwd(command, fd);
        break;
    case _QUIT:
        flag = Internal_quit(command, fd);
        break;
    case _SET:
        flag = Internal_set(command, fd);
        break;
    case _SHIFT:
        flag = Internal_shift(command, fd);
        break;
    case _TEST:
        flag = Internal_test(command, fd);
        break;
    case _TIME:
        flag = Internal_time(command, fd);
        break;
    case _UMASK:
        flag = Internal_umask(command, fd);
        break;
    case _UNSET:
        flag = Internal_unset(command, fd);
        break;
    default:
        flag = -1;
    }
}

// execute external command
int ExecuteCommand_External(CMD command, int fd[2])
{
    J job;
    pid_t pid;    // process id
    pid = fork(); // create a child process
    if (pid < 0)  // error
    {
        Error("Cannot create a child process");
        return -1;
    }
    else if (pid == 0) // child process
    {
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGCONT, SIG_DFL);
        Dup(command, fd);
        if (execvp(command->cmd, command->argv) == -1)
            Error(command->cmd);
        exit(EXIT_FAILURE);
    }
    else // parent process
    {
        signal(SIGINT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGCONT, SIG_DFL);
        if (command->backgroud)
        {
            signal(SIGCHLD, SIG_IGN);
            job = NewJob();
            job->command = command;
            setpgid(pid, job->pgid);
            waitpid(pid, NULL, WNOHANG);
        }
        else
            waitpid(pid, NULL, WUNTRACED);
        return 0;
    }
}

int Foreground(jid_t jid)
{
    int ret;
    J job = FindJob(jid);
    if (job)
    {
        tcsetpgrp(STDIN_FILENO, job->pgid);
        ret = kill(job->pgid, SIGCONT);
        if (ret < 0)
            Error("fg: job not found");
    }
    else
        ret = -1;
    return ret;
}

int Background(jid_t jid)
{
    int ret;
    J job = FindJob(jid);
    if (job) // if found
    {
        ret = kill(job->pgid, SIGCONT);
    }
    else
    {
        Error("bg: job not found");
        ret = -1;
    }
    return ret;
}

int Dup(CMD command, int fd[2])
{
    int in_stream = STDIN_FILENO;
    int out_stream = STDOUT_FILENO;
    int o_flag;
    char errorMessage[512];
    if (command->in)
    {
        in_stream = open(command->in, O_RDONLY);
        if (in_stream < 0)
        {
            sprintf(errorMessage, "no such file or directory: %s", command->in);
            return -1;
        }
        else
            dup2(in_stream, STDIN_FILENO);
    }
    if (command->out)
    {
        // append here
        if (command->append)
            o_flag = O_CREAT | O_WRONLY | O_APPEND;
        else
            o_flag = O_CREAT | O_WRONLY | O_TRUNC;
        out_stream = open(command->out, o_flag, 0600);
        if (out_stream < 0)
        {
            sprintf(errorMessage, "no such file or directory: %s", command->out);
            return -1;
        }
        else
            dup2(out_stream, STDOUT_FILENO);
    }
    // printf("Out: %s\n", command->out);
    // printf("%d %d %d %d\n", STDIN_FILENO, STDOUT_FILENO, fd[0], fd[1]);
    if (in_stream == STDIN_FILENO)
        dup2(fd[0], STDIN_FILENO);
    if (out_stream == STDOUT_FILENO)
        dup2(fd[1], STDOUT_FILENO);
}

int ExecuteCommand(CMDL cmdl)
{
    int i = 0;
    int fd[2];
    int pipefd[2] = {STDIN_FILENO, STDOUT_FILENO};
    enum CMD_ID cmd_id;
    // if no command
    if (cmdl == NULL)
        return 0;
    for (i = 0; i < cmdl->size; ++i)
    {
        // flush the stdin and stdout
        fd[0] = pipefd[0];
        if (is_pipe(cmdl->command[i]))
        {
            if (pipe(pipefd) == -1)
                Error("pipe error");
        }
        else 
            pipefd[1] = STDOUT_FILENO;
        fd[1] = pipefd[1];

        fflush(stdin);
        fflush(stdout);
        cmd_id = is_internal_cmd(cmdl->command[i]);
        // if it is a internal command
        if (cmd_id != _NOT_INTERNAL)
            ExecuteCommand_Internal(cmdl->command[i], cmd_id, fd);
        else // external command
            ExecuteCommand_External(cmdl->command[i], fd);
    }
}