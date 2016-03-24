#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024
#define MAX_TOKEN 128

#define TRUE      1
#define FALSE     0

#define STDIN     0
#define STDOUT    1
#define STDERR    2

void printPrompt();
char * readLine();
char ** getTokens(char *line);
int execute(char **args);

int cdCommand(char **args);
int pwdCommand();
int echoCommand(char **args);
int setCommand(char **args);
int helpCommand();

int launch(char **args);

int main(int argc, char ** argv, char **envp) {
  
  char *line = NULL;
  char **tokens = NULL;
  int status = 0;

  do {
    printPrompt();
    
    line = readLine();
    tokens = getTokens(line);
    status = execute(tokens);

    free(line);
    free(tokens);

  // Execute the command, handling built-in commands separately 
  // Just echo the command line for now
  // write(1, cmd, strnlen(cmd, MAX_INPUT));
  } while(status);

  return 0;
}

void printPrompt() {
  char *prompt = "320sh> ";
  char *cwd = getcwd(NULL, 0);
  
  cwd = strrchr(cwd, '/');

  write(STDOUT, "[", 1);
  write(STDOUT, cwd, strlen(cwd));
  write(STDOUT, "] ", 2);
  write(STDOUT, prompt, strlen(prompt));
}

char * readLine() {
  char *line = NULL;
  size_t len = 0;

  getline(&line, &len, stdin);
  return line;
}

char ** getTokens(char *line) {
  int pos = 0;
  char **tokens = malloc(sizeof(char *) * MAX_TOKEN);
  char *token = NULL;
  const char *delimeter = " \n\t\f\r\v";

  token = strtok(line, delimeter);

  while(token != NULL) {
    tokens[pos] = token;
    pos++;
    token = strtok(NULL, delimeter);
  }

  tokens[pos] = NULL;
  return tokens;
}

int execute(char **args) {

  if(args[0] == NULL) {
    return TRUE;
  }

  if(strcmp(args[0], "cd") == 0) {
    return cdCommand(args);
  } else if(strcmp(args[0], "pwd") == 0) {
    return pwdCommand();
  } else if(strcmp(args[0], "echo") == 0) {
    return echoCommand(args);
  } else if(strcmp(args[0], "set") == 0) {
    return setCommand(args);
  } else if(strcmp(args[0], "help") == 0) {
    return helpCommand();
  } else if(strcmp(args[0], "exit") == 0) {
    return FALSE;
  }

  return launch(args);
}

int cdCommand(char **args) {
  if(args[1] == NULL) {
    if(chdir(getenv("HOME")) != 0) {
      return FALSE;
    }
  } else {
    if(chdir(args[1]) != 0) {
      return FALSE;
    }
  }
  return TRUE;
}

int pwdCommand() {
  char *cwd = NULL;

  if((cwd = getcwd(NULL, 0)) == NULL) {
    return FALSE;
  }

  printf("%s\n", cwd);
  return TRUE;
}

// TBI
int echoCommand(char **args) {
  return TRUE;
}

int setCommand(char **args) {
  char *name = NULL;
  char *value = NULL;
  const char *delimeter = "=";

  if(args[3] != NULL && strcmp(args[2], "=") == 0) { // NAME = VALUE format
    name = args[1];
    value = args[3];
  } else if(args[2] != NULL && args[1][strlen(args[1])-1] == '=') { // NAME= VALUE format
    name = strtok(args[1], delimeter);
    value = args[2];
  } else if(args[2] != NULL && args[2][0] == '=') { // NAME =VALUE format
    name = args[1];
    value = &args[2][1];
  } else { // NAME=VALUE format
    name = strtok(args[1], delimeter);
    value = strtok(NULL, delimeter);
  }

  if(name == NULL || value == NULL) {
    return FALSE;
  }

  if(setenv(name, value, TRUE) != 0) {
    return FALSE;
  }

  return TRUE;
}

// TBI
int helpCommand() {
  return TRUE;
}

int launch(char **args) {
  pid_t pid = 0;

  if((pid = fork()) == 0) {
    if(execvp(args[0], args) < 0) {
      fprintf(stderr, "%s: Command doesn't exist.\n", args[0]);
      exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
  }

  waitpid(pid, NULL, 0);
  return TRUE;
}

/*

int main(int argc, char ** argv, char **envp) {

  char *prompt = "320sh> ";
  char cmd[MAX_INPUT];
  char *line = NULL;

  while(true) {
    char *cursor;
    char last_char;
    int rv;
    int count;

    // Print the prompt
    rv = write(STDOUT, prompt, strlen(prompt));
    if (rv == 0) { 
      break;
    }
    
    // read and parse the input
    for(rv = 1, count = 0, cursor = cmd, last_char = 1;
      rv && (++count < (MAX_INPUT-1)) && (last_char != '\n');
      cursor++) { 

      rv = read(STDIN, cursor, 1);
      last_char = *cursor;
    
      if(last_char == 3) {
        write(STDOUT, "^c", 2);
      } else {
       write(STDOUT, &last_char, 1);
      }
    }

    *cursor = '\0';

    if (rv == 0) { 
      break;
    }

  // Execute the command, handling built-in commands separately 
  // Just echo the command line for now
  // write(1, cmd, strnlen(cmd, MAX_INPUT));
  }

  return 0;
}

*/