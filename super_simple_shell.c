#include "main.h"

void _putchar(char ch)
{
    write(1, &ch, 1);
}

void env_vars(char **env)
{
    int i = 0;

    while (env[i])
    {
        write(1, env[i], strlen(env[i]));
        _putchar('\n');
        i++;
    }
}

char **parser(char *str, char *delim)
{
	char *str_copy;
	char **str_tokens;
	char *token;
	size_t count, i = 0;

	count = token_count(str, delim);

	str_tokens = malloc(sizeof(char *) * count);
	if (!str_tokens)
		exit(98);
	
	str_copy = malloc(sizeof(char) * (strlen(str) + 1));
	if (!str_copy)
		exit(98);

	strcpy(str_copy, str);
	token = strtok(str_copy, delim);
	while (token != NULL)
	{
		str_tokens[i] = malloc(sizeof(char) * (strlen(token) + 1));
		if (!str_tokens[i])
			exit(98);
		strcpy(str_tokens[i], token);
		token = strtok(NULL, delim);
		i++;
	}
	str_tokens[i] = NULL;

	free(str_copy);
	return (str_tokens);
}

size_t token_count(char *str, char *delim)
{
	size_t count = 0;
	char *str_copy;
	char *token;

	str_copy = malloc(sizeof(char) * (strlen(str) + 1));
	if (!str_copy)
		exit(98);

	strcpy(str_copy, str);
	token = strtok(str_copy, delim);
	while (token != NULL)
	{
		count += 1;
		token = strtok(NULL, delim);
	}
	return (count + 1);
}

/**
 * _realloc - Resizes a previously allocated memory block
 * @ptr: Previously allocated memory block
 * @old_size: Size in bytes of the prev allocated block
 * @new_size: New size to allocate to ptr
 *
 * Return: Pointer to the new resized block, NULL otherwise
 */
void *_realloc(void *ptr, unsigned int old_size, unsigned int new_size)
{
	void *new_ptr;
	unsigned int copy_size;

	if (new_size == 0 && ptr != NULL)
	{
		free(ptr);
		return (NULL);
	}

	if (new_size == old_size)
	{
		return (ptr);
	}

	if (ptr == NULL)
	{
		ptr = malloc(new_size);
		if (!ptr)
			return (NULL);
		return (ptr);
	}

	new_ptr = malloc(new_size);
	if (!new_ptr)
		return (NULL);

	copy_size = (new_size < old_size) ? new_size : old_size;

	memcpy(new_ptr, ptr, copy_size);
	free(ptr);

	return (new_ptr);
}


char *run_prompt(void)
{
	size_t n = 0;
	char *buf = NULL;

	printf("$ ");

	if (getline(&buf, &n, stdin) == -1)
	{
		perror("Error could not read the line on the prompt\n");
		exit(98);
	}

	return (buf);
}

int executor(char **argv)
{
	int i, status;
	pid_t pid;

	pid = fork();
	if (pid == -1)
	{
		perror("Error: Child process can't be created\n");
		return (-1);
	}

	if (pid == 0)
	{
		if (execve(argv[0], argv, NULL) == -1)
			return (-1);
	}

	wait(&status);
	return (0);
}

int check_command_in_curdir(char *command)
{
	struct stat st;

	if (stat(command, &st) == 0)
		return (0);
	else
		return (-1);

}

char **modify_cmd(char **cmd_table, char *path)
{
    unsigned int old_size = (unsigned int) (strlen(cmd_table[0]) + 1);
    unsigned int new_size = (unsigned int) (strlen(path)+ 1);

    cmd_table[0] = _realloc((void *) cmd_table[0], old_size, new_size);
    strcpy(cmd_table[0], path);

    return (cmd_table);

}

