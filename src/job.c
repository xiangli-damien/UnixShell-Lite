#include "history.h"
#include "signal_handlers.h"
#include "shell.h"
#include "job.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/wait.h>
#include <errno.h>

extern msh_t* shell;

// Helper function to find an empty slot in the job array
static int find_empty_slot(job_t *jobs, int max_jobs) {
    for (int i = 0; i < max_jobs; i++) {
        if (jobs[i].cmd_line == NULL) {
            return i;
        }
    }
    return -1;  // No empty slot found
}

// Function to add a job to the job array
bool add_job(job_t *jobs, int max_jobs, pid_t pid, job_state_t state, const char *cmd_line) {
    //printf("begin Adding job: PID=%d, Command=%s\n", pid, cmd_line);
    int slot = find_empty_slot(jobs, max_jobs);
    if (slot == -1) {
        return false;  // No empty slot available
    }

    jobs[slot].cmd_line = strdup(cmd_line);  // Allocate memory and copy command line
    if (jobs[slot].cmd_line == NULL) {
        //printf("Failed to allocate memory for job command line\n");
        return false;  // Memory allocation failed
    }

    jobs[slot].state = state;
    jobs[slot].pid = pid;
    jobs[slot].jid = slot;  // Using the array index as the job ID
    //printf("finish Adding job: PID=%d, Command=%s\n", pid, cmd_line);

    return true;
}

// Function to delete a job from the job array based on its pid
bool delete_job(job_t *jobs, int max_jobs, pid_t pid) {
    //printf("Begin Deleting job: PID=%d\n", pid);
    for (int i = 0; i < max_jobs; i++) {
        if (jobs[i].pid == pid) {
            
            //printf("Deleting job: PID=%d, Command=%s\n", pid, jobs[i].cmd_line ? jobs[i].cmd_line : "(null)");
          
            if (jobs[i].cmd_line != NULL) {
                free(jobs[i].cmd_line);
                jobs[i].cmd_line = NULL;
            }

            jobs[i].state = UNDEFINED;
            jobs[i].pid = 0;
            jobs[i].jid = 0;
            return true;
        }
    }
    //printf("Job not found for PID=%d\n", pid);
    return false;
}

// Function to free the job array
void free_jobs(job_t *jobs, int max_jobs) {
    if (jobs != NULL) {
        // for (int i = 0; i < max_jobs; i++) {
        //     if (jobs[i].cmd_line != NULL) {
        //         free(jobs[i].cmd_line); 
        //         jobs[i].cmd_line = NULL;
        //         jobs[i].state = UNDEFINED;
        //         jobs[i].pid = 0;
        //         jobs[i].jid = 0;
        //     }
        // }
        free(jobs); 
    }
}

pid_t get_foreground_job_pid(job_t *jobs, int max_jobs) {
    for (int i = 0; i < max_jobs; i++) {
        if (jobs[i].state == FOREGROUND) {
            return jobs[i].pid;
        }
    }
    return -1; // No foreground job found
}

job_t *find_job_by_pid(job_t *jobs, int max_jobs, int pid) {
    for (int i = 0; i < max_jobs; i++) {
        if (jobs[i].pid == pid && jobs[i].cmd_line != NULL) {
            return &jobs[i];
        }
    }
    return NULL;  // No job found with the given PID
}

job_t *find_job_by_jid(job_t *jobs, int max_jobs, int jid) {
    for (int i = 0; i < max_jobs; i++) {
        if (jobs[i].jid == jid && jobs[i].cmd_line != NULL) {
            return &jobs[i];
        }
    }
    return NULL;  // No job found with the given JID
}


void print_jobs(job_t *jobs, int max_jobs) {
    //printf("Printing job list:\n");
    for (int i = 0; i < max_jobs; i++) {
        if (jobs[i].cmd_line != NULL) {
            //printf("Job %d: cmd_line pointer is %p\n", i, (void *)jobs[i].cmd_line);
            const char *state = (jobs[i].state == BACKGROUND) ? "RUNNING" : "SUSPENDED";
            printf("[%d] %d %s %s\n", jobs[i].jid, jobs[i].pid, state, jobs[i].cmd_line);
        }
    }
}

void change_job_state(job_t *job, job_state_t new_state, bool sendSigCont) {
    if (job != NULL && job->cmd_line != NULL) {
        job->state = new_state;
        // if (sendSigCont && new_state != SUSPENDED) {
        //     kill(job->pid, SIGCONT);  // Only send SIGCONT if requested and not suspending
        // }
    }
}

bool check_jobs_finished(job_t *jobs, int max_jobs) {
    for (int i = 0; i < max_jobs; i++) {
        if (jobs[i].pid != 0) {
            //printf("commadline is: %s, state: %d ,pid=%d\n", jobs[i].cmd_line, jobs[i].state, jobs[i].pid );
            return false;
        }
    }
    return true;
}