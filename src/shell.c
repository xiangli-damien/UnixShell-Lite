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

extern char **environ;

const int DEFAULT_MAX_LINE = 1024;
const int DEFAULT_MAX_JOBS = 16;
const int DEFAULT_MAX_HISTORY = 10;

msh_t *alloc_shell(int max_jobs, int max_line, int max_history) {

    // define a pointer pointing to msh_t type struct, and dynamically allocate memory to it
    msh_t *shell = malloc(sizeof(msh_t));
    if (!shell) {
        fprintf(stderr, "Fail to allocate memory");
        return NULL;
    }

    initialize_signal_handlers();
    
    // Assign value to the structure member using pointer

    shell->max_jobs = (max_jobs > 0 && max_jobs <= DEFAULT_MAX_JOBS) ? max_jobs : DEFAULT_MAX_JOBS;
    shell->max_line = (max_line > 0 && max_line <= DEFAULT_MAX_LINE) ? max_line : DEFAULT_MAX_LINE;
    shell->max_history = (max_history > 0 && max_history <= DEFAULT_MAX_HISTORY) ? max_history : DEFAULT_MAX_HISTORY;
    
    shell->jobs = malloc(sizeof(job_t) * shell->max_jobs);
    if (!shell->jobs) {
        fprintf(stderr, "Failed to allocate memory for jobs\n");
        free(shell);
        return NULL;
    }

    // Initialize the jobs array
    for (int i = 0; i < max_jobs; i++) {
        shell->jobs[i].cmd_line = NULL;
        shell->jobs[i].state = UNDEFINED;
        shell->jobs[i].pid = 0;
        shell->jobs[i].jid = 0;
    }

    // Initialize History
    shell->history = alloc_history(shell->max_history);
    if (!shell->history) {
        fprintf(stderr, "Failed to allocate memory for history\n");
        
        free(shell->jobs);
        free(shell);
        return NULL;
    }

    // return the pointer to the initialized struct
    return shell;
}


char *parse_tok(char *line, int *job_type) {
    static char *next_cmd = NULL;
    if (line != NULL) {
        next_cmd = line;
    }

    if (next_cmd == NULL || *next_cmd == '\0') {
        if (job_type != NULL) {
            *job_type = -1;
        }
        return NULL;
    }

    char *start_cmd = next_cmd;
    char *end_cmd;
    for (end_cmd = start_cmd; *end_cmd != '\0'; ++end_cmd) {
        if (*end_cmd == '&' || *end_cmd == ';') {
            break;
        }    
    }

    if (*end_cmd == '&') {
        *job_type = BACKGROUND;
    } else if (*end_cmd == ';') {
        *job_type = FOREGROUND;
    } else {
        *job_type = FOREGROUND;
    }

    if (*end_cmd != '\0') {
        *end_cmd = '\0';
        next_cmd = end_cmd + 1;

        char *check_cmd = next_cmd;
        while (*check_cmd == ' ' && *check_cmd != '\0') {
            check_cmd++;
        }

        if (*check_cmd == '\0') {
            next_cmd = NULL;
        }

    } else {
        next_cmd = NULL;
    }

    return start_cmd;
}


char **separate_args(char *line, int *argc, bool *is_builtin) {
    //Ignore the is_builtin parameter and set them to false
    *is_builtin = false;
    
    // If the input line is empty or only contains whitespace, return NULL
    if (line == NULL || strcmp(line, "") == 0) {
        *argc = 0;
        return NULL;
    }

    // Count the number of arguments
    int count = 0;
    char *tmp = line;
    while (*tmp) {
        while (*tmp == ' ' && *tmp) tmp++; // Skip spaces
        if (*tmp) count++;
        while (*tmp != ' ' && *tmp) tmp++; // Skip non-space characters
    }

    // Allocate space for the argument array
    char **argv = (char **)malloc((count + 1) * sizeof(char *));
    if (!argv) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    // Populate the argument array
    int index = 0;
    const char *delim = " ";
    char *token = strtok(line, delim);
    while (token != NULL) {
        argv[index++] = strdup(token); // Copy the string
        token = strtok(NULL, delim);
    }
    argv[index] = NULL; // Set the last element to NULL
    *argc = count;

    if (strcmp(argv[0], "jobs") == 0 || strcmp(argv[0], "history") == 0 || argv[0][0] == '!' || strcmp(argv[0], "bg") == 0 ||strcmp(argv[0], "fg") == 0 || strcmp(argv[0], "kill") == 0) {
        *is_builtin = true;
    }

    return argv;
}


