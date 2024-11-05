#define _GNU_SOURCE
#include "shell.h"
#include "history.h"
#include "signal_handlers.h"
#include "job.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/wait.h>
#include <errno.h>


msh_t *shell;

int main(int argc, char **argv) {
    int opt;
    int max_jobs = 0, max_line = 0, max_history = 0;
    char *endptr;

    // Analyze command line
    while ((opt = getopt(argc, argv, "s:j:l:")) != -1) {
        switch (opt) {
            case 's':
                max_history = (int)strtol(optarg, &endptr, 10);
                if (*endptr != '\0' || max_history <= 0) {
                    printf("usage: msh [-s NUMBER] [-j NUMBER] [-l NUMBER]\n");
                    return 1;
                }
                break;
            case 'j':
                max_jobs = (int)strtol(optarg, &endptr, 10);
                if (*endptr != '\0' || max_jobs <= 0) {
                    printf("usage: msh [-s NUMBER] [-j NUMBER] [-l NUMBER]\n");
                    return 1;
                }
                break;
            case 'l':
                max_line = (int)strtol(optarg, &endptr, 10);
                if (*endptr != '\0' || max_line <= 0) {
                    printf("usage: msh [-s NUMBER] [-j NUMBER] [-l NUMBER]\n");
                    return 1;
                }
                break;
            default: // Wrong format
                printf("usage: msh [-s NUMBER] [-j NUMBER] [-l NUMBER]\n");
                return 1;
        }
    }
    if (optind < argc) {
        // If we get here, there are additional arguments that are not options.
        printf("usage: msh [-s NUMBER] [-j NUMBER] [-l NUMBER]\n");
        return 1;
    }

    // Initialize msh_t 
    shell = alloc_shell(max_jobs, max_line, max_history);

    // REPL 
    char *line = NULL;
    size_t len = 0;
    ssize_t nRead;
    int should_exit = 0;
    
    printf("msh> ");
    while ((nRead = getline(&line, &len, stdin)) != -1) {
        should_exit = evaluate(shell, line); 
        
        free(line);
        line = NULL;
        
        if (should_exit) { 
            break;
        }
        printf("msh> ");
    }
    free(line);
    
    // int status;
    // pid_t pid;
    // // Wait for all background jobs to complete
    // while ((pid = waitpid(-1, &status, 0)) > 0) {
    //     // Example: Remove the job from the job list
    //     // delete_job(shell->jobs, shell->max_jobs, pid);
    // }
    exit_shell(shell);
    return 0;
}

