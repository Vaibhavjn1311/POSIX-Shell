# Custom Shell Program

This repository contains a custom shell program implemented in C++. It includes basic shell functionalities such as command execution, input/output redirection, piping, and built-in commands like `cd`, `pwd`, `echo`, `ls`, and `pinfo`. The shell also supports history management and file search within the current directory.

## Features

- **Command Execution**: Executes standard shell commands using `execvp`.
- **Input/Output Redirection**: Supports `>`, `<`, and `>>` for redirection.
- **Piping**: Handles multiple commands connected through pipes (`|`).
- **Built-in Commands**:
  - `cd [directory]`: Change the current working directory.
  - `pwd`: Print the current working directory.
  - `echo [text]`: Print the provided text to the terminal.
  - `ls [options] [directory]`: List files in a directory with optional flags:
    - `-a`: Show all files, including hidden ones.
    - `-l`: Show detailed file information.
  - `pinfo [pid]`: Display process information for a given PID or the current process.
  - `search [file]`: Search for a file in the current directory and its subdirectories.
  - `history`: Show the last 10 commands from the history.
- **History Management**: Keeps track of the last 20 commands.

## Compilation and Execution


To compile the program, use the following command:

```bash
g++ -o 2024201023_Assignment2 2024201023_Assignment2.cpp
```

To run the program, use the following command:

```bash
./2024201023_Assignment2
```
