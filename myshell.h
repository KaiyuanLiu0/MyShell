#ifndef MYSHELL_H
#define MYSHELL_H

#define MAX_LINE 80    // max chars a line can have
#define MAX_TOKEN 80   // max tokens a line can have
#define BUFFER_SIZE 80 // buffer size

#define COLOR_NONE "\033[m"         // no color
#define COLOR_CYAN "\033[0;36m"     // color of cyan
#define COLOR_GREEN "\033[0;32;32m" // color of green



/* 
 * TODO:
*/
void setpath(char* newpath);
// CMDL ReadCommand(); defined in prase.h
// int is_internal_cmd(char* cmd, int cmdlen); defined in process.h
// int is_pipe(char* cmd, int cmdlen); defined in process.h
// int is_io_redirect(char* cmd, int cmdlen); defined in process.h
// int normal_cmd(char* cmd, int cmdlen, int infd, int out, int fork); defined in process.h



// print standard error message
// Precondition: char* string of error message
// Postcondition: error message pritned
void Error(char* errorMessage);

// the handler function of SIG_CHLD which works for background process
// the handler will inform the user when a background process is done
// and will prevent the child process being a zombie process
void Fun_SIG_CHLD();
#endif
