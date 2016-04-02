#ifndef __FILE_MANAGER_H__
#define __FILE_MANAGER_H__

#include "Command.h"

#define MAX_PATH		1024
#define MAX_INPUT		1024
#define HISTORY_PATH  	"/.320sh_history"

void importHistory(CommandList *commandList);
void exportHistory(CommandList *commandList);

#endif