#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Command.h"

void initializeCommandList(CommandList *list) {
	list->head = NULL;
	list->tail = NULL;
	list->count = 0;
}

/* General purpose */
void insertCommand(CommandList *list, char *line) {
	Command *command = malloc(sizeof(Command));

	if(line[strlen(line)-1] == '\n') {
		line[strlen(line)-1] = '\0';
	}
	
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

/* File importing purpose only (Insert at tail) */
void constructCommand(CommandList *list, char *line) {
	Command *command = malloc(sizeof(Command));

	strcpy(command->line, line);

	if(list->count == 0) {
		list->head = command;
		list->tail = command;
		list->head->prev = NULL;
		list->head->next = NULL;
	} else {
		command->prev = list->tail;
		command->next = NULL;
		list->tail->next = command;
		list->tail = command;
	}

	(list->count)++;
}

void freeCommandList(CommandList *list) {
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

void initializeWorkList(WorkList *list) {
	list->head = NULL;
	list->tail = NULL;
	list->count = 0;
}

void insertWork(WorkList *list, char *args, Mode mode) {
	Work *work = malloc(sizeof(Work));
	work->workUnitList = malloc(sizeof(WorkUnitList));

	char *args1 = malloc(sizeof(char) * MAX_INPUT);
  char *args2 = malloc(sizeof(char) * MAX_INPUT);

  Mode workUnitMode = NORMAL;

  initializeWorkUnitList(work->workUnitList);

	strcpy(work->args, args);
	work->mode = mode;

	do {
    parseWorkArgs(args, args1, args2, &workUnitMode);
    insertWorkUnit(work->workUnitList, args1, workUnitMode);
    strcpy(args, args2);
  } while(workUnitMode != NORMAL);

	if(list->count == 0) {
		work->prev = NULL;
		work->next = NULL;
		list->head = work;
		list->tail = work;
	} else {
		work->prev = list->tail;
		work->next = NULL;
		list->tail->next = work;
		list->tail = work;
	}

	(list->count)++;

	free(args1);
	free(args2);
}

void freeWorkList(WorkList *list) {
	Work *cur = list->head;
	Work *next = NULL;
	int i = 0;

	while(i < list->count) {
		next = cur->next;
		freeWorkUnitList(cur->workUnitList);
		free(cur->workUnitList);
		free(cur);
		cur = next;
		i++;
	}
}

void parseWorkArgs(char *args, char *args1, char *args2, Mode *mode) {
	int i = 0;
	int j = 0;
  *mode = NORMAL;

  for(i=0; i<strlen(args); i++) {
    switch(args[i]) {
    case '>':
    	j = i-1;
    	while(args[j] == ' ') {
    		args[j] = '\0';
    	}
      args[i] = '\0';
      strcpy(args1, args);
      while(args[i+1] == ' ') i++;
      strcpy(args2, &args[i+1]);
      *mode = RED_O;
      return;
    case '<':
    	j = i-1;
    	while(args[j] == ' ') {
    		args[j] = '\0';
    	}
      args[i] = '\0';
      strcpy(args1, args);
      while(args[i+1] == ' ') i++;
      strcpy(args2, &args[i+1]);
      *mode = RED_I;
      return;
    default:
      continue;
    }
  }

  strcpy(args1, args);
}

void initializeWorkUnitList(WorkUnitList *list) {
	list->head = NULL;
	list->tail = NULL;
	list->count = 0;
}

void insertWorkUnit(WorkUnitList *list, char *args, Mode mode) {
	WorkUnit *unit = malloc(sizeof(WorkUnit));

	strcpy(unit->args, args);
	unit->mode = mode;

	if(list->count == 0) {
		unit->prev = NULL;
		unit->next = NULL;
		list->head = unit;
		list->tail = unit;
	} else {
		unit->prev = list->tail;
		unit->next = NULL;
		list->tail->next = unit;
		list->tail = unit;
	}
	(list->count)++;
}

void freeWorkUnitList(WorkUnitList *list) {
	WorkUnit *cur = list->head;
	WorkUnit *next = NULL;
	int i = 0;

	while(i < list->count) {
		next = cur->next;
		free(cur);
		cur = next;
		i++;
	}
}