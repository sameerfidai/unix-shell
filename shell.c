#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

// buffer size
#define BUFF 300

char command[BUFF];
char copy[BUFF];
char home[BUFF];
char tilde[BUFF];
char *token;

// true condition
int homeDir = 1;

/**
 * Displays prompt with current directory.
 */
void displayPrompt();

/**
 * Changes directory based on user input.
 */
void changeDirectory();

/**
 * Redirects output to overwrite a file.
 */
void overwrite();

/**
 * Redirects output to append to a file.
 */
void append();

/**
 * Redirects input from standard input.
 */
void redirectInput();

/**
 * Runs the program with specified command.
 */
void runProgram();

int main()
{
    setbuf(stdout, NULL);

    // forever loop till user specifies exit
    while (1)
    {
        // for file file redirecting
        char *redirectFlag;

        // display prompt
        displayPrompt();

        // get user input command and convert to c-string
        int n = read(0, command, sizeof(command));
        command[n] = '\0';

        // copy user command
        strcpy(copy, command);

        // create/parse tokens
        token = strtok(command, " \t\n<>");

        // no token
        if (!token)
        {
            continue;
        }

        // exit command
        else if (strcmp(token, "exit") == 0)
        {
            exit(0);
        }

        // cd command
        else if (strcmp(token, "cd") == 0)
        {
            changeDirectory();
        }

        // redirect input
        else if ((redirectFlag = strchr(copy, '<')))
        {
            redirectInput();
        }

        // redirect output
        else if ((redirectFlag = strchr(copy, '>')))
        {
            // look for second '>'
            if (*(redirectFlag + 1) == '>')
            {
                // append to file
                append();
            }
            else
            {
                // overwrite file
                overwrite();
            }
        }

        // handle unix command
        else
        {
            runProgram();
        }
    }
    return 0;
}

void displayPrompt()
{
    char cwd[BUFF];

    // checks for home directory
    if (homeDir == 1)
    {
        strcat(home, getenv("HOME"));
        homeDir = 0;
    }

    int n = 0;
    while (home[n] != '\0')
    {
        n++;
    }

    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        if (strncmp(home, cwd, n) == 0 && strcmp(home, cwd) <= 0)
        {
            tilde[0] = '~';
            for (int i = 1; cwd[n] != '\0'; i++, n++)
            {
                tilde[i] = cwd[n];
            }
            setbuf(stdout, NULL);
            printf("1730sh:%s$ ", tilde);
            memset(tilde, 0, strlen(tilde));
        }

        else
        {
            setbuf(stdout, NULL);
            printf("1730sh:%s$ ", cwd);
            memset(cwd, 0, strlen(cwd));
        }
    }

    else
    {
        perror("getcwd() error");
    }
}

void changeDirectory()
{
    // get home directory
    char *homeD = getenv("HOME");

    // get next directory
    token = strtok(NULL, " \t\n<>");

    // change to home directory
    if (token == NULL)
    {
        if (homeD == NULL)
        {
            setbuf(stdout, NULL);
            printf("ERROR: HOME environment variable not set.\n");
            return;
        }
        if (chdir(homeD) == -1)
        {
            perror("Error");
        }
        return;
    }

    // handle '~' case
    if (token[0] == '~')
    {
        if (homeD == NULL)
        {
            setbuf(stdout, NULL);
            printf("ERROR: HOME environment variable not set.\n");
            return;
        }
        char directory[500];
        strcpy(directory, homeD);
        token = strcat(directory, &token[1]);
    }

    if (chdir(token) == -1)
    {
        perror("Error");
    }
}

