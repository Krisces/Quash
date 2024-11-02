#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#define MAX_ARGS 100  // Maximum number of arguments

void parse_command(char* input, char** args) {
    int index = 0; // Index for the args array
    bool in_quotes = false; // Flag to track if we're inside quotes
    char* token_start = NULL; // Start of the current token
    char* token = NULL; // To store the current token

    for (char* p = input; *p != '\0'; p++) {
        // Handle quotes
        if (*p == '"' || *p == '\'') {
            in_quotes = !in_quotes; // Toggle the in_quotes flag
            if (in_quotes) {
                token_start = p + 1; // Start after the quote
            } else {
                // We're closing a quoted section
                size_t token_length = p - token_start; // Length of the token
                if (token_length > 0) {
                    token = (char*)malloc(token_length + 1);
                    strncpy(token, token_start, token_length);
                    token[token_length] = '\0'; // Null-terminate
                    args[index++] = token; // Store the token
                }
                token_start = NULL; // Reset token start
            }
        } else if (*p == ' ' && !in_quotes) {
            // Handle whitespace outside of quotes
            if (token_start) {
                size_t token_length = p - token_start; // Length of the token
                if (token_length > 0) {
                    token = (char*)malloc(token_length + 1);
                    strncpy(token, token_start, token_length);
                    token[token_length] = '\0'; // Null-terminate
                    args[index++] = token; // Store the token
                }
                token_start = NULL; // Reset token start
            }
        } else if (token_start == NULL) {
            // Mark the start of a new token if we aren't in quotes
            token_start = p;
        }
    }

    // Handle the last token if it exists
    if (token_start) {
        size_t token_length = strlen(token_start); // Length of the token
        if (token_length > 0) {
            token = (char*)malloc(token_length + 1);
            strcpy(token, token_start);
            args[index++] = token; // Store the token
        }
    }

    // Handle escape sequences in the arguments
    for (int i = 0; i < index; i++) {
        char* arg = args[i];
        char expanded_arg[1024] = ""; // Buffer to hold expanded text
        int j = 0; // Index for expanded_arg

        for (char* p = arg; *p != '\0'; p++) {
            if (*p == '\\') {
                p++; // Move to the next character
                if (*p == 'n') {
                    expanded_arg[j++] = '\n'; // Replace \n with a newline character
                } else if (*p == '\\') {
                    expanded_arg[j++] = '\\'; // Keep single backslash
                } else {
                    expanded_arg[j++] = *p; // Keep the character after the backslash
                }
            } else {
                expanded_arg[j++] = *p; // Normal character
            }
        }
        expanded_arg[j] = '\0'; // Null-terminate the expanded argument
        free(arg); // Free the old arg
        args[i] = strdup(expanded_arg); // Store the expanded argument
    }

    // Null-terminate the argument list
    args[index] = NULL;
}
