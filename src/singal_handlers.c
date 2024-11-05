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
// SIGCHLD

// gloable variable
// the shell I used in main
extern msh_t* shell;


/*
* sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
*     a child job terminates (becomes a zombie), or stops because it
*     received a SIGSTOP or SIGTSTP signal. The handler reaps all
*     available zombie children, but doesn't wait for any other
*     currently running children to terminate.
* Citation: Bryant and O’Hallaron, Computer Systems: A Programmer’s Perspective, Third Edition
*/
void sigchld_handler(int sig) {
    // printf("Entered sigchld_handler\n");

    pid_t pid;
    int olderrno = errno;
    int status;
    sigset_t mask_all, prev_all;

    sigfillset(&mask_all);

    if (shell == NULL) {
        //printf("Error: shell is NULL\n");
        return;
    }
    if (shell->jobs == NULL) {
        //printf("Error: shell->jobs is NULL\n");
        return;
    }

    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        //printf("Handling PID: %d\n", pid);
        sigprocmask(SIG_BLOCK, &mask_all, &prev_all);

        /* Check if the child process is stopped */
        if (WIFSTOPPED(status)) {
            //printf("Child process PID: %d is stopped\n", pid);
            // Handle the case when the child is stopped, you can update job state here
        }
        else if (WIFEXITED(status) || WIFSIGNALED(status)) {
            //printf("Child process PID: %d is terminated\n", pid);
            delete_job(shell->jobs, shell->max_jobs, pid);
        }

        sigprocmask(SIG_SETMASK, &prev_all, NULL);
    }

    if (pid == 0) {
        //printf("No child process changed state\n");
    }

    // if (errno != ECHILD) {
    //     perror("waitpid error");
    // }

    // printf("Exiting sigchld_handler\n");
    errno = olderrno;
}


/*
* sigint_handler - The kernel sends a SIGINT to the shell whenver the
*    user types ctrl-c at the keyboard.  Catch it and send it along
*    to the foreground job.
* Citation: Bryant and O’Hallaron, Computer Systems: A Programmer’s Perspective, Third Edition
*/
void sigint_handler(int sig) {
    pid_t fg_pid = get_foreground_job_pid(shell->jobs, shell->max_jobs);
    if (fg_pid != 0) {
        kill(-fg_pid, SIGINT);  // Send SIGINT to the foreground job
        // delete_job(shell->jobs, shell->max_jobs, fg_pid); // Delete the job
    }
}


/*
* sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
*     the user types ctrl-z at the keyboard. Catch it and suspend the
*     foreground job by sending it a SIGTSTP.
* Citation: Bryant and O’Hallaron, Computer Systems: A Programmer’s Perspective, Third Edition
*/
void sigtstp_handler(int sig) {
    //printf("enter sigstp, %d/n", sig);
    pid_t fg_pid = get_foreground_job_pid(shell->jobs, shell->max_jobs);
    if (fg_pid != 0) {
        kill(-fg_pid, SIGTSTP);  // Send SIGTSTP to the foreground job

        // Find the job and change its state to SUSPENDED
        for (int i = 0; i < shell->max_jobs; i++) {
            if (shell->jobs[i].pid == fg_pid) {
                //printf("start suspend");
                change_job_state(&shell->jobs[i], SUSPENDED, false); // false means no SIGCONT
                break;
            }
        }
    }
}

typedef void handler_t(int);
handler_t *setup_handler(int signum, handler_t *handler) {
    
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART; 

    if (sigaction(signum, &action, &old_action) < 0) {
        perror("sigaction");
        exit(1);
    }
    return (old_action.sa_handler);
}


void initialize_signal_handlers() {
    setup_handler(SIGCHLD, sigchld_handler); 
    setup_handler(SIGINT,  sigint_handler);  
    setup_handler(SIGTSTP, sigtstp_handler);
}
