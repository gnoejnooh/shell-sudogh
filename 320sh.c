#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024
#define MAX_TOKEN 128
#define DELIMETER " \n\t\f\r\v"

#define TRUE      1
#define FALSE     0

#define STDIN     0
#define STDOUT    1
#define STDERR    2

char * readLine();
char ** getTokens(char *line);
int execute(char **args);

int cdCommand(char **args);
int pwdCommand();
int launch(char **args);

int main(int argc, char ** argv, char **envp) {
  
  char *prompt = "320sh> ";
  char *line = NULL;
  char **tokens = NULL;
  int status;

  do {

    write(STDOUT, prompt, strlen(prompt));
    
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

char * readLine() {
  char *line = NULL;
  size_t len = 0;

  getline(&line, &len, stdin);
  return line;
}

char ** getTokens(char *line) {
  int pos = 0;
  char **tokens = malloc(sizeof(char *) * MAX_TOKEN);
  char *token;

  token = strtok(line, DELIMETER);

  while(token != NULL) {
    tokens[pos] = token;
    pos++;
    token = strtok(NULL, DELIMETER);
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
    return TRUE;
  } else if(strcmp(args[0], "set") == 0) {
    return TRUE;
  } else if(strcmp(args[0], "help") == 0) {
    return TRUE;
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
  char *cwd;

  if((cwd = getcwd(NULL, 0)) == NULL) {
    return FALSE;
  }

  printf("%s\n", cwd);
  return TRUE;
}

int launch(char **args) {
  pid_t pid;

  if((pid = fork()) == 0) {
    if(execvp(args[0], args) < 0) {
      fprintf(stderr, "%s: Command not found.\n", args[0]);
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