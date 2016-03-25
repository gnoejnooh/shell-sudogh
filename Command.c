#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Command.h"

void initializeList(CommandList *list) {
	list->head = NULL;
	list->current = NULL;
	list->count = 0;
}

void insertCommand(CommandList *list, char *line) {
	Command *command = malloc(sizeof(Command));

	line[strlen(line)-1] = '\0';
	strcpy(command->line, line);

	if(list->count < CAPACITY) {
		if(list->count == 0) {
			list->head = command;
			list->head->prev = command;
			list->head->next = command;
		} else if(list->count == 1) {
			command->prev = list->head;
			command->next = list->head;
			list->head->prev = command;
			list->head->next = command;
			list->head = command;
		} else {
			command->prev = list->head->prev;
			command->next = list->head;
			list->head->prev->next = command;
			list->head->prev = command;
			list->head = command;
		}
		(list->count)++;
	} else {
		list->head = list->head->prev;
		strcpy(list->head->line, line);
	}
}

void freeList(CommandList *list) {
	Command *cur = list->head;
	Command *next = NULL;
	int i = 0;
	
	while(i < list->count) {
		next = cur->next;
		free(cur);
		cur = next;
		i++;
	}
}