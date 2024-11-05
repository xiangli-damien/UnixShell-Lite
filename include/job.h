#ifndef _JOB_H_
#define _JOB_H_

#include <stdbool.h>
#include <sys/types.h>
// #include "signal_handlers.h"

typedef enum job_state{FOREGROUND, BACKGROUND, SUSPENDED, UNDEFINED} job_state_t;

// Represents a job in a shell.
typedef struct job {
    char *cmd_line;     // The command line for this specific job.
    job_state_t state;  // The current state for this job
    pid_t pid;          // The process id for this job
    int jid;            // The job number for this job
}job_t;

/*
* add_job: Adds a new job to the jobs array.
*
* jobs: The array of job_t structures.
* 
* max_jobs: The maximum number of jobs allowed.
*
* pid: The process id of the new job.
*
* state: The initial state of the new job (FOREGROUND, BACKGROUND, etc.).
*
* cmd_line: The command line string associated with the new job.
*
* Returns: true if the job was successfully added, false otherwise.
*/
bool add_job(job_t *jobs, int max_jobs, pid_t pid, job_state_t state, const char *cmd_line);

/*
* delete_job: Removes a job from the jobs array based on its pid.
*
* jobs: The array of job_t structures.
* 
* max_jobs: The maximum number of jobs allowed.
*
* pid: The process id of the job to be removed.
*
* Returns: true if the job was successfully removed, false otherwise.
*/
bool delete_job(job_t *jobs, int max_jobs, pid_t pid);

/*
* free_jobs: Frees the memory allocated for the jobs array.
*
* jobs: The array of job_t structures to be freed.
*
* max_jobs: The maximum number of jobs in the array.
*/
void free_jobs(job_t *jobs, int max_jobs);


pid_t get_foreground_job_pid(job_t *jobs, int max_jobs);


void print_jobs(job_t *jobs, int max_jobs);

job_t *find_job_by_pid(job_t *jobs, int max_jobs, int pid);

job_t *find_job_by_jid(job_t *jobs, int max_jobs, int jid);


bool check_jobs_finished(job_t *jobs, int max_jobs);

void change_job_state(job_t *job, job_state_t new_state, bool sendSigCont);


#endif