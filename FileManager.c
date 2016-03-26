#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FileManager.h"

void importHistory(CommandList *commandList) {
	FILE *fp;
	char *line = malloc(sizeof(char) * MAX_INPUT);
	
	if((fp = fopen(HISTORY_FILE, "r")) != NULL) {
		
		while(fscanf(fp, "%s", line) != EOF) {
			constructCommand(commandList, line);
		}

		fclose(fp);
	}
}

void exportHistory(CommandList *commandList) {
	FILE *fp;
	Command *cur = commandList->head;
	
	if((fp = fopen(HISTORY_FILE, "w")) != NULL) {
		
		while(fp != NULL && cur != NULL) {
			fprintf(fp, "%s\n", cur->line);
			cur = cur->next;
		}

		fclose(fp);
	}
}