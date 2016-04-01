#ifndef __JOB_H__
#define __JOB_H__

#define MAX_JOB		256

typedef enum jobState {
	UNDEF,
	FG,
	BG,
	STOP
} JobState;

typedef struct job {
	char args[1024];
	int pid;
	int jid;

	JobState jobState;

	struct job *prev;
	struct job *next;
} Job;

typedef struct jobList {
 	Job *head;
 	Job *tail;
 	
 	int count;
} JobList;

void initializeJobList(JobList *list);
void insertJob(JobList *list, char *args, int pid, JobState state);
void freeJobList(JobList *list);

#endif