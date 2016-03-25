#ifndef __COMMAND_H__
#define __COMMAND_H__

#define MAX_INPUT 1024
#define CAPACITY 	50

typedef struct command {
	char line[MAX_INPUT];
	struct command *prev;
	struct command *next;
} Command;

typedef struct commandList {
	Command *head;
	Command *current;
	int count;
} CommandList;

void initializeList(CommandList *list);
void insertCommand(CommandList *list, char *line);
void freeList(CommandList *list);

#endif