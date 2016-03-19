#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024

#define STDIN     0
#define STDOUT    1
#define STDERR    2

typedef enum _bool {
  false = 0,
  true = 1
} bool;

/* It might be changed for Async-signal-safe function. (read) */
char * readLine() {
  char *line = NULL;
  size_t len = 0;
  getline(&line, &len, stdin);
  return line;
}

int main(int argc, char ** argv, char **envp) {
  
  char *prompt = "320sh> ";
  char *line = NULL;

  while(true) {

    if (write(STDOUT, prompt, strlen(prompt)) == 0) { 
      break;
    }
    
    line = readLine();
    write(STDOUT, line, strnlen(line, MAX_INPUT));

  // Execute the command, handling built-in commands separately 
  // Just echo the command line for now
  // write(1, cmd, strnlen(cmd, MAX_INPUT));
  }

  return 0;
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