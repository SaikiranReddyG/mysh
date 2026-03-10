#include "shell.h"

/**
 * signal_handler() - Handle signals for the shell
 * 
 * SIGCHLD: Reap background/stopped child processes
 * (SIGINT and SIGTSTP are ignored by parent shell)
 */
void signal_handler(int sig)
{
    if (sig == SIGCHLD) {
        /* Handle child process state changes */
        pid_t pid;
        while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
            /* Child process finished or stopped */
            Job *job = find_job_by_pid(pid);
            if (job) {
                remove_job(pid);
            }
        }
    }
}

/**
 * setup_signals() - Initialize signal handlers for the shell
 * 
 * - Shell ignores SIGINT (Ctrl+C) and SIGTSTP (Ctrl+Z)
 *   so user input doesn't accidentally kill the shell
 * - Child processes inherit default signal handlers
 * - Sets up SIGCHLD handler to reap background processes
 */
void setup_signals(void)
{
    struct sigaction sa;

    /* Ignore SIGINT (Ctrl+C) in the shell */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = SIG_IGN;
    sigaction(SIGINT, &sa, NULL);

    /* Ignore SIGTSTP (Ctrl+Z) in the shell */
    sigaction(SIGTSTP, &sa, NULL);

    /* Set up SIGCHLD handler to reap background processes */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;  /* Restart interrupted system calls */
    sa.sa_handler = signal_handler;
    sigaction(SIGCHLD, &sa, NULL);
}

