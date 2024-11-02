#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "quash.h" // Ensure you have your header file included

job_t jobs[MAX_JOBS];
int job_count = 0;

void check_background_jobs() {
    int status;
    for (int i = 0; i < job_count; i++) {
        pid_t result = waitpid(jobs[i].pid, &status, WNOHANG);

        // Check if the process has exited or was killed
        if ((result == jobs[i].pid && WIFEXITED(status)) || result == -1) {
            printf("Completed: [%d] %d %s\n", jobs[i].job_id, jobs[i].pid, jobs[i].command);
            remove_job(jobs[i].job_id);
            i--; // Adjust index after removal
        }
    }
}

void execute_command(char** args) {
    pid_t pid;
    int background = 0;
    char command_line[MAX_LINE];

    // Handle built-in commands first
    if (strcmp(args[0], "echo") == 0) {
        built_in_echo(args);
        return;
    } else if (strcmp(args[0], "export") == 0) {
        built_in_export(args);
        return;
    } else if (strcmp(args[0], "cd") == 0) {
        built_in_cd(args);
        return;
    } else if (strcmp(args[0], "pwd") == 0) {
        built_in_pwd();
        return;
    } else if (strcmp(args[0], "jobs") == 0) {
        print_jobs();
        return;
    } else if (strcmp(args[0], "quit") == 0 || strcmp(args[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(args[0], "kill") == 0) {
        built_in_kill(args);
        return;
    }

    // Check for background execution
    for (int i = 0; args[i] != NULL; i++) {
        if (is_background(args[i])) {
            background = 1;
            args[i] = NULL; // Remove '&' from args
            break;
        }
    }

    // Handle piping commands
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            handle_pipe(args, background); // Pass background flag to handle_pipe
            return; // Exit after handling pipe
        }
    }

    // Prepare the full command line for background jobs
    snprintf(command_line, sizeof(command_line), "%s", args[0]); // Start with the command itself
    for (int i = 1; args[i] != NULL; i++) {
        strcat(command_line, " "); // Add a space before each argument
        strcat(command_line, args[i]); // Append each argument
    }

    if ((pid = fork()) == 0) {
        // Child process
        handle_redirection(args);
        if (execvp(args[0], args) == -1) {
            perror("quash");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("quash");
    } else {
        // Parent process
        if (background) {
            add_job(pid, job_count + 1, command_line);
            printf("Background job started: [%d] %d %s\n", job_count, pid, command_line);
        } else {
            waitpid(pid, NULL, 0);
        }
    }
}



int main() {
    char input[MAX_LINE];
    char* args[MAX_ARGS];

    printf("Welcome...\n");

    while (true) {
        printf("[QUASH]$ ");
        fflush(stdout);

        // Read user input
        if (!fgets(input, MAX_LINE, stdin)) {
            break; // Exit on Ctrl+D
        }

        // Remove newline character
        input[strcspn(input, "\n")] = 0;

        // Parse command into arguments
        parse_command(input, args);
        if (args[0] == NULL) {
            continue; // Empty command
        }

        // Check for finished background jobs after executing command
        check_background_jobs();

        // Execute the parsed command
        execute_command(args);

    }

    return 0;
}