int evaluate(msh_t *shell, char *line) {
    // Replace newline character at the end of the line with a null character
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') {
        line[len - 1] = '\0';
    }

    // Check if the input line exceeds the maximum length limit
    if (strlen(line) > shell->max_line) {
        printf("error: reached the maximum line limit\n");
        return 0;
    }

    // Add the command line to the history, excluding empty lines and 'exit'
    if (line[0] != '\0' && strcmp(line, "exit") != 0 && line[0] != '!') {
        //printf("Adding to history: %s\n", line);
        add_line_history(shell->history, line);
    }

    char *job;
    int job_type;
    int argc;
    char **argv;
    bool is_builtin;

    // Parse and execute each command in the input line
    job = parse_tok(line, &job_type);
    while (job != NULL) {
        argv = separate_args(job, &argc, &is_builtin);
        //printf("Parsed command: %s\n", job);
        // for (int i = 0; i < argc; i++) {
        //     printf("Arg %d: %s\n", i, argv[i]);
        // }

        if (argv != NULL) {
            // Handle 'exit' command
            if (strcmp(argv[0], "exit") == 0) {
                for (int i = 0; i < argc; i++) {
                    free(argv[i]);
                }
                free(argv);
                return 1; // Indicate shell should exit
            }

            if (is_builtin) {
                //printf("Executing builtin command: %s\n", argv[0]);
                char *builtin_result = builtin_cmd(argv, shell);
                //printf("Builtin command executed\n");

            // For history expansion (!N command)
                if (builtin_result != NULL) {
                    evaluate(shell, builtin_result);
                    free(builtin_result);
                }
            } else {
                // Execute external command if not a builtin command

                // Block SIGCHLD signals to avoid race conditions
                sigset_t mask_all, mask_one, prev_mask;
                sigfillset(&mask_all);
                sigemptyset(&mask_one);
                sigaddset(&mask_one, SIGCHLD);
                //signal(SIGCHLD, handler);
                sigprocmask(SIG_BLOCK, &mask_one, &prev_mask);

                // Fork a child process to execute the external command
                pid_t pid = fork();
                if (pid < 0) {
                    perror("fork");
                } else if (pid == 0) {
                    // Child process
                    setpgid(0, 0); // Put child in a new process group
                    sigprocmask(SIG_SETMASK, &prev_mask, NULL); // Unblock SIGCHLD
                    // char *envp[] = { "PATH=/bin:/usr/bin", NULL };
                    if (execve(argv[0], argv, environ) == -1) {
                        perror("execve");
                        exit(EXIT_FAILURE);
                    }
                } else {
                    // Parent process
                    sigprocmask(SIG_BLOCK, &mask_all, NULL);
                    add_job(shell->jobs, shell->max_jobs, pid, (job_type == BACKGROUND) ? BACKGROUND : FOREGROUND, job);
                    sigprocmask(SIG_SETMASK, &prev_mask, NULL); // Restore signal mask

                    // Wait for foreground job to complete
                    if (job_type != BACKGROUND) {
                        waitfg(pid, shell);
                    }
                }
            }

            // Free the memory allocated for argument array
            for (int i = 0; i < argc; i++) {
                free(argv[i]);
            }
            free(argv);
        }

        // Parse the next command in the line
        job = parse_tok(NULL, &job_type);
    }

    return 0; // Continue running the shell
}


void waitfg(pid_t pid, msh_t *shell) {
    // Loop until the foreground process completes or is suspended/deleted
    while(1) {
        sleep(1);
        //printf("Waiting for foreground job PID: %d\n", pid);
        if(is_job_deleted_or_suspended(shell, pid)) {
            //printf("foreground job PID is deleted or suspended: %d\n", pid);
            break;
        }
    }
}


