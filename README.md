# Simple Shell

## Overview

This project implements a simple Unix-like shell in C. The shell supports basic command execution, piping, input/output redirection, and handling of internal commands like `cd` and `exit`. This shell is designed to demonstrate fundamental concepts in operating systems and process management.

## Features

- **Command Execution**: Execute commands entered by the user.
- **Piping**: Use the output of one command as the input to another.
- **Input/Output Redirection**: Redirect input from a file and output to a file.
- **Internal Commands**: Built-in commands like `cd` (change directory) and `exit` (terminate the shell).

## Installation

### On Windows

1. **Install MinGW:**
   - Download and install MinGW from [MinGW SourceForge](https://sourceforge.net/projects/mingw/).
   - During installation, select the `mingw32-gcc-g++` package.
   - Add the MinGW `bin` directory to your system's PATH environment variable.

2. **Compile the Shell:**
   - Open Command Prompt (`cmd`).
   - Navigate to the directory containing `shell.c`:
     ```bash
     cd C:\Users\Admin\Desktop\my_shell
     ```
   - Compile the program:
     ```bash
     gcc shell.c -o shell.exe
     ```

3. **Run the Shell:**
   - Execute the compiled shell:
     ```bash
     shell.exe
     ```

### On Linux or Windows Subsystem for Linux (WSL)

1. **Install WSL:**
   - Open PowerShell as Administrator and run:
     ```bash
     wsl --install
     ```
   - Install a Linux distribution from the Microsoft Store (e.g., Ubuntu).

2. **Install GCC:**
   - Open the WSL terminal.
   - Install GCC:
     ```bash
     sudo apt update
     sudo apt install gcc
     ```

3. **Compile the Shell:**
   - Navigate to the directory containing `shell.c`:
     ```bash
     cd /mnt/c/Users/Admin/Desktop/my_shell
     ```
   - Compile the program:
     ```bash
     gcc shell.c -o shell
     ```

4. **Run the Shell:**
   - Execute the compiled shell:
     ```bash
     ./shell
     ```

## Usage

Once you have compiled and run the shell, you can use it by typing commands at the prompt. Here are some examples:

- **Execute a command**: 
  ```bash
  ls -l
  ```
- **Use piping**: 
  ```bash
  ls -l | grep ".c"
  ```
- **Redirect output to a file**: 
  ```bash
  echo "Hello, World!" > output.txt
  ```
- **Redirect input from a file**: 
  ```bash
  sort < input.txt
  ```

## Contributing

If you have any improvements or bug fixes, please submit a pull request or open an issue. Contributions are welcome!

## Acknowledgments

This project was created to demonstrate basic shell functionalities and to provide hands-on experience with process management and command execution in C.
