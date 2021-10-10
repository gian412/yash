#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>

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

// Function declaration for builtin shell commands
int yash_ls(char **args);
int yash_cd(char **args);
int yash_pwd(char **args);
int yash_help(char **args);
int yash_exit(char **args);

// List of builtin commands
char *builtin_str[] = {
    "ls",
    "cd",
    "pwd",
    "help",
    "exit"
};

// List of builtin functions
int (*builtin_func[]) (char **) = {
    &yash_ls,
    &yash_cd,
    &yash_pwd,
    &yash_help,
    &yash_exit
};

int yash_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

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

    for (i = 0; i < yash_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return yash_launch(args);
}

// Builtin function implementation
int yash_ls(char **args) {
    DIR *d;
    if (args[1] != NULL) {
        d = opendir(args[1]);
    } else {
        d = opendir(".");
    }
    struct dirent *dir;
    if (d==NULL) return 1;
    while ((dir = readdir(d)) != NULL) {
        if (dir->d_type != DT_DIR) {
            printf("%s%s\n", BLUE, dir->d_name);
        }
        else {
            if (dir->d_type == DT_DIR && strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
                printf("%s%s\n", GREEN, dir->d_name);
            }
        }
    }
    closedir(d);
    return 1;
}

int yash_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "%s%s\n", RED, "yash: expected argument to \"cd\"");
    } else {
        if (chdir(args[1]) != 0) {
            perror("yash");
        }
    }
    return 1;
}

int yash_pwd(char **args) {
    char s[100];
    if (args[1] != NULL) {
        fprintf(stderr, "%s%s\n", RED, "Too many arguments for \"pwd\"");
    } else {
        printf("%s%s\n", NORMAL_COLOR, getcwd(s, 100));
    }
    return 1;
}

int yash_help(char **args) {
    int i;
    printf("%s%s\n", CYAN, "Gianluca Regis' YASH");
    printf("%s%s\n", NORMAL_COLOR, "Type program names and arguments, and hit enter.");
    printf("%s%s\n", NORMAL_COLOR, "The following are built in:");

    for (i = 0; i < yash_num_builtins(); i++) {
        printf("  %s%s\n", BLUE,  builtin_str[i]);
    }

    printf("%s%s\n", NORMAL_COLOR, "Use the man command for information on other programs.");
    return 1;
}

int yash_exit(char **args) {
    return 0;
}

