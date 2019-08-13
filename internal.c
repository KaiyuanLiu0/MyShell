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
#include "internal.h"

extern int Dup(CMD command, int fd[2]);
int Internal_bg(CMD command, int fd[2])
{
    ;    
}

// cd
int Internal_cd(CMD command, int fd[2])
{
    int ret;
    char *path;
    // if there is any argument
    if (command->argc > 2)
    {
        // change dir
        ret = chdir(command->argv[1]);
        // set env
        path = getcwd(NULL, 0);
        setenv("PWD", path, 1);
        free(path);
    }
    else
    {
        // cd to the home path
        path = getenv("HOME");
        ret = chdir(path);
        setenv("PWD", path, 1);
    }
    return ret;
}

// clear
int Internal_clr(CMD command, int fd[2])
{
    // string to clear the screen
    // ANSI escape sequence
    // \033 means escape which is ESC ^[
    // CSI H: move the cursor to the top left corner
    // CSI 2J: clear the entire screen
    // CSI 3J: clear entire screen and delete all lines
    //         saved in the scrollback buffer
    // refer to https://en.wikipedia.org/wiki/ANSI_escape_code
    static char clr[] = "\033[3J\033[H\033[2J";
    ssize_t result = 0;
    result = write(STDOUT_FILENO, clr, strlen(clr));
    if (result == -1)
        return -1;
    else
        return 0;
}

int Internal_dir(CMD command, int fd[2])
{
    pid_t pid;
}

int Internal_echo(CMD command, int fd[2])
{
    int i;
    pid_t pid;
    pid = fork();
    if (pid < 0)
    {
        perror("fork error");
        return -1;
    }
    else if (pid == 0)
    {
        Dup(command, fd);
        for (i = 1; i < command->argc - 1; ++i)
        {
            printf("%s ", command->argv[i]);
        }
        printf("%s", "\n");
        exit(EXIT_SUCCESS);
    }
    else
    {
        waitpid(pid, NULL, 0);
        return 0;
    }
}

// this should not fork a child
// since the original shell should
// be replaced by the new process
int Internal_exec(CMD command, int fd[2])
{
    int ret;
    // argc > 2 means exec has instead command
    if (command->argc > 2)
    {
        ret = execvp(command->argv[1], command->argv + 1);
        if (ret < 0) // exec fail
        {
            return -1;
        }
    }
    return 0;
}

// exit
int Internal_exit(CMD command, int fd[2])
{
    exit(EXIT_SUCCESS);
}

int Internal_environ(CMD command, int fd[2])
{
    extern char **environ;
    char **env = environ;
    pid_t pid;
    pid = fork();
    if (pid < 0) // fork error
    {
        Error("fork error");
        return -1;
    }
    else if (pid == 0) // child
    {
        Dup(command, fd);
        while (*env)
            puts(*env++);
        exit(EXIT_SUCCESS);
    }
    else
    {
        waitpid(pid, NULL, 0);
        return 0;
    }
}

int Internal_fg(CMD command, int fd[2])
{
    ;
}

int Internal_help(CMD command, int fd[2])
{
    int pid;
    pid = fork();
    if (pid < 0)
    {
        Error("fork error");
        return -1;
    }
    else if (pid == 0)
    {
        Dup(command, fd);
        printf("%s\n", "to be done");
        exit(EXIT_SUCCESS);
    }
    else
    {
        return 0;
    }
}

int Internal_jobs(CMD command, int fd[2])
{
    ;
}

// pwd
int Internal_pwd(CMD command, int fd[2])
{
    char *path;
    pid_t pid;
    pid = fork(); // fork a new process
    if (pid < 0)
    {
        Error("fork error");
        return -1;
    }
    else if (pid == 0) // child process
    {
        // get the pwd, allocate memory automatically
        char *path = getcwd(NULL, 0);
        Dup(command, fd);
        dprintf(STDOUT_FILENO, "%s\n", path);
        // free the memory
        free(path);
        // exit the child process
        exit(EXIT_SUCCESS);
    }
    else // parent process
    {
        waitpid(pid, NULL, 0);
    }
    return 0;
}

// exit
int Internal_quit(CMD command, int fd[2])
{
    exit(EXIT_SUCCESS);
}

int Internal_set(CMD command, int fd[2])
{
    int ret;
    // if at least two arguments
    if (command->argc > 3)
    {
        ret = setenv(command->argv[1], command->argv[2], 1);
        return ret;
    }
    return -1;
}

int Internal_shift(CMD command, int fd[2])
{
    shiftarg();
    return 0;
}

int Internal_test(CMD command, int fd[2])
{
    int status;
    pid_t pid;
    pid = fork();
    if (pid < 0) // error
    {
        Error("fork error");
        return -1;
    }
    else if (pid == 0) // child
    {
        pid = fork();
        if (pid < 0)
        {
            Error("fork error");
            return -1;
        }
        else if (pid == 0) // child
        {
            execvp(command->cmd, command->argv);
            exit(EXIT_FAILURE);
        }
        else
        {
            waitpid(pid, &status, 0);
            printf("%s\n", status ? "false" : "true");
            exit(EXIT_SUCCESS);
        }
    }
    else
    {
        waitpid(pid, NULL, 0);
    }
    return 0;
}

int Internal_time(CMD command, int fd[2])
{
    time_t rawTime;
    struct tm *timeInfo;
    char *resultTime;
    pid_t pid;
    pid = fork();
    if (pid < 0)
    {
        perror("fork error");
        return -1;
    }
    else if (pid == 0) // child
    {
        time(&rawTime);                 // get the raw time
        timeInfo = localtime(&rawTime); // change to local time
        resultTime = asctime(timeInfo); // change format
        Dup(command, fd);
        dprintf(STDOUT_FILENO, "%s", resultTime);
        exit(EXIT_SUCCESS);
    }
    else
    {
        waitpid(pid, NULL, 0);
        return 0;
    }
}

int Internal_umask(CMD command, int fd[2])
{
    mode_t mode;
    pid_t pid;
    // set umask if any argument
    if (command->argc > 2)
    {
        umask(strtol(command->argv[1], NULL, 8));
    }
    else
    {
        // print the current umask
        pid = fork();
        if (pid < 0)
        {
            Error("fork error");
            return -1;
        }
        else if (pid == 0)
        {
            Dup(command, fd);
            mode = umask(0);
            printf("%04o\n", mode);
            exit(EXIT_SUCCESS);
        }
        else
        {
            waitpid(pid, NULL, 0);
        }
        
    }
    return 0;
}

int Internal_unset(CMD command, int fd[2])
{
    int ret;
    // at least 1 argument
    if (command->argc > 2)
    {
        ret = unsetenv(command->argv[1]);
        return ret;
    }
    return -1;
}
