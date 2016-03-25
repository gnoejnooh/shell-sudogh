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
			list->tail = command;
			list->head->prev = NULL;
			list->head->next = NULL;
		} else {
			command->prev = NULL;
			command->next = list->head;
			list->head->prev = command;
			list->head = command;
		}
		(list->count)++;
	} else {
		Command *temp = NULL;

		command->prev = NULL;
		command->next = list->head;
		list->head->prev = command;
		list->head = command;
		
		temp = list->tail;
		list->tail = list->tail->prev;
		list->tail->next = NULL;
		free(temp);
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