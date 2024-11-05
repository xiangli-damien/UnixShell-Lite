#ifndef _SIGNAL_HANDLERS_H_
#define _SIGNAL_HANDLERS_H_


// extern msh_t *shell;

void initialize_signal_handlers();
void sigchld_handler(int sig);
void sigint_handler(int sig);
void sigtstp_handler(int signo);

#endif
