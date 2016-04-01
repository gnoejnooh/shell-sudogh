#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Job.h"

void initializeJobList(JobList *list) {
  list->head = NULL;
  list->tail = NULL;
  list->count = 0;
}

void insertJob(JobList *list, char *args, int pid, JobState state) {

  Job *job = malloc(sizeof(Job));

  strcpy(job->args, args);
	job->pid = pid;
	job->jid = list->count;
  job->state = state;

  if(list->count == 0) {
    job->prev = NULL;
    job->next = NULL;
    list->head = job;
    list->tail = job;
  } else {
    job->prev = list->tail;
    job->next = NULL;
    list->tail->next = job;
    list->tail = job;
  }

  (list->count)++;
}

void freeJobList(JobList *list) {
	Job *cur = list->head;
	Job *next = NULL;
	int i = 0;

	while(i < list->count) {
		next = cur->next;
		free(cur);
		cur = next;
		i++;
	}
}