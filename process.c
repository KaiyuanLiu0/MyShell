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

const char *INTERNAL_COMMAND[_NOT_INTERNAL] = {
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
    "unset"};

// the head of all jobs
// the head of foregound jobs
static J jobList = NULL;
static J fregroundJobList = NULL;

// find the last job
static J LastJob(J job)
{
    J ptr_job;
    if (job == NULL)
        fprintf(stderr, "NOPE\n");
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

int PrintJobList(jid_t jid)
{
    J ptr_job;

    // print a certain job info
    if (jid)
    {
        // first, find it
        ptr_job = FindJob(jid);
        if (ptr_job) // if found
            printf("[%u] %d         %s\n", ptr_job->jid, ptr_job->pid, ptr_job->command->cmd);
        else
            Error("no such job\n");
    }
    else // print all jobs
    {
        ptr_job = jobList;
        while (ptr_job)
        {
            printf("[%u] %d         %s\n", ptr_job->jid, ptr_job->pid, ptr_job->command->cmd);
            ptr_job = ptr_job->next;
        }
    }
}

int AddJob(J job)
{
    J ptr_job = jobList;
    jid_t jid = 0;
    // the job with greatest jid
    while (ptr_job)
    {
        if (jid < ptr_job->jid)
            jid = ptr_job->jid;
        ptr_job = ptr_job->next;
    }
    job->jid = jid + 1;
    ptr_job = jobList;
    if (!ptr_job)
        jobList = job;
    else
    {
        while (ptr_job->next)
            ptr_job = ptr_job->next;
        ptr_job->next = job;
        job->next = NULL;
    }
    PrintJobList(job->jid);
    // PrintJobList(0);
    return 0;
}

J NewJob()
{
    J job = (J)malloc(sizeof(struct Job));
    job->pid = 0;
    job->jid = 0;
    job->command = NULL;
    job->size = 0;
    job->next = NULL;
    return job;
}

J DeleteJob(pid_t pid)
{
    J job = jobList;
    J temp;
    jid_t jid = 0;
    while (job)
    {
        if (job->pid == pid)
        {
            jid = job->jid;
            temp = FindJob(jid);
            temp->next = job->next;
            break;
        }
        else
            job = job->next;
    }
    return job;
}

int WaitChild()
{
    usleep(100000);
}

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
    return command->pipe;
}

int is_io_redirect(CMD command)
{
    return command->in || command->out;
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
    case _DIR:
        flag = Internal_dir(command, fd);
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

int ClosePipe(int *fd)
{
    if (fd[0] != STDIN_FILENO)
        close(fd[0]);
    if (fd[1] != STDOUT_FILENO)
        close(fd[1]);
}

int CheckZombie()
{
    int status, pid;
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0)
    {
        continue;
    }
    return 0;
}

// execute external command
int ExecuteCommand_External(CMD command, int fd[2])
{
    int status;
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
        signal(SIGTSTP, SIG_IGN);
        signal(SIGCONT, SIG_DFL);
        Dup(command, fd);
        if (execvp(command->cmd, command->argv) == -1)
            Error(command->cmd);
        exit(EXIT_FAILURE);
    }
    else // parent process
    {
        if (command->backgroud)
        {
            ClosePipe(fd);
            signal(SIGCHLD, sigchldhdlr);
            job = NewJob();
            job->command = command;
            job->pid = pid;
            AddJob(job);
            setpgid(pid, job->pid);
            WaitChild();
            waitpid(pid, NULL, WNOHANG);
        }
        else
        {
            ClosePipe(fd);
            if (is_pipe(command))
                waitpid(pid, &status, WNOHANG);
            else
                waitpid(pid, &status, WUNTRACED);
        }
        CheckZombie();
        return 0;
    }
}

int Foreground(jid_t jid)
{
    int ret;
    J job = FindJob(jid);
    if (job)
    {
        setpgid(job->pid, job->pid);
        tcsetpgrp(1, job->pid);
        ret = kill(job->pid, SIGCONT);
        if (ret < 0)
            Error("fg: job not found");
        else
            waitpid(job->pid, NULL, WUNTRACED);
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
        ret = kill(job->pid, SIGCONT);
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
    char errorMessage[BUFFER_SIZE];
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
    int status;
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
        if (status > 0)
            printf("Done pid: %d", status);
        // if it is a internal command
        if (cmd_id != _NOT_INTERNAL)
            ExecuteCommand_Internal(cmdl->command[i], cmd_id, fd);
        else // external command
            ExecuteCommand_External(cmdl->command[i], fd);
    }
}