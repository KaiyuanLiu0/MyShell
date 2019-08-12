#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "prase.h"

// read a line
// precondition: none
// postcondition: return a line of string, NULL for error
static char *ReadLine()
{
    char *buffer = (char *)malloc(sizeof(char) * MAX_LINE);
    size_t n = 0;
    int pos = 0;
    char ch = '\0';

    // cannot allocate space for buffer
    if (buffer == NULL)
    {
        Error("Cannot allocate new space");
        return NULL; // emergency return an empty line
    }
    // read till meeting with a new line or end of file or exceeding the line limit
    while ((ch = fgetc(stdin)) != '\n' && ch != EOF && pos < MAX_LINE - 1)
    {
        buffer[pos++] = ch;
    }
    buffer[pos] = '\0';
    // command too long
    if (pos == MAX_LINE - 1)
    {
        // prompt the error
        Error("Command is too long");
        // free the space and return instantly with NULL
        free(buffer);
        return NULL;
    }
    return buffer;
}

// split a line of string by the delimiter DELIM
// precondition: a line of string
// postcondition: array of string
char **SplitToken(char *line)
{
    char *token = NULL;
    char **tokens = (char **)malloc(sizeof(char *) * MAX_TOKEN);
    int position = 0;

    // cannot allocate space for token
    if (!tokens)
    {
        Error("Cannot allocate new space");
        return NULL; // emergency return
    }

    // delimit
    token = strtok(line, DELIMIT);
    while (token != NULL && position < MAX_TOKEN - 1)
    {
        tokens[position++] = token;
        token = strtok(NULL, DELIMIT);
    }
    tokens[position] = NULL;
    // if too many commands a line
    if (position == MAX_TOKEN - 1)
    {
        Error("Line has too many commands");
        // free(tokens);
        return NULL;
    }
    return tokens;
}

// Initialize a command
// precondition: none
// postcondition: a initialized CMD
static CMD InitializeCommand()
{
    CMD command = NULL;
    command = (CMD)malloc(sizeof(struct Command));
    // if malloc failed
    if (command == NULL)
    {
        Error("Cannot allocate space");
        return NULL;
    }
    // initialize process
    command->cmd = NULL;
    command->argv = NULL;
    command->argc = 0;
    command->in = NULL;
    command->out = NULL;
    command->append = false;
    command->backgroud = false;
}

static CMDL InitializeCommandLine()
{
    CMDL cmdl = NULL;
    cmdl = (CMDL) malloc(sizeof(struct CommandLine));
    cmdl->command = NULL;
    cmdl->size = 0;
}

// split tokens into commands
// precondition: tokens
// postcondition: commands
static CMDL SplitCommand(char **tokens)
{
    int i;
    int position = 0;
    CMD commandList[MAX_TOKEN] = {NULL}; // temporarily store the cmd
    CMD command = NULL;
    char* argumentList[MAX_TOKEN] = {NULL}; // temporarily store the arguments
    char* argument = NULL; // temporarily store the argument
    bool begin = true; // mark whther the beginning of a command
    CMDL cmdl = NULL;
    cmdl = InitializeCommandLine();
    for (position = 0; position < MAX_TOKEN - 1 && tokens[position] != NULL; ++position)
    {
        if (begin)
        {
            begin = false;
            // copy the arguments into the cmd for the fommer command
            if (position != 0)
            {
                commandList[command->argc++] = NULL;
                command->argv = (char**) malloc(sizeof(char*) * command->argc);
                for (i = command->argc - 1; i >= 0; --i)
                {
                    command->argv[i] = argumentList[i];
                }
                commandList[cmdl->size++] = command;
            }
            // create a command
            command = InitializeCommand();
            command->cmd = (char*) malloc(sizeof(char) * (strlen(tokens[position]) + 1));
            strcpy(command->cmd, tokens[position]);
        }
        argument = (char*) malloc(sizeof(char) * (strlen(tokens[position] + 1)));
        strcpy(argument, tokens[position]);
        argumentList[command->argc++] = argument;
    }
    if (command) // if there is any command
    {
        // the last command's arguments
        commandList[command->argc++] = NULL;
        command->argv = (char**) malloc(sizeof(char*) * command->argc);
        for (i = command->argc - 1; i >= 0; --i)
        {
            command->argv[i] = argumentList[i];
        }
        commandList[cmdl->size++] = command;
        cmdl->command = (CMD*) malloc(sizeof(CMD) * cmdl->size);
        for (i = 0; i < cmdl->size; ++i)
        {
            cmdl->command[i] = commandList[i];
        }
    }
    
    return cmdl;
}

CMDL ReadCommand()
{
    int i = 0;
    char *line = NULL;
    char **tokens = NULL;
    CMDL commandList = NULL;
    line = ReadLine();
    tokens = SplitToken(line);
    commandList = SplitCommand(tokens);
    // free no use space
    free(line);
    free(tokens);
    return commandList;
}
