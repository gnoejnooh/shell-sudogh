#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "Command.h"
#include "Job.h"
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
void insertChar(char *string, int len, int c, int pos);
void deleteChar(char *string, int len, int pos);
void eraseLine(int pos, int count);
void parseLine(char *line, char *args1, char *args2, Mode *mode);
char ** getTokens(char *line);
void executeLine(char *line, JobList *jobList, int *run, int debug);
void constructOrder(Work *cur, int workCount, char *line, JobList *jobList, int *run, int debug); // For piping and redirection
void executeWork(WorkUnit *cur, int workUnitCount, int *status, int *run, Mode mode);
void execute(char **args, int *status, int *run, Mode mode); // For single command

void cdCommand(char **args, int *status);
void pwdCommand(int *status);
void echoCommand(char **args, int *status);
void setCommand(char **args, int *status);
void helpCommand(int *status);
void exitCommand(int *status, int *run);

void launch(char **args, int *status, Mode mode);

int main(int argc, char ** argv, char **envp) {

  CommandList *commandList = malloc(sizeof(CommandList));
  JobList *jobList = malloc(sizeof(JobList));

  char *line = NULL;

  int run = TRUE;
  int debug = FALSE;
  int c = 0;

  while((c = getopt(argc, argv, "d")) != -1) {
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
    line = readLine(commandList);
    insertCommand(commandList, line);

    executeLine(line, jobList, &run, debug);

    free(line);

  } while(run == TRUE);

  exportHistory(commandList);
  freeCommandList(commandList);
  freeJobList(jobList);
  free(commandList);
  free(jobList);

  return 0;
}

void printPrompt() {
  char *prompt = "320sh> ";
  char *cwd = getcwd(NULL, 0);
  char *directory = strrchr(cwd, '/');

  write(STDOUT, "[", 1);
  write(STDOUT, directory, strlen(directory));
  write(STDOUT, "] ", 2);
  write(STDOUT, prompt, strlen(prompt));

  free(cwd);
}

char * readLine(CommandList *commandList) {

  Command *cur = NULL;
  char *line = malloc(sizeof(char) * MAX_INPUT);
  char *remainLine = NULL;
  int c = 0;
  int pos = 0;

  int count = 0;
  int last_char = 0;

  line[0] = '\0';

  while(count < MAX_INPUT) {

    read(STDIN, &c, 1);
    last_char = c;

    if(last_char == '\n') {
      write(STDOUT, "\n", 1);
      break;
    } else if(last_char == '\t') {
      continue;
    } else if(last_char == 3) { // Ctrl + C
      write(STDOUT, "^c", 2);
    } else if(last_char != 0 && (last_char == 127 || last_char == 8)) { // Backspace

      int i = 0;
      int temp = 0;
      
      if(pos == 0) {
        continue;
      }

      deleteChar(line, count, pos-1);
      eraseLine(pos, count);
      temp = count - pos;
      count = 0;

      write(STDOUT, line, strlen(line));
      count += strlen(line);
      pos--;

      for(i=0; i<temp; i++) {
        write(STDOUT, "\033[1D", 4);
      }

    } else if(last_char == 27) {
      read(STDIN, &c, 1);
      read(STDIN, &c, 1);
      last_char = c;
      
      switch(last_char) {
      case 'A': // UP KEY
        eraseLine(pos, count);
        count = 0;
        pos = 0;

        if(cur == NULL) {
          cur = commandList->head;
        } else if(cur->next != NULL) {
          cur = cur->next;
        }

        if(cur != NULL) {
          strcpy(line, cur->line);
          write(STDOUT, line, strlen(line));
          count += strlen(line);
          pos += strlen(line);
        }

        break;

      case 'B': // DOWN KEY
        eraseLine(pos, count);
        count = 0;
        pos = 0;

        if(cur != NULL && cur->prev != NULL) {
          cur = cur->prev;
        }

        if(cur != NULL) {
          strcpy(line, cur->line);
          write(STDOUT, line, strlen(line));
          count += strlen(line);
          pos += strlen(line);
        }
        
        break;
      case 'C': // RIGHT KEY
        if(pos == count) {
          continue;
        }

        write(STDOUT, "\033[1C", 4);
        pos++;
        remainLine = &line[pos];
        break;
      case 'D': // LEFT KEY
        if(pos == 0) {
          continue;
        }
        
        write(STDOUT, "\033[1D", 4);
        pos--;
        remainLine = &line[pos];
        break;
      default:
        break;
      }
    } else {
      if(pos == count) {
        write(STDOUT, &last_char, 1);   
      } else {
        int i = 0;

        write(STDOUT, &last_char, 1);
        write(STDOUT, remainLine, strlen(remainLine));

        for(i=pos; i<count; i++) {
          write(STDOUT, "\033[1D", 4);
        }
      }

      insertChar(line, count, last_char, pos);
      pos++;
      count++;
      remainLine = &line[pos];
    }
  }

  return line;
}