char *builtin_cmd(char **argv, msh_t *shell) {
    if (strcmp(argv[0], "jobs") == 0) {
        print_jobs(shell->jobs, shell->max_jobs);
        return NULL;
    } else if (strcmp(argv[0], "history") == 0) {
        print_history(shell->history);
        return NULL;
    } else if (argv[0][0] == '!') {
        int index = atoi(&argv[0][1]);
        if (index >= 1 && index <= shell->history->next) {
            return strdup(shell->history->lines[index - 1]);
        } else {
            printf("No such command in history.\n");
        }
        return NULL;
    } else if (strcmp(argv[0], "bg") == 0 || strcmp(argv[0], "fg") == 0) {
        if (argv[1] != NULL) {
            bool is_jid = argv[1][0] == '%';
            int id = atoi(is_jid ? argv[1] + 1 : argv[1]);
            job_t *job = is_jid ? find_job_by_jid(shell->jobs, shell->max_jobs, id) : find_job_by_pid(shell->jobs, shell->max_jobs, id);

            if (job != NULL) {
                change_job_state(job, strcmp(argv[0], "bg") == 0 ? BACKGROUND : FOREGROUND, true); // true 表示发送 SIGCONT 信号
                if (strcmp(argv[0], "fg") == 0) {
                    waitfg(job->pid, shell);
                }
                //printf("[%d] %d continued %s\n", job->jid, job->pid, job->cmd_line);
            } else {
                printf("Job [%d] not found.\n", id);
            }
        } else {
            //printf("Usage: %s <job_id>\n", argv[0]);
        }
        return NULL;
    } else if (strcmp(argv[0], "kill") == 0) {

        char *endptr;
        int sig_num = strtol(argv[1], &endptr, 10);
        if (*endptr != '\0') {
            //printf("error: invalid signal number\n");
            return NULL;
        }

        int pid = strtol(argv[2], &endptr, 10);
        if (*endptr != '\0' || pid <= 0) {
            //printf("error: invalid PID\n");
            return NULL;
        }

        // 根据信号编号更新作业状态
        job_t *job = find_job_by_pid(shell->jobs, shell->max_jobs, pid);
        if (job != NULL) {
            switch (sig_num) {
                case 2: // SIGINT
                case 9: // SIGKILL
                    delete_job(shell->jobs, shell->max_jobs, pid);
                    break;
                case 18: // SIGCONT
                    job->state = SUSPENDED;
                    break;
                case 19: // SIGSTOP
                    job->state = SUSPENDED;
                    break;
                default:
                    //printf("error: invalid signal number\n");
                    return NULL;
            }
        } else {
            printf("No job found with PID %d\n", pid);
        }

        return NULL;
    }
}
    // } else if (strcmp(argv[0], "bg") == 0) {
    //     if (argv[1] != NULL) {
    //         int job_id = atoi(argv[1]);
    //         job_t *job = find_job_by_jid(shell->jobs, shell->max_jobs, job_id);
    //         if (job != NULL && job->state == SUSPENDED) {
    //             change_job_state(job, BACKGROUND, true);  // true indicates to send SIGCONT
    //             printf("[%d] %d continued %s\n", job->jid, job->pid, job->cmd_line);
    //         } else {
    //             printf("Job [%d] not found or not suspended.\n", job_id);
    //         }
    //     } else {
    //         printf("Usage: bg <job_id>\n");
    //     }
    //     return NULL;
    // } else if (strcmp(argv[0], "fg") == 0) {
    //     if (argv[1] != NULL) {
    //         int job_id = atoi(argv[1]);
    //         job_t *job = find_job_by_jid(shell->jobs, shell->max_jobs, job_id);
    //         if (job != NULL) {
    //             change_job_state(job, FOREGROUND, true);  // true indicates to send SIGCONT
    //             waitfg(job->pid, shell);
    //             printf("[%d] %d continued %s\n", job->jid, job->pid, job->cmd_line);
    //         } else {
    //             printf("Job [%d] not found.\n", job_id);
    //         }
    //     } else {
    //         printf("Usage: fg <job_id>\n");
    //     }
    //     return NULL;
    // }



void exit_shell(msh_t *shell) {
    // int status;
    // pid_t pid;
    // // Wait for all background jobs to complete
    // while ((pid = waitpid(-1, &status, 0)) > 0) {
    //     // Example: Remove the job from the job list
    //     // delete_job(shell->jobs, shell->max_jobs, pid);
    // }

    // Free the job array and other resources
    while(1){
        if(check_jobs_finished(shell->jobs, shell->max_jobs)){
            break;
        } 
        sleep(1);
    }
    free_jobs(shell->jobs, shell->max_jobs);
    if (shell) {
        free(shell);
    }
}


bool is_job_deleted_or_suspended(msh_t *shell, pid_t pid) {
    for (int i = 0; i < shell->max_jobs; i++) {
        if (shell->jobs[i].pid == pid) {
            // If the job is still present but suspended, return true
            if (shell->jobs[i].state == SUSPENDED) {
                return true;
            }
            // Job is present and not suspended, return false
            return false;
        }
    }
    // If the job is not found, it means it has been deleted
    return true;
}
