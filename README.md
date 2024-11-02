# Quash Shell

Quash (Quite a Shell) is a Unix-based custom shell implementation written in. It is designed to mimic features of popular shells like `bash` and `csh`, offering command execution, environment variable handling, background and foreground process management, I/O redirection, and piping.

## Features
- **Executable Commands**: Run commands with parameters; search `PATH` if not provided as absolute or relative paths.
- **I/O Redirection**: Redirect input/output to/from files using `<`, `>`, and `>>`.
- **Process Control**: Support for foreground and background executions (`&` for background).
- **Pipes**: Enable command chaining with pipes (`|`) for inter-process communication.
- **Built-in Commands**: Includes `echo`, `export`, `cd`, `pwd`, `jobs`, and `kill`.
- **Environment Variable Expansion**: Expands variables like `$HOME` and `$PWD`.

## System Calls and Restrictions
Quash relies on system calls like `fork`, `execvp`, `waitpid`, `pipe`, and `dup2`, with a strict prohibition on `system()` calls.

## Getting Started
1. Clone the repository.
2. Run `make` to build the program.
3. Execute `./quash` to start the shell.
