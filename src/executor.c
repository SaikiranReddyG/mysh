#include "shell.h"

/**
 * execute_command() - Execute a single command using fork/execvp
 * 
 * Creates a child process via fork(). The child process applies any
 * redirects, then replaces itself with the requested program via execvp().
 * The parent process returns the child's PID.
 * 
 * Returns: PID of child process on success, -1 on fork() failure
 */
pid_t execute_command(const Command *cmd)
{
    if (!cmd || !cmd->argv[0]) {
        fprintf(stderr, "Error: invalid command\n");
        return -1;
    }

    pid_t pid = fork();

    if (pid == -1) {
        /* Fork failed */
        perror("fork");
        return -1;
    }
    else if (pid == 0) {
        /* Child process */

        /* Apply redirects (if any) */
        if (apply_redirects(cmd) != 0) {
            fprintf(stderr, "Error applying redirects\n");
            exit(1);
        }

        /* Replace process with the requested program */
        execvp(cmd->argv[0], cmd->argv);

        /* execvp only returns on error */
        perror(cmd->argv[0]);
        exit(127);
    }
    else {
        /* Parent process: return child PID */
        return pid;
    }
}

/**
 * execute_pipeline() - Execute a pipeline of commands connected by pipes
 * 
 * For a pipeline like: cmd1 | cmd2 | cmd3
 * - Creates N-1 pipes (pipe() syscall) between N commands
 * - Forks N child processes
 * - Wires each child's stdout to next child's stdin via dup2()
 * - Parent waits for all children (or records them as background jobs)
 * 
 * Returns: PID of last child process (for waiting), or -1 on error
 */
pid_t execute_pipeline(const Pipeline *pipeline)
{
    if (!pipeline || pipeline->num_commands == 0) {
        return -1;
    }

    /* Special case: single command (no pipe) */
    if (pipeline->num_commands == 1) {
        return execute_command(&pipeline->commands[0]);
    }

    /* Multiple commands: create pipes and fork children */
    int num_commands = pipeline->num_commands;
    int pipes[num_commands - 1][2];  /* Array of pipes */
    pid_t pids[num_commands];        /* Track all child PIDs */

    /* Create all pipes */
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return -1;
        }
    }

    /* Fork and exec each command */
    for (int i = 0; i < num_commands; i++) {
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            return -1;
        }
        else if (pid == 0) {
            /* Child process */

            /* Wire stdin from previous command's pipe (if not first) */
            if (i > 0) {
                if (dup2(pipes[i - 1][0], STDIN_FILENO) == -1) {
                    perror("dup2");
                    exit(1);
                }
            }

            /* Wire stdout to next command's pipe (if not last) */
            if (i < num_commands - 1) {
                if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                    perror("dup2");
                    exit(1);
                }
            }

            /* Close all pipe ends in child (will use inherited copies) */
            for (int j = 0; j < num_commands - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            /* Apply any redirects for this command */
            if (apply_redirects(&pipeline->commands[i]) != 0) {
                fprintf(stderr, "Error applying redirects\n");
                exit(1);
            }

            /* Execute command */
            execvp(pipeline->commands[i].argv[0], pipeline->commands[i].argv);
            perror(pipeline->commands[i].argv[0]);
            exit(127);
        }
        else {
            /* Parent process: record child PID */
            pids[i] = pid;
        }
    }

    /* Parent: close all pipes (they're not needed here) */
    for (int i = 0; i < num_commands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    /* Return PID of last child (for waiting) */
    return pids[num_commands - 1];
}

