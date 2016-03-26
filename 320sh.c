#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "Command.h"
#include "FileManager.h"

// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT     1024
#define MAX_TOKEN     128

#define TRUE          1
#define FALSE         0

#define SUCCESS       0
#define FAILURE       1

#define STDIN         0
#define STDOUT        1
#define STDERR        2

typedef enum mode {
  NORMAL,
  RED_O, // >
  RED_I, // <
  PIPE // |
} Mode;

void printPrompt();
char * readLine(CommandList *commandList);
void eraseLine(int count);
char ** getTokens(char *line);
void execute(char **args, int *status, int *run);

void cdCommand(char **args, int *status);
void pwdCommand(int *status);
void echoCommand(char **args, int *status);
void setCommand(char **args, int *status);
void helpCommand(int *status);
void exitCommand(int *status, int *run);

void launch(char **args, int *status);

int main(int argc, char ** argv, char **envp) {

  CommandList *commandList = malloc(sizeof(CommandList));

  char *line = NULL;
  char **tokens = NULL;

  //Mode mode = NORMAL;
  int status = 0;
  int run = TRUE;
  int debug = FALSE;
  int c = 0;

  while((c = getopt(argc, argv, "d")) != -1){
    switch(c) {
      case 'd':
        debug = TRUE;
        break;
      default:
        break;
      }
  }

  initializeList(commandList);
  importHistory(commandList);

  do {
    printPrompt();
    line = readLine(commandList);
    insertCommand(commandList, line);
    tokens = getTokens(line);

    if(debug == TRUE) {
      fprintf(stderr, "RUNNING: %s\n", *tokens);  
    }

    execute(tokens, &status, &run);

    if(debug == TRUE) {
      fprintf(stderr, "ENDED: %s (ret=%d)\n", tokens[0], status);  
    }

    //mode = RED_O;
    free(line);
    free(tokens);
  } while(run == TRUE);

  exportHistory(commandList);
  freeList(commandList);
  free(commandList);
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

char * readLine(CommandList *commandList) {

  Command *cur = NULL;
  char *line = malloc(sizeof(char) * MAX_INPUT);
  char *cursor = line;

  int count = 0;
  int last_char = 0;

  while(count < MAX_INPUT && last_char != '\n') {

    read(STDIN, cursor, 1);
    last_char = *cursor;

    if(last_char == 3) { // Ctrl + C
      write(STDOUT, "^c", 2);
      cursor++;
      count++;
    } else if(last_char != 0 && (last_char == 127 || last_char == 8)) { // Backspace

      if(count == 0) {
        continue;
      }
      
      write(STDOUT, "\b \b", 3);
      cursor--;
      count--;
    } else if(last_char == 27) {
      read(STDIN, cursor, 1);
      read(STDIN, cursor, 1);
      last_char = *cursor;
      
      switch(last_char) {
      case 'A': // UP KEY
        eraseLine(count);
        count = 0;
        cursor = line;

        if(cur == NULL) {
          cur = commandList->head;
        } else if(cur->next != NULL) {
          cur = cur->next;
        }

        if(cur != NULL) {
          strcpy(line, cur->line);
          write(STDOUT, line, strlen(line));
          count += strlen(line);
          cursor += strlen(line);
        }

        break;

      case 'B': // DOWN KEY
        eraseLine(count);
        count = 0;
        cursor = line;

        if(cur != NULL && cur->prev != NULL) {
          cur = cur->prev;
        }

        if(cur != NULL) {
          strcpy(line, cur->line);
          write(STDOUT, line, strlen(line));
          count += strlen(line);
          cursor += strlen(line);
        }
        
        break;
      case 'C': // RIGHT KEY
        eraseLine(count);
        count = 0;
        cursor = line;
        break;
      case 'D': // LEFT KEY
        eraseLine(count);
        count = 0;
        cursor = line;
        break;
      default:
        break;  
      }
    } else {
      write(STDOUT, &last_char, 1);
      cursor++;
      count++;
    }

    /*
    else if(strcmp(cursor, "\027[C")) {
      write(STDOUT, " ", 1);
      cursor += 2;
    } else if(strcmp(cursor, "\027[D")) {
      write(STDOUT, "", 1);
      cursor += 2;
    } 
    */
  }

  *cursor = '\0';
  return line;
}

void eraseLine(int count) {
  int i = 0;

  for(i=0; i<count; i++) {
    write(STDOUT, "\b", 1);
  }
  for(i=0; i<count; i++) {
    write(STDOUT, " ", 1);
  }
  for(i=0; i<count; i++) {
    write(STDOUT, "\b", 1);
  }
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

void execute(char **args, int *status, int *run) {
  if(args[0] == NULL) {
    return;
  }
  if(strcmp(args[0], "cd") == 0) {
    cdCommand(args, status);
    return;
  } else if(strcmp(args[0], "pwd") == 0) {
    pwdCommand(status);
    return;
  } else if(strcmp(args[0], "echo") == 0) {
    echoCommand(args, status);
    return;
  } else if(strcmp(args[0], "set") == 0) {
    setCommand(args, status);
    return;
  } else if(strcmp(args[0], "help") == 0) {
    helpCommand(status);
    return;
  } else if(strcmp(args[0], "exit") == 0) {
    exitCommand(status, run);
    return;
  }

  launch(args, status);
}

void cdCommand(char **args, int *status) {
  *status = FAILURE;

  if(args[1] == NULL) {
    chdir(getenv("HOME"));
  } else {
    if(chdir(args[1]) == -1) {
      fprintf(stderr, "cd: %s: No such file or directory\n", args[1]);
      return;
    }
  }

  *status = SUCCESS;
}

void pwdCommand(int *status) {
  *status = FAILURE;

  printf("%s\n", getcwd(NULL, 0));

  *status = SUCCESS;
}


// TBI
void echoCommand(char **args, int *status) {
  char *name = NULL;

  *status = FAILURE;

  if(args[1] == NULL) { // echo
    fprintf(stderr, "unsupported format\n");
    return;
  } else {
    if(args[1][0] == '$') {
      name = &args[1][1];

      if(strcmp(name, "?") == 0) { // echo $?
        printf("%d\n", WEXITSTATUS(*status));
      } else if(strcmp(args[1], "$") == 0) { // echo $
        printf("$\n");
      } else if(getenv(name) != NULL) { // echo $name
        printf("%s\n", getenv(name));
      } else { // echo $non_exist
        printf("\n");
      }
    } else {
      printf("%s\n", args[1]); // echo text
    }
  }

  *status = SUCCESS;
}

void setCommand(char **args, int *status) {
  char *name = NULL;
  char *value = NULL;
  const char *delimeter = "=";

  *status = FAILURE;

  if(args[3] != NULL && strcmp(args[2], "=") == 0) { // NAME = VALUE format
    name = args[1];
    value = args[3];
  } else if(args[2] != NULL && args[1][strlen(args[1])-1] == '=') { // NAME= VALUE format
    name = strtok(args[1], delimeter);
    value = args[2];
  } else if(args[2] != NULL && args[2][0] == '=') { // NAME =VALUE format
    name = args[1];
    value = &args[2][1];
  } else if(strchr(args[1], '=') != NULL) { // NAME=VALUE format
    name = strtok(args[1], delimeter);
    value = strtok(NULL, delimeter);
  }

  if(name != NULL && value != NULL) {
    setenv(name, value, TRUE);
  } else {
    fprintf(stderr, "unsupported format\n");
  }

  *status = SUCCESS;
}

// TBI
void helpCommand(int *status) {
  
  char *USAGE = (
  "320 SHell Builtin Commands\n"
  "cd: cd [dir]\n"
  "    Change the current directory to DIR. The variable $HOME is the default DIR.\n"
  "pwd: pwd\n"
  "     Print the current working directory.\n"
  "echo: echo\n"
  "     Output the ARGs.\n"
  "set: set opt\n"
  "     Modify existing environment variables and create new ones.\n"
  "exit: exit\n"
  "     Exit the shell with a status.\n"
  "help: help\n"
  "     Display helpful information about builtin commands.\n");

  *status = FAILURE;

  printf("%s", USAGE);
  
  *status = SUCCESS;
}

void exitCommand(int *status, int *run) {
  *status = FAILURE;
  *run = FALSE;
  *status = SUCCESS;
}

void launch(char **args, int *status) {
  pid_t pid = 0;

  if((pid = fork()) == 0) {
    if(execvp(args[0], args) < 0) {
      fprintf(stderr, "%s: command not found\n", args[0]);
      exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
  }

  waitpid(pid, status, 0);
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