#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
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

void printPrompt();
char * readLine(CommandList *commandList);
void eraseLine(int count);
void parseLine(char *line, char *args1, char *args2, Mode *mode);
char ** getTokens(char *line);
void executeLine(char *line, int *run, int debug);
void constructOrder(Work *cur, int *run, int debug); // For piping and redirection
void execute(char **args, int *status, int *run); // For single command

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

  initializeCommandList(commandList);
  importHistory(commandList);

  do {
    printPrompt();
    line = readLine(commandList);\
    insertCommand(commandList, line);

    executeLine(line, &run, debug);

    free(line);

  } while(run == TRUE);

  exportHistory(commandList);
  freeCommandList(commandList);
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
  char *screen = malloc(sizeof(char) * MAX_INPUT);
  char *cursor = line;
  int screenCursor = 0;
  int screenEnd = 0;
  char *start = cursor;
  char *end = cursor;
  int count = 0;
  int last_char = 0;

  while(count < MAX_INPUT && last_char != '\n') {

    read(STDIN, cursor, 1);
    last_char = *cursor;
    if(last_char == '\n') {
      continue;
    }
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
          strcpy(screen, cur->line);
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
          strcpy(screen, cur->line);
          write(STDOUT, line, strlen(line));
          count += strlen(line);
          cursor += strlen(line);
        }
        
        break;
      case 'C': // RIGHT KEY
        if(last_char == *end) {
          continue;
        }
        write(STDOUT, "\033[1C", 4);
        cursor++;
        screenCursor++;
        break;
      case 'D': // LEFT KEY
        if(last_char == *start) {
          continue;
        }
        write(STDOUT, "\033[1D", 4);
        cursor--;
        screenCursor--;
        break;
      default:
        break;  
      }
    } else {
      if(screenCursor < screenEnd) {
        char* q;
        int i;
        for(q = &screen[screenCursor], i = screenEnd - screenCursor; --i >= 0;) {
          q[i + 1] = q[i];
        }
        memcpy(&screen[screenCursor], &last_char, 1);
        screen[screenEnd + 1] = '\0';
      } else {
        screen[screenCursor] = *cursor;
      }
      if(cursor < end) {
        write(STDOUT, "\r", 1);
        printPrompt();
        strcpy(line, screen);
        write(STDOUT, line, strlen(line));
        for(int i = screenEnd - screenCursor; --i >= 0;) {
          write(STDOUT, "\033[1D", 4);
        }
      } else {
        write(STDOUT, &last_char, 1); 
      }
      cursor++;
      screenCursor++;
      screenEnd++;
      count++;
      end++;
    }
  }
  write(STDOUT, "\n", 1);
  *cursor = '\0';
  return screen;
}

void eraseLine(int count) {
	write(STDOUT, "\r", 1);
  printPrompt();
  /*
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
  */
}

void parseLine(char *line, char *args1, char *args2, Mode *mode) {
  int i = 0;
  *mode = NORMAL;

  for(i=0; i<strlen(line); i++) {
    switch(line[i]) {
    case '>':
      line[i] = '\0';
      strcpy(args1, line);
      while(line[i+1] == ' ') i++;
      strcpy(args2, &line[i+1]);
      *mode = RED_O;
      return;
    case '<':
      line[i] = '\0';
      strcpy(args1, line);
      while(line[i+1] == ' ') i++;
      strcpy(args2, &line[i+1]);
      *mode = RED_I;
      return;
    case '|':
      line[i] = '\0';
      strcpy(args1, line);
      while(line[i+1] == ' ') i++;
      strcpy(args2, &line[i+1]);
      *mode = PIPE;
      return;
    default:
      continue;
    }
  }

  strcpy(args1, line);
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

void executeLine(char *line, int *run, int debug) {

  WorkList *workList = malloc(sizeof(WorkList));
  Work *cur = NULL;

  char *args1 = malloc(sizeof(char) * MAX_TOKEN);
  char *args2 = malloc(sizeof(char) * MAX_TOKEN);

  Mode mode = NORMAL;

  initializeWorkList(workList);

  do {
    parseLine(line, args1, args2, &mode);
    insertWork(workList, args1, mode);
    strcpy(line, args2);
  } while(mode != NORMAL);

  cur = workList->head;

  constructOrder(cur, run, debug);

  free(args1);
  free(args2);
  freeWorkList(workList);
  free(workList);
}

void constructOrder(Work *cur, int *run, int debug) {
  int pid, pid2 = 0;
  int fd = 0;
  int status = 0;
  int pip[2];
  char **tokens = getTokens(cur->args);

  if(debug == TRUE) {
    fprintf(stderr, "RUNNING: %s\n", *tokens);  
  }

  switch(cur->mode) {
  case NORMAL:
    execute(tokens, &status, run);
    break;
  case RED_O: // >
    if((pid = fork()) == 0) {
      if((fd = open(cur->next->args, O_WRONLY | O_CREAT | O_TRUNC)) != -1) {
        dup2(fd, STDOUT);
        execute(tokens, &status, run);
      }
      exit(EXIT_SUCCESS);
    }
    waitpid(pid, &status, 0);
    break;
  case RED_I: // <
    if((pid = fork()) == 0) {
      if((fd = open(cur->next->args, O_RDONLY)) != -1) {
        dup2(fd, STDIN);
        execute(tokens, &status, run);
      }
      exit(EXIT_SUCCESS);
    }
    waitpid(pid, &status, 0);
    break;
  case PIPE:
  	pipe(pip);
    if((pid = fork()) < 0) {
      printf("\nerror\n");
    } else if(pid == 0) {
      printf("\nRUNNING PIPE:%s\n", tokens[0]);
      close(pip[0]);
      dup2(pip[1], STDOUT);
      close(pip[1]);
      execvp(tokens[0], tokens);
    } else {
      waitpid(pid, &status, 0);
      if((pid2 = fork()) < 0) {
        printf("\nerror\n");
      } else if(pid2 == 0) {
        printf("\nRUNNING PIPE2:%s\n", tokens[1]);
        close(pip[1]);
        dup2(pip[0], STDIN);
        close(pip[0]);
        execvp(tokens[0], tokens);
      } else {
        close(pip[0]);
        close(pip[1]);
      }
    }
    break;
  default:
    break;
  }

  if(debug == TRUE) {
    fprintf(stderr, "ENDED: %s (ret=%d)\n", tokens[0], status);  
  }

  free(tokens);
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