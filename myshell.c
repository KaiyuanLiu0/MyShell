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

void Fun_SIG_CHLD()
{
    pid_t pid;
    J job;
    pid = wait(NULL); // catch the child process
    if (pid > 0) // if catched
    {
        job = DeleteJob(pid); // delete it from the job list
        if (job) // delete success
        {
            if (reading) // if it's reading from user
                printf("\n[%d] %d done    %s\n", job->jid, pid, job->command->cmd);
            else
                printf("[%d] %d done    %s\n", job->jid, pid, job->command->cmd);
        }
        free(job);
    }
}

void Error(char* errorMessage)
{
    fprintf(stderr, "%s\n", errorMessage); // print stand error
}

// execute file if assigned 
// this function will redirect the file to stdin
// which means the shell will read constantly from
// the file until it meets the EOF
// precondition: the file name of the file to be executed
// postcondition: 0 for success, -1 for failure
static int ExecFile(char* fileName)
{
    int fd = open(fileName, O_RDONLY); // open the file in read mode
    if (fd < 0)
        return -1;
    dup2(fd, STDIN_FILENO); // duplicate the file discriptor to stdin
    return 0;    
}

// This init will init the shell
// both the environmental variables
// and the handlers of signals
// Precondition: none
// Postcondition: the shell is initialized 
static void init()
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
    // set signal
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGCONT, SIG_DFL);
}


// print a prompt to stdout
// Precondition: none
// Postcondition: prompt printed
static void Prompt()
{
    char user[BUFFER_SIZE];
    char hostname[BUFFER_SIZE];
    char* path = getcwd(NULL, 0);
    getlogin_r(user, sizeof(char) * BUFFER_SIZE); // the user name
    gethostname(hostname, sizeof(char) * BUFFER_SIZE); // the host name
    fprintf(stdout, COLOR_GREEN "%s@%s:", user, hostname);
    fprintf(stdout, COLOR_CYAN "%s$ ", path); // the path
    fprintf(stdout, COLOR_NONE);
}

int main(int argc, char* argv[])
{
    CMDL cmdl;
    int should_run = 1;
    pid_t pid;
    int ret;
    // char *args[MAX_LINE/2 + 1];
    if (argc > 1)
    {
        ret = ExecFile(argv[1]);
        if (ret < 0)
        {
            Error("cannot open file\n");
            exit(ret);
        }
    }
    init(); // init the shell
    while (should_run) 
    {
        Prompt(); // print the prompt message
        cmdl = ReadCommand(); // read command from stdin
        ExecuteCommand(cmdl); // execute it
    }
    return 0;
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