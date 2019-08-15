/*
 * file: prase.h
 * The prase used to prase the command
 */
#ifndef PRASE_H_
#define PRASE_H_
#include <stdbool.h>
#include "myshell.h"

#define DELIMIT " \t\r\n\a" // the delimiter which is $IFS

bool reading; // a glocal variable to show whether it's reading from stdin

// struct to store the single command
typedef struct Command
{
    char* cmd; // command
    char** argv; // arguments
    int argc; // argument count 
    char* in;  // redirect in
    char* out; // redirect out
    bool append; // append or replace
    bool background; // whether background
    bool pipe; // whether output to pipe
} *CMD;

// a line of command
typedef struct CommandLine
{
    CMD* command;
    int size;     // how many command;
} *CMDL;

// read a line of command
// Precondition: none
// Postcondition: return a pointer to struct CommandLine
CMDL ReadCommand();
#endif