#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 1024  // Maximum length of a command line
#define MAX_ARGS 128   // Maximum number of arguments per command

// Function declarations
void execute(char **args, int background);
void parse_command(char *line, char **args);
int is_builtin(char **args);
void handle_redirection(char **args);
void handle_piping(char *line);

// Built-in command handlers
void cd_command(char **args);
void exit_command();

int main() {
    char line[MAX_LINE];  // Buffer to store the input command line
    char *args[MAX_ARGS]; // Array to store command arguments
    int background = 0;   // Flag to determine if the command should run in the background

    // Main loop of the shell
    while (1) {
        printf("shell> "); // Display the prompt
        if (!fgets(line, MAX_LINE, stdin)) break; // Read input from the user

        // If the input is empty (user just pressed Enter), skip processing
        if (strcmp(line, "\n") == 0) continue;

        // Remove the newline character from the end of the input
        line[strcspn(line, "\n")] = 0;

        // Check if the command contains a pipe symbol
        if (strchr(line, '|')) {
            handle_piping(line); // Handle piping if present
            continue;
        }

        // Parse the command line into arguments
        parse_command(line, args);

        // Check if the command should run in the background
        background = 0;
        if (args[0] && strcmp(args[strlen(args[0]) - 1], "&") == 0) {
            background = 1; // Set background flag
            args[strlen(args[0]) - 1] = NULL; // Remove "&" from the arguments
        }

        // Check if the command is a built-in command (like "cd" or "exit")
        if (is_builtin(args)) continue;

        // Handle input and output redirection
        handle_redirection(args);

        // Execute the command
        execute(args, background);
    }

    return 0;
}

// Function to parse the command line into individual arguments
void parse_command(char *line, char **args) {
    int i = 0;
    args[i] = strtok(line, " "); // Split the line into tokens separated by spaces
    while (args[i] != NULL) { // Continue until no more tokens
        i++;
        args[i] = strtok(NULL, " "); // Get the next token
    }
}

// Function to execute the command
void execute(char **args, int background) {
    pid_t pid = fork(); // Create a new process
    if (pid == 0) { // This is the child process
        // Try to execute the command
        if (execvp(args[0], args) == -1) {
            perror("shell"); // Print an error message if execvp fails
        }
        exit(EXIT_FAILURE); // Exit the child process if execvp fails
    } else if (pid > 0) { // This is the parent process
        if (!background) wait(NULL); // Wait for the child process to finish if not running in the background
    } else {
        perror("fork"); // Print an error message if fork fails
    }
}

// Function to check if the command is a built-in command and handle it
int is_builtin(char **args) {
    if (strcmp(args[0], "cd") == 0) {
        cd_command(args); // Handle the "cd" command
        return 1;
    } else if (strcmp(args[0], "exit") == 0) {
        exit_command(); // Handle the "exit" command
        return 1;
    }
    return 0; // Return 0 if the command is not built-in
}

// Handle the "cd" (change directory) command
void cd_command(char **args) {
    if (args[1] == NULL) { // Check if an argument was provided
        fprintf(stderr, "shell: expected argument to \"cd\"\n"); // Print an error message if no argument
    } else {
        if (chdir(args[1]) != 0) { // Try to change the directory
            perror("shell"); // Print an error message if chdir fails
        }
    }
}

// Handle the "exit" command
void exit_command() {
    exit(EXIT_SUCCESS); // Exit the shell with success status
}

// Function to handle input and output redirection
void handle_redirection(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) { // Check for output redirection
            int fd = open(args[i + 1], O_CREAT | O_WRONLY | O_TRUNC, 0644); // Open file for writing
            if (fd < 0) {
                perror("shell"); // Print an error message if open fails
                return;
            }
            dup2(fd, STDOUT_FILENO); // Redirect stdout to the file
            close(fd); // Close the file descriptor
            args[i] = NULL; // Remove the redirection symbols from the arguments
        } else if (strcmp(args[i], "<") == 0) { // Check for input redirection
            int fd = open(args[i + 1], O_RDONLY); // Open file for reading
            if (fd < 0) {
                perror("shell"); // Print an error message if open fails
                return;
            }
            dup2(fd, STDIN_FILENO); // Redirect stdin to the file
            close(fd); // Close the file descriptor
            args[i] = NULL; // Remove the redirection symbols from the arguments
        }
    }
}

// Function to handle piping between commands
void handle_piping(char *line) {
    int pipefd[2]; // Array to hold pipe file descriptors
    pid_t pid; // Process ID
    char *cmd1[MAX_ARGS]; // Arguments for the first command
    char *cmd2[MAX_ARGS]; // Arguments for the second command

    // Split the command into two parts using the pipe symbol
    char *left = strtok(line, "|");
    char *right = strtok(NULL, "|");

    // Parse the commands into arguments
    parse_command(left, cmd1);
    parse_command(right, cmd2);

    // Create a pipe
    if (pipe(pipefd) == -1) {
        perror("pipe"); // Print an error message if pipe fails
        return;
    }

    // Fork the first process
    if ((pid = fork()) == 0) {
        // Child process 1: Writes to the pipe
        dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to the pipe
        close(pipefd[0]); // Close unused read end
        close(pipefd[1]);
        execvp(cmd1[0], cmd1); // Execute the first command
        perror("execvp"); // Print an error message if execvp fails
        exit(EXIT_FAILURE); // Exit the child process if execvp fails
    }

    // Fork the second process
    if ((pid = fork()) == 0) {
        // Child process 2: Reads from the pipe
        dup2(pipefd[0], STDIN_FILENO); // Redirect stdin to the pipe
        close(pipefd[1]); // Close unused write end
        close(pipefd[0]);
        execvp(cmd2[0], cmd2); // Execute the second command
        perror("execvp"); // Print an error message if execvp fails
        exit(EXIT_FAILURE); // Exit the child process if execvp fails
    }

    // Parent process: Close both ends of the pipe and wait for children
    close(pipefd[0]);
    close(pipefd[1]);
    wait(NULL); // Wait for the first child process to finish
    wait(NULL); // Wait for the second child process to finish
}