void overwrite()
{
    char *arguments[BUFF];
    pid_t pid = fork();
    int argc = 0;
    int status = 0;

    if (pid == -1)
    {
        perror("Error");
        exit(EXIT_FAILURE);
    }

    // child process
    else if (pid == 0)
    {
        arguments[argc] = 0;

        // process tokens
        while (token != NULL)
        {
            token = strtok(NULL, " \t\n<>");
            argc++;
            arguments[argc] = token;
        }

        // name of old file
        char *oldFile = arguments[argc - 2];

        // name of new file (overwrite to)
        char *newFile = arguments[argc - 1];

        // open old file to read only
        int oldFd = open(oldFile, O_RDONLY);

        // open new file to write only and truncate
        int newFd = open(newFile, O_CREAT | O_WRONLY | O_TRUNC, 0777);

        // temporary file to create and write to
        int tempFd = open("temp.txt", O_CREAT | O_WRONLY);

        int n;
        char z;

        // redirect tempFd to newFd (new file)
        dup2(newFd, tempFd);
        while ((n = read(oldFd, &z, sizeof(z))) > 0)
        {
            write(tempFd, &z, sizeof(z));
        }

        // remove temp file
        remove("temp.txt");

        if (strcmp(token, "exit") == 0)
        {
            exit(0);
        }
    }

    // parent process
    else
    {
        if (wait(&status) == -1)
        {
            perror("Error");
            return;
        }
    }
}

void append()
{
    char *arguments[BUFF];
    pid_t pid = fork();
    int argc = 0;
    int status = 0;

    if (pid == -1)
    {
        perror("Error");
        exit(EXIT_FAILURE);
    }

    // child process
    else if (pid == 0)
    {
        arguments[argc] = 0;

        // process tokens
        while (token != NULL)
        {
            token = strtok(NULL, " \t\n<>");
            argc++;
            arguments[argc] = token;
        }

        // name of old file
        char *oldFile = arguments[argc - 2];

        // name of new file (overwrite to)
        char *newFile = arguments[argc - 1];

        // open old file to read only
        int oldFd = open(oldFile, O_RDONLY);

        // open new file to write only
        int newFd = open(newFile, O_CREAT | O_WRONLY | O_APPEND, 0777);

        // temporary file to create and write to
        int tempFd = open("temp.txt", O_CREAT | O_WRONLY);

        int n;
        char z;

        // redirect tempFd to newFd (new file)
        dup2(newFd, tempFd);
        while ((n = read(oldFd, &z, sizeof(z))) > 0)
        {
            write(tempFd, &z, sizeof(z));
        }

        // remove temp file
        remove("temp.txt");

        if (strcmp(token, "exit") == 0)
        {
            exit(0);
        }
    }

    // parent process
    else
    {
        if (wait(&status) == -1)
        {
            perror("Error");
            return;
        }
    }
}

void redirectInput()
{
    char *arguments[BUFF];
    pid_t pid = fork();
    int argc = 0;
    int status = 0;

    if (pid == -1)
    {
        perror("Error");
        exit(EXIT_FAILURE);
    }

    // child process
    else if (pid == 0)
    {
        arguments[argc] = 0;

        // process tokens
        while (token != NULL)
        {
            token = strtok(NULL, " \t\n<>");
            argc++;
            arguments[argc] = token;
        }

        // name of input file
        char *file = arguments[argc - 1];

        // open file to read only
        int oldFd = open(file, O_RDONLY);

        int n;
        char z;

        // redirect tempFd to newFd (new file)
        dup2(1, 1);

        while ((n = read(oldFd, &z, sizeof(z))) > 0)
        {
            write(1, &z, sizeof(z));
        }

        if (strcmp(token, "exit") == 0)
        {
            exit(0);
        }
    }

    // parent process
    else
    {
        if (wait(&status) == -1)
        {
            perror("Error");
            return;
        }
    }
}

void runProgram()
{
    char *arguments[BUFF];
    pid_t pid = fork();
    int argc = 0;
    int status = 0;

    // fork error
    if (pid == -1)
    {
        perror("Error");
        exit(EXIT_FAILURE);
    }

    // child process
    else if (pid == 0)
    {
        arguments[argc] = token;

        while (token != NULL)
        {
            token = strtok(NULL, " \t\n<>");
            argc++;
            arguments[argc] = token;
        }

        // run command
        if ((execvp(arguments[0], arguments) == -1))
        {
            // invalid command error
            setbuf(stdout, NULL);
            printf("bash: %s: command not found...\n", command);
        }

        exit(EXIT_SUCCESS);
    }

    // parent process
    else
    {
        if (wait(&status) == -1)
        {
            perror("Error");
            return;
        }
    }
}