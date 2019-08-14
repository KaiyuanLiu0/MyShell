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

int main()
{
    pid_t pid;
    char * argv[] = {"ls", "-al", 0};
    pid=fork();
    if (pid == 0)
        execvp("ls", argv);

    return 0;
}
