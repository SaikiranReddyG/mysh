#include "shell.h"

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
        printf("No jobs\n");
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
    (void)args; /* TODO: Implement */
    printf("fg: not yet implemented\n");
    return 0;
}

int builtin_bg(char **args)
{
    (void)args; /* TODO: Implement */
    printf("bg: not yet implemented\n");
    return 0;
}

void builtin_history(void)
{
    /* TODO: Implement */
    printf("history: not yet implemented\n");
}