void insertChar(char *string, int len, int c, int pos) {
  int i = 0;

  if(string == NULL) {
    return;
  }

  for(i = len; i >= pos; i--) {
    string[i+1] = string[i];
  }

  string[pos] = c;
}

void deleteChar(char *string, int len, int pos) {
  int i = 0;

  if(string == NULL) {
    return;
  }

  for(i=pos; i<len; i++) {
    string[i] = string[i+1];
  }
}

void eraseLine(int pos, int count) {
  int i = 0;

  for(i=0; i<pos; i++) {
    write(STDOUT, "\b", 1);
  }
  for(i=0; i<count; i++) {
    write(STDOUT, " ", 1);
  }
  for(i=0; i<count; i++) {
    write(STDOUT, "\b", 1);
  }
}

void parseLine(char *line, char *args1, char *args2, Mode *mode) {
  int i = 0;
  *mode = NORMAL;

  for(i=0; i<strlen(line); i++) {
    switch(line[i]) {
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

void executeLine(char *line, JobList *jobList, int *run, int debug) {

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

  constructOrder(cur, workList->count, line, jobList, run, debug);

  free(args1);
  free(args2);
  freeWorkList(workList);
  free(workList);
}

void constructOrder(Work *cur, int workCount, char *line, JobList *jobList, int *run, int debug) {
  
  char **tokens = NULL;

  int pid = 0;
  int fd[workCount * 2];

  int status = 0;
  int i = 0;
  int count = 0;

  switch(cur->mode) {
  case NORMAL:

    tokens = getTokens(cur->args);
    if(debug == TRUE) {
      fprintf(stderr, "RUNNING: %s\n", *tokens);  
    }
    executeWork(cur->workUnitList->head, cur->workUnitList->count, &status, run, NORMAL);
    if(debug == TRUE) {
      fprintf(stderr, "ENDED: %s (ret=%d)\n", tokens[0], status);  
    }

    free(tokens);
    break;
  case PIPE:
    for(i=0; i<workCount; i++) {
      pipe(&fd[i*2]);
    }

    while(cur != NULL) {

      if((pid = fork()) == 0) {
        // FIRST WORK
        if(cur->prev == NULL) {
          insertJob(jobList, line, getpid(), UNDEF);
        }
        // NOT FIRST WORK
        if(cur->prev != NULL) {
          dup2(fd[(count-1)*2], STDIN);
        }
        // NOT LAST WORK
        if(cur->next != NULL) {
          dup2(fd[count*2+1], STDOUT);
        }

        for(i=0; i<workCount * 2; i++) {
          close(fd[i]);
        }

        tokens = getTokens(cur->args);
        if(debug == TRUE) {
          fprintf(stderr, "RUNNING: %s\n", *tokens);  
        }
        executeWork(cur->workUnitList->head, cur->workUnitList->count, &status, run, PIPE);
        if(debug == TRUE) {
          fprintf(stderr, "ENDED: %s (ret=%d)\n", tokens[0], status);  
        }

        free(tokens);
      }

      cur = cur->next;
      count++;
    }

    for(i=0; i<workCount * 2; i++) {
      close(fd[i]);
    }

    for(i=0; i<workCount+1; i++) {
      wait(&status);
    }

    break;
  default:
    break;
  }
}

void executeWork(WorkUnit *cur, int workUnitCount, int *status, int *run, Mode mode) {

  char **tokens = NULL;
  WorkUnit *head = cur;
  
  int pid = 0;
  int fd = 0;

  if(cur->mode == NORMAL) {
    tokens = getTokens(cur->args);
    execute(tokens, status, run, mode);
    free(tokens);

  } else {
    if((pid = fork()) == 0) {
      while(cur->next != NULL) {
        Mode unitMode = cur->mode;
        cur = cur->next;

        if(unitMode == RED_O) { // >
          if((fd = open(cur->args, O_WRONLY | O_CREAT | O_TRUNC)) != -1) {
            dup2(fd, STDOUT);
            close(fd);
          }
        } else if(unitMode == RED_I) { // <
          if((fd = open(cur->args, O_RDONLY)) != -1) {
            dup2(fd, STDIN);
            close(fd);
          }
        }
      }

      tokens = getTokens(head->args);
      execute(tokens, status, run, mode);

      free(tokens);
      exit(EXIT_SUCCESS);
    }

    waitpid(pid, status, 0);

    if(mode == PIPE) {
      exit(EXIT_SUCCESS);
    }
  }
}

void execute(char **args, int *status, int *run, Mode mode) {
  if(args[0] == NULL) {
    return;
  }

  if(strcmp(args[0], "cd") == 0) {
    cdCommand(args, status);
    if(mode == PIPE) {
      exit(EXIT_SUCCESS);
    }
    return;
  } else if(strcmp(args[0], "pwd") == 0) {
    pwdCommand(status);
    if(mode == PIPE) {
      exit(EXIT_SUCCESS);
    }
    return;
  } else if(strcmp(args[0], "echo") == 0) {
    echoCommand(args, status);
    if(mode == PIPE) {
      exit(EXIT_SUCCESS);
    }
    return;
  } else if(strcmp(args[0], "set") == 0) {
    setCommand(args, status);
    if(mode == PIPE) {
      exit(EXIT_SUCCESS);
    }
    return;
  } else if(strcmp(args[0], "help") == 0) {
    helpCommand(status);
    if(mode == PIPE) {
      exit(EXIT_SUCCESS);
    }
    return;
  } else if(strcmp(args[0], "exit") == 0) {
    exitCommand(status, run);
    if(mode == PIPE) {
      exit(EXIT_SUCCESS);
    }
    return;
  }

  launch(args, status, mode);
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
  char *cwd = getcwd(NULL, 0);
  *status = FAILURE;

  printf("%s\n", cwd);
  free(cwd);

  *status = SUCCESS;
}


// TBI
void echoCommand(char **args, int *status) {
  //char *name = NULL;
  char* var_plus = NULL;
  int i = 2;
  *status = FAILURE;

  if(args[1] == NULL) { // echo
    printf("\n");
    return;
  }

  if(strcmp(args[1], "$") == 0) {
    printf("%s", "$");
    while(args[i] != NULL) {
      printf(" %s", args[i]);
      i++;
    }
    printf("\n");
    fflush(stdout);
  } else if(args[1][0] == '$') {
    var_plus = &args[1][2];
    if(args[1][1] == '?') {
      printf("%d", WEXITSTATUS(*status));
      printf("%s", var_plus);
      while(args[i] != NULL) {
        printf(" %s", args[i]);
        i++;
      }
      printf("\n");
      fflush(stdout);
    } else if(args[1][1] == '$') {
      printf("%d", getpid());
      printf("%s", var_plus);
      while(args[i] != NULL) {
        printf(" %s", args[i]);
        i++;
      }
      printf("\n");
      fflush(stdout);
    } else {
      printf("\n");
    }
  } else {
    printf("%s", args[1]);
    while(args[i] != NULL) {
      printf(" %s", args[i]);
      i++;
    }
    printf("\n");
    fflush(stdout);
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

void launch(char **args, int *status, Mode mode) {
  pid_t pid = 0;

  if(mode == PIPE) {
    if(execvp(args[0], args) < 0) {
      fprintf(stderr, "%s: command not found\n", args[0]);
      exit(EXIT_FAILURE);
    }
    return;
  }

  if((pid = fork()) == 0) {
    if(execvp(args[0], args) < 0) {
      fprintf(stderr, "%s: command not found\n", args[0]);
      exit(EXIT_FAILURE);
    }
  }

  waitpid(pid, status, 0);
}