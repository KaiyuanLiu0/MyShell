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
#include "prase.h"
#include "myshell.h"

void sigchldhdlr()
{
    pid_t pid;
    J job;
    pid = wait(NULL);
    if (pid > 0)
    {
        job = DeleteJob(pid);
        if (job)
        {
            if (reading)
                printf("\n[%d] %d done    %s\n", job->jid, pid, job->command->cmd);
            else
                printf("[%d] %d done    %s\n", job->jid, pid, job->command->cmd);
        }
        free(job);
        // Prompt();
    }
}

// print a prompt
// precondition: none
// postcondition: a prompt printed
void Prompt()
{
    char user[BUFFER_SIZE];
    char hostname[BUFFER_SIZE];
    char* path = getcwd(NULL, 0);
    getlogin_r(user, sizeof(char) * BUFFER_SIZE);
    gethostname(hostname, sizeof(char) * BUFFER_SIZE);
    fprintf(stdout, COLOR_GREEN "%s@%s:", user, hostname);
    fprintf(stdout, COLOR_CYAN "%s$ ", path);
    fprintf(stdout, COLOR_NONE);
}

// init the shell
// precondition: none
// postcondition: the shell is initialized
void init()
{
    // the shell path
    char* path;
    // link to the path
    char* link = "/proc/self/exe";
    char* parent;
    // the temporarily buffer size of the path
    ssize_t pathSize = 1024;
    ssize_t len = 0;
    path = (char*) malloc(sizeof(char) * pathSize);
    len = readlink(link, path, pathSize);
    parent = (char*) malloc(sizeof(char) * (strlen(path) + 9));
    sprintf(parent, "%s/%s", path, "myshell");
    // if the buffer is small
    while (len >= pathSize)
    {
        // free the space and allocate a larger one
        free(path);
        pathSize *= 2;
        path = (char*) malloc(sizeof(char) * pathSize);
    }
    fflush(stdout);
    // set the path an environment variable
    setenv("myshell", path, 1);
    setenv("parent", parent, 1);
    // setpath("/bin:/usr/bin");
    free(path);
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGCONT, SIG_DFL);
}


void Error(char* errorMessage)
{
    perror(errorMessage);
}

int main(int argc, char* argv[])
{
    CMDL cmdl;
    char *args[MAX_LINE/2 + 1];
    int should_run = 1;
    pid_t pid;
    init();
    while (should_run)
    {
        Prompt();
        cmdl = ReadCommand();
        ExecuteCommand(cmdl);
    }
    // int i;
    // CMDL cmdl;
    // cmdl = ReadCommand();
    // printf("cmd: %s\n", cmdl->command[1]->cmd);
    // printf("args: ");
    // for (i = 0; i < cmdl->command[1]->argc - 1; ++i)
    //     printf("%s ", cmdl->command[1]->argv[i]);
    // printf("\n");
    // printf("in: %s\n", cmdl->command[1]->in);
    // printf("out: %s\n", cmdl->command[1]->out);
    // printf("size: %d\n", cmdl->size);
}