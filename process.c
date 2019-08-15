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

// the internal command list
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

// find the job just before it
static J LastJob(J job)
{
    J ptr_job;

    // loop till finding it
    while (ptr_job)
    {
        if (ptr_job->next == job)
            return ptr_job;
        else
            ptr_job = ptr_job->next;
    }
    return ptr_job;
}

// find the job according its jid
static J FindJob(jid_t jid)
{
    J ptr_job = jobList;
    // loop till finding the same jid
    while (ptr_job)
    {
        if (ptr_job->jid == jid)
            return ptr_job;
        else
            ptr_job = ptr_job->next;
    }
    return ptr_job;
}


// tell whether it is a internal command
// precondition: the command
// precondition: return corresponding cid if it is, -1 otherwise
static enum CMD_ID is_internal_cmd(CMD command)
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

// execute the internal command account to its cid
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
    return flag;
}

// execute external command
static int ExecuteCommand_External(CMD command, int fd[2])
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
        // signal handling
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGCONT, SIG_DFL);
        // used for direct and pipe
        Dup(command, fd);
        // execute the command by exec
        if (execvp(command->cmd, command->argv) == -1)
            Error(command->cmd);
        exit(EXIT_FAILURE);
    }
    else // parent process
    {
        // if it runs in background, (a.k.a. it has '&')
        if (command->background)
        {
            // close the pipe
            ClosePipe(fd); 
            // this is important
            // when the parent receives a SIGCHLD signal
            // it will use Fun_SIG_CHLD as a handler
            // then we can prevent the child being a zombie
            signal(SIGCHLD, Fun_SIG_CHLD); 
            // the the child to the job list
            job = NewJob();
            job->command = command;
            job->pid = pid;
            AddJob(job);
            setpgid(pid, job->pid);
            // wait a little time, prompt later
            WaitChild();
            // WNOHSNG is a must
            // otherwise the parent will be blocking
            waitpid(pid, NULL, WNOHANG);
        }
        else
        {
            // close the pipe
            ClosePipe(fd);
            // if it has pipe
            // then we should use WNOHANG
            // otherwise the parent will be blocking
            // zombie process will be checked later
            if (is_pipe(command))
                waitpid(pid, &status, WNOHANG);
            else
                waitpid(pid, &status, WUNTRACED);
        }
        return 0;
    }
}

// print the job list
// there are two different output
// if the jid is 0, then it will 
// print all the jobs in the job list
// otherwise it will only print the
// job which has the same jid
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
        // loop till NULL
        while (ptr_job)
        {
            printf("[%u] %d         %s\n", ptr_job->jid, ptr_job->pid, ptr_job->command->cmd);
            ptr_job = ptr_job->next;
        }
    }
    return 0;
}

// create a initialized job
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

// add the job to the job list
int AddJob(J job)
{
    J ptr_job = jobList;
    jid_t jid = 0;
    // find the job with greatest jid
    while (ptr_job)
    {
        if (jid < ptr_job->jid)
            jid = ptr_job->jid;
        ptr_job = ptr_job->next;
    }
    job->jid = jid + 1;
    ptr_job = jobList;
    if (!ptr_job) // not find the job
        jobList = job; // set it the head
    else
    {
        // loop to find the last job
        // then append the job after it
        while (ptr_job->next)
            ptr_job = ptr_job->next;
        ptr_job->next = job;
        job->next = NULL;
    }
    PrintJobList(job->jid);
    // PrintJobList(0);
    return 0;
}

// delete a job from the job list
// and return the pointer
// it will not free the space
// since the job's info is very
// useful for the caller
J DeleteJob(pid_t pid)
{
    J job = jobList;
    J temp;
    jid_t jid = 0;
    while (job) // job list is not empty
    {
        if (job->pid == pid) // find it
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

int is_pipe(CMD command)
{
    return command->pipe;
}


int CheckZombie()
{
    int status, pid;
    // loop till no zombie process
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0)
    {
        continue;
    }
    return 0;
}

int WaitChild()
{
    usleep(100000);
    return 0;
}

int ClosePipe(int *fd)
{
    if (fd[0] != STDIN_FILENO)
        close(fd[0]);
    if (fd[1] != STDOUT_FILENO)
        close(fd[1]);
    return 0;
}

int Background(jid_t jid)
{
    int ret;
    J job = FindJob(jid);
    // if found
    if (job)
    {
        // send the job SIGCONT
        ret = kill(job->pid, SIGCONT);
    }
    else
    {
        Error("bg: job not found");
        ret = -1;
    }
    return ret;
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

int Dup(CMD command, int fd[2])
{
    // default set to stdin and stdout
    int in_stream = STDIN_FILENO;
    int out_stream = STDOUT_FILENO;
    int o_flag;

    // if need to redirect in ('<')
    if (command->in)
    {
        // set the instream to the file descriptor
        // if open it successfully, dup it to stdin
        in_stream = open(command->in, O_RDONLY);
        // cannot open
        if (in_stream < 0)
        {
            fprintf(stderr, "no such file or directory: %s", command->in);
            return -1;
        }
        else 
            dup2(in_stream, STDIN_FILENO);
    }
    if (command->out)
    {
        // if it is '', then we should append it, otherwise truncate
        if (command->append)
            o_flag = O_CREAT | O_WRONLY | O_APPEND;
        else
            o_flag = O_CREAT | O_WRONLY | O_TRUNC;
        // the same as redirect in
        out_stream = open(command->out, o_flag, 0600);
        // cannot open
        if (out_stream < 0)
        {
            fprintf(stderr, "no such file or directory: %s", command->out);
            return -1;
        }
        else
            dup2(out_stream, STDOUT_FILENO);
    }
    // the following is for pipe
    // if no redirection then we check for whether using pipe
    // both our internal command and external commands functions 
    // have parameters to store the file descriptor
    // we will set the file descriptor correctly when we 
    // execute command sequentially, so just need to dup it
    // if no redirection
    if (in_stream == STDIN_FILENO)
        dup2(fd[0], STDIN_FILENO);
    if (out_stream == STDOUT_FILENO)
        dup2(fd[1], STDOUT_FILENO);
    return 0;
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
        // (COMMAND1 | COMMAND2 we only 
        // think COMMAND1 is 'pipe' and 
        // COMMAND 2 is not 'pipe')

        // set the file descriptor
        // If the last command is 'pipe'
        // then we will dup the pipe out 
        // to stdin. 
        // I f it is not, read from stdin
        fd[0] = pipefd[0];
        if (is_pipe(cmdl->command[i]))
        {
            // open pipe
            // this only change the
            // stdout to pipe in
            if (pipe(pipefd) == -1)
                Error("pipe error");
        }
        else
            pipefd[1] = STDOUT_FILENO;
        fd[1] = pipefd[1];

        // flush the stdin and stdout
        fflush(stdin);
        fflush(stdout);
        // whether it's internal command
        cmd_id = is_internal_cmd(cmdl->command[i]);
        if (status > 0)
            printf("Done pid: %d", status);
        // if it is a internal command
        if (cmd_id != _NOT_INTERNAL)
            ExecuteCommand_Internal(cmdl->command[i], cmd_id, fd);
        else // external command
            ExecuteCommand_External(cmdl->command[i], fd);
        // check any zombie process
        CheckZombie();
    }
        return 0;
}