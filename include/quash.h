#ifndef QUASH_H
#define QUASH_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>

#define MAX_LINE 1024
#define MAX_ARGS 100
#define MAX_JOBS 100
#define MAX_COMMAND_LENGTH MAX_LINE

typedef struct {
    pid_t pid;
    int job_id;
    char command[MAX_LINE];
} job_t;

void parse_command(char* input, char** args);
void execute_command(char** args);
void handle_background(char** args);
void handle_redirection(char** args);
void handle_pipe(char** args, int background);
void add_job(pid_t pid, int job_id, char* command);
void remove_job(int job_id);
void print_jobs();
bool is_background(char* arg);
void built_in_echo(char **args);
void built_in_export(char **args);
void built_in_cd(char **args);
void built_in_pwd();
void built_in_jobs();
void built_in_kill(char** args);

extern job_t jobs[MAX_JOBS];
extern int job_count;

#endif // QUASH_H
