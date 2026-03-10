#include "shell.h"

/* Command history buffer */
static char history[MAX_HISTORY][MAX_INPUT];
static int history_count = 0;

/**
 * add_to_history() - Add a command to the history buffer
 */
static void add_to_history(const char *cmd)
{
    if (history_count < MAX_HISTORY) {
        strncpy(history[history_count], cmd, MAX_INPUT - 1);
        history[history_count][MAX_INPUT - 1] = '\0';
        history_count++;
    }
    else {
        /* Rotate: remove oldest, add newest */
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            strcpy(history[i], history[i + 1]);
        }
        strncpy(history[MAX_HISTORY - 1], cmd, MAX_INPUT - 1);
        history[MAX_HISTORY - 1][MAX_INPUT - 1] = '\0';
    }
}

int is_builtin(const char *cmd)
{
    if (!cmd) return 0;

    return strcmp(cmd, "cd") == 0 ||
           strcmp(cmd, "exit") == 0 ||
           strcmp(cmd, "jobs") == 0 ||
           strcmp(cmd, "fg") == 0 ||
           strcmp(cmd, "bg") == 0 ||
           strcmp(cmd, "history") == 0;
}

int execute_builtin(const Pipeline *pipeline)
{
    if (!pipeline || pipeline->num_commands == 0 || !pipeline->commands[0].argv[0]) {
        return -1;
    }

    char *cmd = pipeline->commands[0].argv[0];
    char **args = pipeline->commands[0].argv;

    if (strcmp(cmd, "cd") == 0) {
        return builtin_cd(args);
    }
    else if (strcmp(cmd, "exit") == 0) {
        builtin_exit(args);
        return 999; /* Special code to trigger shell exit */
    }
    else if (strcmp(cmd, "jobs") == 0) {
        builtin_jobs();
        return 0;
    }
    else if (strcmp(cmd, "fg") == 0) {
        return builtin_fg(args);
    }
    else if (strcmp(cmd, "bg") == 0) {
        return builtin_bg(args);
    }
    else if (strcmp(cmd, "history") == 0) {
        builtin_history();
        return 0;
    }

    return -1;
}

int builtin_cd(char **args)
{
    char *target = NULL;

    if (args[1] == NULL) {
        /* cd with no args: go to home */
        target = getenv("HOME");
        if (!target) {
            fprintf(stderr, "cd: HOME not set\n");
            return -1;
        }
    }
    else {
        target = args[1];
    }

    if (chdir(target) != 0) {
        perror("cd");
        return -1;
    }

    return 0;
}

void builtin_exit(char **args)
{
    int code = 0;
    if (args[1] != NULL) {
        code = atoi(args[1]);
    }
    exit(code);
}

void builtin_jobs(void)
{
    Job *jobs = list_jobs();
    int count = get_job_count();

    if (count == 0) {
        printf("(no jobs)\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        const char *state_str = (jobs[i].state == JOB_RUNNING) ? "Running" : 
                                (jobs[i].state == JOB_STOPPED) ? "Stopped" : "Done";
        printf("[%d]\t%s\t%s\n", jobs[i].job_id, state_str, jobs[i].command);
    }
}

int builtin_fg(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "fg: usage: fg [job_id]\n");
        return -1;
    }

    int job_id = atoi(args[1]);
    Job *job = find_job_by_id(job_id);

    if (!job) {
        fprintf(stderr, "fg: job %d not found\n", job_id);
        return -1;
    }

    /* Bring job to foreground and wait */
    printf("%s\n", job->command);
    
    /* Resume if stopped */
    if (job->state == JOB_STOPPED) {
        kill(job->pid, SIGCONT);
    }

    /* Wait for job to finish */
    int status;
    waitpid(job->pid, &status, 0);

    /* Remove from job list */
    remove_job(job->pid);
    
    return 0;
}

int builtin_bg(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "bg: usage: bg [job_id]\n");
        return -1;
    }

    int job_id = atoi(args[1]);
    Job *job = find_job_by_id(job_id);

    if (!job) {
        fprintf(stderr, "bg: job %d not found\n", job_id);
        return -1;
    }

    if (job->state != JOB_STOPPED) {
        fprintf(stderr, "bg: job %d is not stopped\n", job_id);
        return -1;
    }

    /* Resume in background */
    if (kill(job->pid, SIGCONT) != 0) {
        perror("kill");
        return -1;
    }

    job->state = JOB_RUNNING;
    printf("[%d]\t%s\n", job->job_id, job->command);
    
    return 0;
}

void builtin_history(void)
{
    printf("Command history (last %d commands):\n", history_count);
    for (int i = 0; i < history_count; i++) {
        printf("%3d: %s\n", i + 1, history[i]);
    }
}

/**
 * history_add() - Add command to history (called from main.c)
 */
void history_add(const char *cmd)
{
    add_to_history(cmd);
}
