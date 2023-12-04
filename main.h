#ifndef MAIN_H
#define MAIN_H

#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>


size_t token_count(char *str, char *delim);
char **parser(char *str, char *delim);
char *run_prompt(void);
int executor(char **argv);
char *path_to_executable(char *str_dirs, char *exec);
int check_command_in_curdir(char *command);
void *_realloc(void *ptr, unsigned int old_size, unsigned int new_size);
char **modify_cmd(char **cmd_table, char *path);
void _putchar(char ch);
void env_vars(char **env);
int run_shell(char **argv, char **env);
int check_and(char *cmd);
int check_or(char *cmd);
int check_semi_colon(char *cmd);

#endif
