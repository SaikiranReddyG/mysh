#ifndef SHELL_H
#define SHELL_H

/* Enable POSIX features like strtok_r */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

#define MAX_INPUT 1024          /* Max length of input line */
#define MAX_ARGS 64             /* Max number of arguments in a command */
#define MAX_JOBS 32             /* Max number of background jobs tracked */
#define MAX_HISTORY 100         /* Max number of commands in history */

/* ============================================================================
 * JOB CONTROL STRUCTURES
 * ============================================================================ */

/* Job states for background/stopped processes */
typedef enum {
    JOB_RUNNING,
    JOB_STOPPED,
    JOB_DONE
} JobState;

/* Represents a background or stopped job */
typedef struct {
    int job_id;                 /* Job number (1, 2, 3, ...) */
    pid_t pid;                  /* Process ID */
    JobState state;             /* Current state */
    char command[MAX_INPUT];    /* Original command string */
} Job;

/* ============================================================================
 * COMMAND PARSING STRUCTURES
 * ============================================================================ */

/* Redirection types */
typedef enum {
    REDIR_NONE,
    REDIR_STDOUT,               /* > file */
    REDIR_STDOUT_APPEND,        /* >> file */
    REDIR_STDIN,                /* < file */
    REDIR_STDERR                /* 2> file */
} RedirType;

/* Single redirection target */
typedef struct {
    RedirType type;
    char *filename;
} Redirect;

/* Represents a single command (part of a pipeline) */
typedef struct {
    char *argv[MAX_ARGS];       /* Command arguments (argv[0] is program name) */
    int argc;                   /* Number of arguments */
    int bg_flag;                /* 1 if command ends with &, 0 otherwise */
    Redirect *redirects;        /* Array of redirections (or NULL) */
    int num_redirects;          /* Number of redirections */
} Command;

/* Represents a pipeline: potentially multiple commands connected by pipes */
typedef struct {
    Command *commands;          /* Array of commands */
    int num_commands;           /* Number of commands in pipeline */
} Pipeline;

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/* -------- parser.c -------- */
/**
 * parse_input() - Parse raw input string into a pipeline structure
 * @input: Raw input string from user
 * Returns: Allocated Pipeline struct, or NULL on error
 * 
 * Handles: pipes (|), redirects (>, <, >>, 2>), background (&)
 */
Pipeline *parse_input(const char *input);

/**
 * free_pipeline() - Free all memory associated with a pipeline
 */
void free_pipeline(Pipeline *p);

/* -------- executor.c -------- */
/**
 * execute_command() - Execute a single command (fork/exec pattern)
 * @cmd: Parsed command to execute
 * Returns: PID of child process, or -1 on fork failure
 * 
 * Handles: redirects, background execution
 */
pid_t execute_command(const Command *cmd);

/**
 * execute_pipeline() - Execute multiple commands connected by pipes
 * @pipeline: Parsed pipeline structure
 * Returns: PID of last process in pipeline, or -1 on error
 */
pid_t execute_pipeline(const Pipeline *pipeline);

/* -------- builtins.c -------- */
/**
 * is_builtin() - Check if command is a built-in shell command
 * @cmd: Command to check (argv[0])
 * Returns: 1 if builtin, 0 otherwise
 */
int is_builtin(const char *cmd);

/**
 * execute_builtin() - Execute a built-in command
 * @pipeline: Parsed command (only uses first command)
 * Returns: 0 on success, -1 on error, or exit code if exit command
 */
int execute_builtin(const Pipeline *pipeline);

/**
 * builtin_cd() - Change directory
 * @args: Argument array (argv[1] is target directory)
 * Returns: 0 on success, -1 on failure
 */
int builtin_cd(char **args);

/**
 * builtin_exit() - Exit the shell
 * @args: Argument array (may contain exit code)
 * Exits process; does not return
 */
void builtin_exit(char **args);

/**
 * builtin_jobs() - List background and stopped jobs
 */
void builtin_jobs(void);

/**
 * builtin_fg() - Bring job to foreground
 * @args: Argument array (argv[1] is job id or PID)
 * Returns: 0 on success, -1 on failure
 */
int builtin_fg(char **args);

/**
 * builtin_bg() - Resume stopped job in background
 * @args: Argument array (argv[1] is job id or PID)
 * Returns: 0 on success, -1 on failure
 */
int builtin_bg(char **args);

/**
 * builtin_history() - Show command history
 */
void builtin_history(void);

/**
 * history_add() - Add a command to the command history
 */
void history_add(const char *cmd);

/* -------- redirect.c -------- */
/**
 * apply_redirects() - Apply all redirections to current process file descriptors
 * @cmd: Command with redirect information
 * Returns: 0 on success, -1 on error
 * 
 * Must be called in child process before execvp().
 * Opens files with appropriate flags and uses dup2() to redirect stdin/stdout/stderr.
 */
int apply_redirects(const Command *cmd);

/* -------- pipes.c -------- */
/**
 * execute_pipeline() - Execute a pipeline of commands
 * @pipeline: Pipeline structure with multiple commands
 * Returns: PID of last process in pipeline, or -1 on error
 * 
 * Creates pipes between commands, forks multiple children, wires file descriptors,
 * and closes pipes appropriately to avoid deadlocks.
 */
pid_t execute_pipeline_impl(const Pipeline *pipeline);

/* -------- signals.c -------- */
/**
 * setup_signals() - Initialize signal handlers
 * 
 * Makes shell ignore SIGINT and SIGTSTP so Ctrl+C/Z doesn't kill the shell.
 * Child processes inherit default handlers and respond normally.
 * Sets up SIGCHLD handler to reap background processes.
 */
void setup_signals(void);

/**
 * signal_handler() - Handle various signals
 */
void signal_handler(int sig);

/* -------- jobs.c -------- */
/**
 * init_jobs() - Initialize job tracking system
 */
void init_jobs(void);

/**
 * add_job() - Add a new background/stopped job to the list
 * @pid: Process ID of job
 * @state: Initial state (JOB_RUNNING or JOB_STOPPED)
 * @command: Original command string
 * Returns: Job ID (1, 2, 3, ...), or -1 if job list full
 */
int add_job(pid_t pid, JobState state, const char *command);

/**
 * remove_job() - Remove job by PID
 * @pid: Process ID to remove
 * Returns: Job ID that was removed, or -1 if not found
 */
int remove_job(pid_t pid);

/**
 * find_job_by_id() - Find job by job number
 * @job_id: Job number (1, 2, 3, ...)
 * Returns: Job structure, or NULL if not found
 */
Job *find_job_by_id(int job_id);

/**
 * find_job_by_pid() - Find job by process ID
 * @pid: Process ID
 * Returns: Job structure, or NULL if not found
 */
Job *find_job_by_pid(pid_t pid);

/**
 * update_job_state() - Update job state (used by SIGCHLD handler)
 * @pid: Process ID of job
 * @new_state: New state
 * Returns: 0 on success, -1 if job not found
 */
int update_job_state(pid_t pid, JobState new_state);

/**
 * list_jobs() - Return pointer to job array
 * Returns: Pointer to internal job array
 */
Job *list_jobs(void);

/**
 * get_job_count() - Get current number of jobs
 */
int get_job_count(void);

/* -------- main.c -------- */
/**
 * repl() - Run the read-eval-print loop
 * 
 * Prompts for input, parses commands, checks for builtins,
 * executes external commands, and manages signal handlers.
 */
void repl(void);

#endif /* SHELL_H */
