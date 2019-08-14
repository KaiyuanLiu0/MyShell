/*
 * file: prase.h
 * The prase used to prase the command
 */
#ifndef PRASE_H_
#define PRASE_H_
#include <stdbool.h>
#include "myshell.h"

#define DELIMIT " \t\r\n\a"
// struct to store the single command
typedef struct Command
{
    char* cmd;
    char** argv;
    int argc; // argument count 
    char* in;  // redirect in
    char* out; // redirect out
    bool append; // append or replace
    bool backgroud; // whether backgroud
    bool pipe; // whether output to pipe
} *CMD;

// a line of command
typedef struct CommandLine
{
    CMD* command;
    int size;     // how many command;
} *CMDL;

// read a line of command
CMDL ReadCommand();
#endif