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
    char line[MAX_LINE];
    char *args[MAX_ARGS];
    int background = 0;

    while (1) {
        printf("shell> ");
        if (!fgets(line, MAX_LINE, stdin)) break; // Read input

        // Check if input was empty
        if (strcmp(line, "\n") == 0) continue;

        // Remove the newline character at the end
        line[strcspn(line, "\n")] = 0;

        // Handle piping if present
        if (strchr(line, '|')) {
            handle_piping(line);
            continue;
        }

        parse_command(line, args);

        // Check for background execution
        background = 0;
        if (args[0] && strcmp(args[strlen(args[0]) - 1], "&") == 0) {
            background = 1;
            args[strlen(args[0]) - 1] = NULL;
        }

        // Check for built-in commands
        if (is_builtin(args)) continue;

        // Handle redirection if present
        handle_redirection(args);

        // Execute the command
        execute(args, background);
    }

    return 0;
}

// Function to parse the command line into arguments
void parse_command(char *line, char **args) {
    int i = 0;
    args[i] = strtok(line, " ");
    while (args[i] != NULL) {
        i++;
        args[i] = strtok(NULL, " ");
    }
}

// Function to execute commands
void execute(char **args, int background) {
    pid_t pid = fork();
    if (pid == 0) { // Child process
        if (execvp(args[0], args) == -1) {
            perror("shell");
        }
        exit(EXIT_FAILURE);
    } else if (pid > 0) { // Parent process
        if (!background) wait(NULL); // Wait for the child process unless it's in the background
    } else {
        perror("fork");
    }
}

// Function to check and handle built-in commands
int is_builtin(char **args) {
    if (strcmp(args[0], "cd") == 0) {
        cd_command(args);
        return 1;
    } else if (strcmp(args[0], "exit") == 0) {
        exit_command();
        return 1;
    }
    return 0;
}

// Change directory built-in command
void cd_command(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "shell: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("shell");
        }
    }
}

// Exit built-in command
void exit_command() {
    exit(EXIT_SUCCESS);
}

// Function to handle I/O redirection
void handle_redirection(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) { // Output redirection
            int fd = open(args[i + 1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
            if (fd < 0) {
                perror("shell");
                return;
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[i] = NULL; // Remove the redirection symbols from the arguments
        } else if (strcmp(args[i], "<") == 0) { // Input redirection
            int fd = open(args[i + 1], O_RDONLY);
            if (fd < 0) {
                perror("shell");
                return;
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
            args[i] = NULL;
        }
    }
}

// Function to handle piping
void handle_piping(char *line) {
    int pipefd[2];
    pid_t pid;
    char *cmd1[MAX_ARGS];
    char *cmd2[MAX_ARGS];

    // Split the command into two parts using the pipe symbol
    char *left = strtok(line, "|");
    char *right = strtok(NULL, "|");

    // Parse the commands
    parse_command(left, cmd1);
    parse_command(right, cmd2);

    // Create a pipe
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return;
    }

    // Fork the first process
    if ((pid = fork()) == 0) {
        // Child process 1: Writes to the pipe
        dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to the pipe
        close(pipefd[0]); // Close unused read end
        close(pipefd[1]);
        execvp(cmd1[0], cmd1);
        perror("execvp");
        exit(EXIT_FAILURE);
    }

    // Fork the second process
    if ((pid = fork()) == 0) {
        // Child process 2: Reads from the pipe
        dup2(pipefd[0], STDIN_FILENO); // Redirect stdin to the pipe
        close(pipefd[1]); // Close unused write end
        close(pipefd[0]);
        execvp(cmd2[0], cmd2);
        perror("execvp");
        exit(EXIT_FAILURE);
    }

    // Parent process: Close both ends of the pipe and wait for children
    close(pipefd[0]);
    close(pipefd[1]);
    wait(NULL);
    wait(NULL);
}
