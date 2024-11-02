#include "quash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

void built_in_echo(char **args) {
    int output_redirected = 0;
    FILE *output_file = NULL;
    int new_line = 1; // Default to printing a new line at the end

    // Check for output redirection and handle new line option
    for (int i = 1; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            output_redirected = 1;
            args[i] = NULL; // Terminate args at '>'
            output_file = fopen(args[i + 1], "w"); // Open the file for writing
            if (output_file == NULL) {
                perror("quash");
                return;
            }
            break;
        }
        if (strcmp(args[i], "-n") == 0) {
            new_line = 0;
            args[i] = NULL; // Remove '-n' from args
        }
    }

    // Print the arguments with environment variable expansion and escape sequences handling
    for (int i = 1; args[i] != NULL; i++) {
        char *arg = args[i];
        char expanded_arg[1024] = "";  // Buffer to hold expanded text

        while (*arg != '\0') {
            if (*arg == '$') {
                // Move to the variable name (skip the '$')
                arg++;
                char var_name[100] = "";
                int j = 0;

                // Extract variable name
                while (*arg && (isalnum(*arg) || *arg == '_')) {
                    var_name[j++] = *arg++;
                }
                var_name[j] = '\0';

                // Get environment variable value
                char *var_value = getenv(var_name);
                if (var_value) {
                    strcat(expanded_arg, var_value);  // Append variable value
                } else {
                    strcat(expanded_arg, "$");  // Keep the '$' if no value is found
                    strcat(expanded_arg, var_name);
                }
             } else if (*arg == '\\') {
                // Handle escape sequences
                arg++;
                if (*arg == 'n') {
                    fputc('\n', output_redirected ? output_file : stdout);
                } else if (*arg == '\\') {
                    fputc('\\', output_redirected ? output_file : stdout);
                } else {
                    fputc('\\', output_redirected ? output_file : stdout); // Print the backslash if next character is not recognized
                    fputc(*arg, output_redirected ? output_file : stdout); // Print the next character
                }
            } else {
                fputc(*arg, output_redirected ? output_file : stdout);
            }
            arg++;
        }

        output_redirected ? fputs(expanded_arg, output_file) : fputs(expanded_arg, stdout);

        if (args[i + 1] != NULL) {
            output_redirected ? fputs(" ", output_file) : fputs(" ", stdout);
        }
    }

    if (new_line) {
        output_redirected ? fputc('\n', output_file) : fputc('\n', stdout);
    }

    if (output_redirected) {
        fclose(output_file);
    }
}


// Set or print environment variables
void built_in_export(char **args) {
    if (args[1] == NULL) {
        // Print all environment variables
        extern char **environ;
        for (int i = 0; environ[i]; i++) {
            printf("%s\n", environ[i]);
        }
    } else {
        // Set the environment variable
        char *equals = strchr(args[1], '=');
        if (equals != NULL) {
            *equals = '\0'; // Split key and value
            char *value = equals + 1;

            // Expand any environment variables in the value
            char expanded_value[1024]; // Buffer for the expanded value
            char *token = strtok(value, ":"); // Split by colon for PATH
            int first = 1; // To manage leading separators

            expanded_value[0] = '\0'; // Initialize the buffer

            while (token != NULL) {
                // Check if token is an environment variable
                if (token[0] == '$') {
                    char *env_var = getenv(token + 1); // Get the value
                    if (env_var != NULL) {
                        if (!first) {
                            strcat(expanded_value, ":"); // Add colon separator
                        }
                        strcat(expanded_value, env_var);
                        first = 0;
                    }
                } else {
                    if (!first) {
                        strcat(expanded_value, ":"); // Add colon separator
                    }
                    strcat(expanded_value, token); // Add the token
                    first = 0;
                }
                token = strtok(NULL, ":"); // Continue splitting
            }

            // Set the environment variable with the expanded value
            setenv(args[1], expanded_value, 1);
        }
    }
}

// Change the current working directory
void built_in_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "quash: cd: missing argument\n");
    } else {
        char *path = args[1];

        // Check if the path contains a variable (like $HOME)
        if (path[0] == '$') {
            char *env_var = getenv(path + 1); // Get the value of the variable
            if (env_var != NULL) {
                path = env_var; // Replace the variable with its value
            } else {
                fprintf(stderr, "quash: cd: %s: No such file or directory\n", path);
                return;
            }
        }

        // Attempt to change the directory
        if (chdir(path) != 0) {
            perror("quash");
        } else {
            // Update PWD environment variable after successful directory change
            char *new_pwd = getcwd(NULL, 0); // Get the current working directory
            if (new_pwd != NULL) {
                setenv("PWD", new_pwd, 1); // Update the PWD variable
                free(new_pwd); // Free the allocated memory
            }
        }
    }
}

// Print the current working directory
void built_in_pwd() {
    char cwd[MAX_LINE];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd); // Print the actual working directory
    } else {
        perror("quash");
    }
}

// Print currently running background jobs
void built_in_jobs() {
    print_jobs();
}

void built_in_kill(char** args) {
    int job_id = -1;
    pid_t pid = -1;
    char command[MAX_LINE] = "";

    // Check if the argument is a job ID
    if (args[1][0] == '%') {
        job_id = atoi(&args[1][1]); // Extract the job ID
        // Find the PID corresponding to the JOBID
        for (int i = 0; i < job_count; i++) {
            if (jobs[i].job_id == job_id) {
                pid = jobs[i].pid;
                strncpy(command, jobs[i].command, sizeof(command));
                break;
            }
        }
    } else {
        // Otherwise, treat the argument as a PID
        pid = atoi(args[1]);
        // Check if the PID exists in the jobs list
        for (int i = 0; i < job_count; i++) {
            if (jobs[i].pid == pid) {
                job_id = jobs[i].job_id; // Retrieve the JOBID
                strncpy(command, jobs[i].command, sizeof(command));
                break;
            }
        }
    }

    // Now we have both PID and JOBID if applicable
    if (pid > 0) {
        if (kill(pid, SIGKILL) == 0) {
            printf("Killed process: [%d] %d %s\n", job_id, pid, command);
            // Remove the job by JOBID if it was found
            if (job_id != -1) {
                remove_job(job_id); // Remove the job from the list
            }
        } else {
            perror("kill");
        }
    } else {
        printf("No such job or process: %s\n", args[1]);
    }
}
