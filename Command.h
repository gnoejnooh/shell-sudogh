#ifndef __COMMAND_H__
#define __COMMAND_H__

#define MAX_INPUT 		1024
#define MAX_FILE_LEN	256
#define CAPACITY 			50

typedef enum mode {
  NORMAL,
  RED_O, // >
  RED_I, // <
  PIPE // |
} Mode;

typedef struct command {
	char line[MAX_INPUT];
	struct command *prev;
	struct command *next;
} Command;

typedef struct commandList {
	Command *head;
	Command *tail;
	int count;
} CommandList;

// Redirection-wise
typedef struct workUnit {
	char args[MAX_INPUT];

	Mode mode;

	struct workUnit *prev;
	struct workUnit *next;
} WorkUnit;

typedef struct workUnitList {
	WorkUnit *head;
	WorkUnit *tail;
	int count;
} WorkUnitList;

// Piping-wise
typedef struct work {
  char args[MAX_INPUT];
  WorkUnitList *workUnitList;

  Mode mode;

  struct work *prev;
  struct work *next;
} Work;

typedef struct workList {
	Work *head;
	Work *tail;
	int count;
} WorkList;

void initializeCommandList(CommandList *list);
void insertCommand(CommandList *list, char *line); // General purpose
void constructCommand(CommandList *list, char *line); // File importing purpose only
void freeCommandList(CommandList *list);

void initializeWorkList(WorkList *list);
void insertWork(WorkList *list, char *args, Mode mode);
void freeWorkList(WorkList *list);

void parseWorkArgs(char *args, char *args1, char *args2, Mode *mode);

void initializeWorkUnitList(WorkUnitList *list);
void insertWorkUnit(WorkUnitList *list, char *args, Mode mode);
void freeWorkUnitList(WorkUnitList *list);

#endif