#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <dirent.h>
#include "builtins.h"

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

// List of builtin commands
char *builtin_str[] = {
    "ls",
    "cd",
    "pwd",
    "help",
    "clear",
    "exit"
};

// List of builtin functions
int (*builtin_func[]) (char **) = {
    &yash_ls,
    &yash_cd,
    &yash_pwd,
    &yash_help,
    &yash_clear,
    &yash_exit
};

int yash_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

bool is_yash_builtin(char *functionName) {
    for (int i = 0; i < yash_num_builtins(); i++) {
        if (strcmp(functionName, builtin_str[i]) == 0) {
            /* if (strcmp(args[0], "help")) { */
            /*     return (*builtin_func[i])(args); */
            /* } */
            /* return (*builtin_func[i])(args); */
            return true;
        }
    }
    return false;
}

int launch_yash_builtin(char ** args) {
    for (int i = 0; i < yash_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return -1;
}

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

int yash_clear(char **args) {
    if (args[1] != NULL) {
        fprintf(stderr, "%s%s\n", RED, "Too many arguments for \"pwd\"");
    } else {
        printf("\e[1;1H\e[2J");
    }
    return 1;
}

int yash_exit(char **args) {
    return 0;
}
