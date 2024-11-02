#include "quash.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

void handle_background(char** args) {
    // Checks for '&' in args
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "&") == 0) {
            args[i] = NULL;
            return;
        }
    }
}

void handle_redirection(char** args) {
    // Implement I/O redirection here
    int i;
    for (i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            args[i] = NULL; // Truncate
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1) {
                perror("quash");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        } else if (strcmp(args[i], ">>") == 0) {
            args[i] = NULL; // Append
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd == -1) {
                perror("quash");
                
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        } else if (strcmp(args[i], "<") == 0) {
            args[i] = NULL; // Read
            int fd = open(args[i + 1], O_RDONLY);
            if (fd == -1) {
                perror("quash");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
    }
}

void handle_pipe(char** args, int background) {
    int fd[2];
    pid_t pid;
    int command_start = 0; // Index to track the start of the current command
    int num_commands = 0;  // Count the number of commands

    // Count the number of commands (separated by pipes)
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            num_commands++;
        }
    }
    num_commands++; // Increment for the last command

    int previous_fd = -1; // Track the read end of the previous pipe

    for (int j = 0; j < num_commands; j++) {
        // Create a pipe for all but the last command
        if (j < num_commands - 1) {
            if (pipe(fd) == -1) {
                perror("quash");
                exit(EXIT_FAILURE);
            }
        }

        // Fork a new process
        if ((pid = fork()) == 0) {
            // Child process
            if (previous_fd != -1) { // If not the first command, read from the previous pipe
                dup2(previous_fd, STDIN_FILENO);
                close(previous_fd);
            }

            if (j < num_commands - 1) { // If not the last command, write to the current pipe
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
            }

            // Null-terminate the current command args
            int current_arg_index = command_start;
            while (args[current_arg_index] != NULL && strcmp(args[current_arg_index], "|") != 0) {
                current_arg_index++;
            }
            args[current_arg_index] = NULL; // Null-terminate the command

            handle_redirection(&args[command_start]); // Handle any redirection for the current command

            execvp(args[command_start], &args[command_start]);
            perror("quash"); // If exec fails
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            perror("quash");
            exit(EXIT_FAILURE);
        }

        // Parent process
        if (j == 0 && background) { // Only add the job for the first command
            int current_job_id = job_count + 1; // Determine the job ID based on the current job count
            add_job(pid, current_job_id, args[command_start]);
            printf("Background job started: [%d] %d %s\n", current_job_id, pid, args[command_start]);
        }

        // Close the write end of the current pipe in the parent
        close(fd[1]);

        // If not the first command, close the read end of the previous pipe
        if (previous_fd != -1) {
            close(previous_fd);
        }

        // Update previous_fd to the read end of the current pipe
        previous_fd = fd[0];

        // Move to the next command in args
        command_start++;
        while (args[command_start] != NULL && strcmp(args[command_start], "|") != 0) {
            command_start++;
        }
        command_start++; // Move past the pipe
    }

    // Close the last read end pipe in the parent process
    if (previous_fd != -1) {
        close(previous_fd);
    }

    // Wait for all child processes to finish
    for (int j = 0; j < num_commands; j++) {
        wait(NULL);
    }
}

void add_job(pid_t pid, int job_id, char* command_line) {
    // Ensure we are within the bounds of the job array
    if (job_count < MAX_JOBS) {
        jobs[job_count].pid = pid;
        jobs[job_count].job_id = job_id;
        snprintf(jobs[job_count].command, sizeof(jobs[job_count].command), "%s", command_line);
        job_count++; // Increment job count after adding the job
    } else {
        fprintf(stderr, "Error: Maximum job limit reached.\n");
    }
}


void remove_job(int job_id) {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].job_id == job_id) {
            for (int j = i; j < job_count - 1; j++) {
                jobs[j] = jobs[j + 1]; // Shift jobs left
            }
            job_count--; // Decrease job count
            return;
        }
    }
}


void print_jobs() {
    for (int i = 0; i < job_count; i++) {
        printf("[%d] %d %s\n", jobs[i].job_id, jobs[i].pid, jobs[i].command);
    }
}

int find_job_by_jobid(int job_id) {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].job_id == job_id) {
            return i;
        }
    }
    return -1;
}

int find_job_by_pid(pid_t pid) {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].pid == pid) {
            return i;
        }
    }
    return -1;
}


bool is_background(char* arg) {
    return strcmp(arg, "&") == 0;
}