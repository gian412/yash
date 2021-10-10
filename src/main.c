#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include "builtins.h"

// Global constants
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

// Colors
#define NORMAL_COLOR "\x1B[0m"
#define BLACK "\x1b[30m"
#define RED "\x1b[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define BLUE "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN "\x1B[36m"
#define WHITE "\x1B[37m"

// Function declaration for runtime commands
void yash_loop(void);
char *yash_readLine(void);
char **yash_splitLine(char * line);
int yash_execute();
int yash_launch(char **args);

// Main
int main(int argc, char **argv) {
    // Load config file, if any

    // Run command loop
    yash_loop();

    // Perform any shutdown/cleanup

    return EXIT_SUCCESS;
}

// Function implementation
void yash_loop(void) {
    char *line;
    char **args;
    int status;

    do {
        printf("%s%s%s ", MAGENTA, ">", NORMAL_COLOR);
        line = yash_readLine();
        args = yash_splitLine(line);
        status = yash_execute(args);

        free(line);
        free(args);
    } while (status);
}

#define YASH_RL_BUFSIZE 1024
char *yash_readLine(void) {
    int position = 0;
    int c;
    int bufSize = YASH_RL_BUFSIZE;
    char *buffer = malloc(sizeof(char) * bufSize);

    if (!buffer) {
        fprintf(stderr, "yash: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Read a character
        c = getchar();

        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;

        // If we have exceeded the buffer, reallocate
        if (position >= bufSize) {
            bufSize += YASH_RL_BUFSIZE;
            buffer = realloc(buffer, bufSize);
            if (!buffer) {
                fprintf(stderr, "%s%s\n", RED, "yash: allocation error");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define YASH_TOK_BUFSIZE 64
#define YASH_TOK_DELIM " \t\r\n\a"
char **yash_splitLine(char *line) {
    int position = 0;
    int bufSize = YASH_TOK_BUFSIZE;
    char **tokens = malloc(bufSize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "%s%s\n", RED, "yash: allocation error");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, YASH_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufSize) {
            bufSize += YASH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufSize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "%s%s\n", RED, "yash: allocation error");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, YASH_TOK_DELIM);
    }

    tokens[position] = NULL;
    return tokens;
}

int yash_launch(char **args) {
    pid_t pid;
    pid_t wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("yash");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("yash");
    } else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int yash_execute(char **args) {
    int i;

    if (args[0] == NULL) {
        // An empty command was entered
        return 1;
    }

    if (is_yash_builtin(args[0])) {
        return launch_yash_builtin(args);
    }
    return yash_launch(args);
}