char *path_to_executable(char *str_dirs, char *exec)
{
	char *path;
	char **all_dirs = parser(str_dirs, ":");
	int i = 0;
	DIR *dp;

	while (all_dirs[i])
	{
		struct dirent *dir;

		dp = opendir(all_dirs[i]);
		if (!dp)
			return (NULL);
		while ((dir = readdir(dp)) != NULL)
		{
			if (strcmp(dir->d_name, exec) == 0)
			{
				char *path = malloc(sizeof(char) * (strlen(all_dirs[i]) + strlen(exec) + 2));
				if (!path)
					return (NULL);
				strcpy(path, all_dirs[i]);
				strcat(path, "/");
				strcat(path, exec);
				
				closedir(dp);
				return(path);
			}
		}
		i++;
	}

	closedir(dp);
	return (NULL);
}

int run_shell(char **argv, char **env)
{
	int found;
	char *dirs_in_PATH, *path;
	if (strcmp(argv[0], "exit") == 0 && argv[1] != NULL)
	{
		exit(atoi(argv[1]));
	}
	else if(strcmp(argv[0], "exit") == 0)
		exit(0);

	if(strcmp(argv[0], "env") == 0)
		env_vars(env);

	found = check_command_in_curdir(argv[0]);
	if (found == 0)
	{
		if (executor(argv) == 0)
			return (0);
		else
			return (-1);
	}	
	else
	{
		dirs_in_PATH = getenv("PATH");
		path = path_to_executable(dirs_in_PATH, argv[0]);
		if (path)
		{
			argv = modify_cmd(argv, path);
			if (executor(argv) == 0)
				return (0);
			else
				return (-1);
		}
		else
		{
			perror(argv[0]);
			return (-1);
		}
	}
}

int check_and(char *cmd)
{
	int and = 0, i = 0;

	for (i = 0; cmd[i]; i++)
	{
		if (cmd[i] == '&' && cmd[i + 1] == '&')
			and++;
	}	
	return (and);
}

int check_or(char *cmd)
{
	int or = 0, i = 0;

	for (i = 0; cmd[i]; i++)
	{
		if (cmd[i] == '|' && cmd[i + 1] == '|')
			or++;
	}	
	return (or);
}

int check_semi_colon(char *cmd)
{
	int colon = 0, i = 0;

	for (i = 0; cmd[i]; i++)
	{
		if (cmd[i] == ';')
			colon++;
	}	
	return (colon);
}

int execute_and(char *cmd, char **env)
{
	int i = 0, result;
	char **argv, **arr_argv;

	arr_argv = parser(cmd, "&&");

	while (arr_argv[i])
	{
		argv = parser(arr_argv[i], " \n");
		result = run_shell(argv, env);
		if (result == -1)
			return(0);
		i++;
	}
	return (0);
}

int execute_or(char *cmd, char **env)
{
	int i = 0, result;
	char **argv, **arr_argv;

	arr_argv = parser(cmd, "||");

	while (arr_argv[i])
	{
		argv = parser(arr_argv[i], " \n");
		result = run_shell(argv, env);
		if (result == 0)
			return(0);
		i++;
	}
	return (0);
}

int main(int __attribute__((unused))ac, char __attribute__((unused))**av, char **env)
{
	char *str, **argv;
	int chains = 0, or, and, i; 

	while (isatty(0))
	{
		str = run_prompt();
		while (strlen(str) == 1 && str[0] == '\n')
			str = run_prompt();
		chains = check_semi_colon(str);
		if(chains)
		{
			char **arr_argv = parser(str, ";");
			i = 0;
			while (arr_argv[i])
			{
				and = check_and(arr_argv[i]);
				or = check_or(arr_argv[i]);

				if (and)
					execute_and(arr_argv[i], env);
				else if (or)
					execute_or(arr_argv[i], env);
				else
				{
					argv = parser(arr_argv[i], " \n");
					run_shell(argv, env);
				}
				i++;
			}
		}
		else
		{
			and = check_and(str);
			or = check_or(str);

			if (and)
				execute_and(str, env);
			else if (or)
				execute_or(str, env);
			else
			{
				argv = parser(str, " \n");
				run_shell(argv, env);
			}
		}
	}

	return (0);
}

