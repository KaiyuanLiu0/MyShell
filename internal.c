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
#include "process.h"


int Internal_bg(CMD command, int fd[2])
{
    J job;
    jid_t jid;
    // if the jid is given
    if (command->argc > 2)
    {
        // char* to integer
        jid = atoi(command->argv[1]);
        return Background(jid);
    }
    else
    {
        fprintf(stderr, "%s\n", "bg: no enough arguments");
        return -1;
    }
}

int Internal_cd(CMD command, int fd[2])
{
    int ret;
    char *path;
    // if there is any argument
    if (command->argc > 2)
    {
        // change dir
        ret = chdir(command->argv[1]);
        // set environmental variables
        path = getcwd(NULL, 0);
        setenv("PWD", path, 1);
        free(path);
    }
    else
    {
        // if no arguments given
        // cd to the home directory
        path = getenv("HOME");
        ret = chdir(path);
        setenv("PWD", path, 1);
    }
    return ret;
}

int Internal_clr(CMD command, int fd[2])
{
    // string to clear the screen
    // ANSI escape sequence
    // \033 means escape which is ESC ^[
    // CSI H: move the cursor to the top left corner
    // CSI 2J: clear the entire screen
    // CSI 3J: clear entire screen and delete all lines
    //         saved in the scrollback buffer
    // reference to https://en.wikipedia.org/wiki/ANSI_escape_code
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
    int status;
    char errorMessage[BUFFER_SIZE];
    DIR *dir;
    struct dirent *ptr;
    J job;
    pid_t pid;
    pid = fork();
    if (pid < 0)
    {
        Error("fork error");
        return -1;
    }
    else if (pid == 0)
    {
        // redirect and pipe
        Dup(command, fd);
        // if path given
        if (command->argc > 2)
            dir = opendir(command->argv[1]);
        // path not given
        else
            dir = opendir(getenv("PWD"));
        if (dir == NULL)
        {
            fprintf(stderr, "%s: %s\n", "cannot open dir", command->argv[1]);
            exit(EXIT_FAILURE);
        }
        // loop to display
        while ((ptr = readdir(dir)) != NULL)
            printf("%s\n", ptr->d_name);
        // close the directory
        closedir(dir);
        exit(EXIT_SUCCESS);
    }
    else
    {
        // the same as external background running
        if (command->background)
        {
            ClosePipe(fd);
            signal(SIGCHLD, Fun_SIG_CHLD);
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
        return 0;
    }
}

int Internal_echo(CMD command, int fd[2])
{
    int i;
    int status;
    J job;
    pid_t pid;
    pid = fork();
    if (pid < 0)
    {
        perror("fork error");
        return -1;
    }
    else if (pid == 0)
    {
        // redirect and pipe
        Dup(command, fd);
        // print each arguments
        for (i = 1; i < command->argc - 1; ++i)
        {
            printf("%s ", command->argv[i]);
        }
        printf("%s", "\n");
        exit(EXIT_SUCCESS);
    }
    else
    {
        // the same as external background running
        if (command->background)
        {
            ClosePipe(fd);
            signal(SIGCHLD, Fun_SIG_CHLD);
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
        // call the exec
        ret = execvp(command->argv[1], command->argv + 1);
        if (ret < 0) // exec fail
        {
            return -1;
        }
    }
    return 0;
}

int Internal_exit(CMD command, int fd[2])
{
    // exit 
    exit(EXIT_SUCCESS);
}

int Internal_environ(CMD command, int fd[2])
{
    extern char **environ;
    char **env = environ;
    int status;
    J job;
    pid_t pid;
    pid = fork();
    if (pid < 0) // fork error
    {
        Error("fork error");
        return -1;
    }
    else if (pid == 0) // child
    {
        // redirect and pipe
        Dup(command, fd);
        // loop to display
        while (*env)
            puts(*env++);
        exit(EXIT_SUCCESS);
    }
    else
    {
        // the same as external background running
        if (command->background)
        {
            ClosePipe(fd);
            signal(SIGCHLD, Fun_SIG_CHLD);
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
        return 0;
    }
}

int Internal_fg(CMD command, int fd[2])
{
    jid_t jid;
    if (command->argc > 2)
    {   
        // get the jobid
        jid = atoi(command->argv[1]);
        // call Foreground
        return Foreground(jid);
    }
    else
    {
        fprintf(stderr, "%s\n", "fg: no enough arguments");
        return -1;
    }
}

int Internal_help(CMD command, int fd[2])
{
    int status;
    J job;
    int pid;
    pid = fork();
    if (pid < 0)
    {
        Error("fork error");
        return -1;
    }
    else if (pid == 0)
    {
        // direct and piep
        Dup(command, fd);
        printf("%s\n", "to be done");
        exit(EXIT_SUCCESS);
    }
    else
    {
        // the same as external background running
        if (command->background)
        {
            ClosePipe(fd);
            signal(SIGCHLD, Fun_SIG_CHLD);
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
        return 0;
    }
}

int Internal_jobs(CMD command, int fd[2])
{
    int status;
    J job;
    pid_t pid;
    pid = fork();
    if (pid < 0)
    {
        Error("fork error");
        return -1;
    }
    else if (pid == 0)
    {
        // direct and pipe
        Dup(command, fd);
        PrintJobList(0);
        exit(EXIT_SUCCESS);
    }
    else
    {
        // the same as external background running
        if (command->background)
        {
            ClosePipe(fd);
            signal(SIGCHLD, Fun_SIG_CHLD);
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
        return 0;
    }
}

int Internal_pwd(CMD command, int fd[2])
{
    char *path;
    int status;
    J job;
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
        // the same as external background running
        if (command->background)
        {
            ClosePipe(fd);
            signal(SIGCHLD, Fun_SIG_CHLD);
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
        return 0;
    }
    return 0;
}

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
        // directly calling setenv
        ret = setenv(command->argv[1], command->argv[2], 1);
        return ret;
    }
    return -1;
}

int Internal_shift(CMD command, int fd[2])
{
    // shiftarg();
    return 0;
}

int Internal_test(CMD command, int fd[2])
{
    int status;
    J job;
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
        if (command->background)
        {
            ClosePipe(fd);
            signal(SIGCHLD, Fun_SIG_CHLD);
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
        return 0;
    }
}

int Internal_time(CMD command, int fd[2])
{
    time_t rawTime;
    struct tm *timeInfo;
    char *resultTime;
    int status;
    J job;
    pid_t pid;
    pid = fork();
    if (pid < 0)
    {
        perror("fork error");
        return -1;
    }
    else if (pid == 0) // child
    {
        // get the raw time
        time(&rawTime);
        // change to local time
        timeInfo = localtime(&rawTime); 
        // change format
        resultTime = asctime(timeInfo); 
        Dup(command, fd);
        dprintf(STDOUT_FILENO, "%s", resultTime);
        exit(EXIT_SUCCESS);
    }
    else
    {
        // the same as external background running
        if (command->background)
        {
            ClosePipe(fd);
            signal(SIGCHLD, Fun_SIG_CHLD);
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
        return 0;
    }
}

int Internal_umask(CMD command, int fd[2])
{
    mode_t mode;
    int status;
    J job;
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
            // redirect and pipe
            Dup(command, fd);
            // return the current umask
            mode = umask(0);
            umaks(mode);
            printf("%04o\n", mode);
            exit(EXIT_SUCCESS);
        }
        else
        {
            // the same as external background running
            if (command->background)
            {
                ClosePipe(fd);
                signal(SIGCHLD, Fun_SIG_CHLD);
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
        // directly calling unsetenv
        ret = unsetenv(command->argv[1]);
        return ret;
    }
    return -1;
}
