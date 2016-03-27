#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FileManager.h"

void importHistory(CommandList *commandList) {
	FILE *fp;
	char *line = malloc(sizeof(char) * MAX_INPUT);
	char *path = malloc(sizeof(char) * MAX_PATH);

	strcpy(path, getenv("HOME"));
	strcat(path, HISTORY_PATH);
	
	fp = fopen(path, "r");

	if(fp == NULL) {
		return;
	}

	while(fscanf(fp, "%s", line) != EOF) {
		constructCommand(commandList, line);
	}

	free(path);
	fclose(fp);
}

void exportHistory(CommandList *commandList) {
	FILE *fp;
	Command *cur = commandList->head;
	char *path = malloc(sizeof(char) * MAX_PATH);

	strcpy(path, getenv("HOME"));
	strcat(path, HISTORY_PATH);

	fp = fopen(path, "w");
	
	if(fp == NULL) {
		return;
	}
		
	while(cur != NULL) {
		fprintf(fp, "%s\n", cur->line);
		cur = cur->next;
	}

	free(path);
	fclose(fp);
}