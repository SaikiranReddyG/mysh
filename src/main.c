#include "shell.h"

/**
 * repl() - Read-eval-print loop
 * 
 * Main loop of the shell:
 * 1. Print prompt
 * 2. Read input
 * 3. Parse into pipeline
 * 4. Check if builtin command
 * 5. If not builtin, execute as external command
 * 6. Repeat until EOF or exit command
 */
void repl(void)
{
    char buffer[MAX_INPUT];
    Pipeline *pipeline;
    int should_exit = 0;

    while (!should_exit) {
        /* Print prompt */
        printf("mysh> ");
        fflush(stdout);

        /* Read input */
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            /* EOF (Ctrl+D) */
            printf("\n");
            break;
        }

        /* Remove trailing newline */
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        /* Skip empty lines */
        if (strlen(buffer) == 0) {
            continue;
        }

        /* Parse input */
        pipeline = parse_input(buffer);
        if (!pipeline) {
            /* Parse error or empty command; continue loop */
            continue;
        }

        /* Check if first command is a builtin */
        if (pipeline->num_commands == 1 && 
            is_builtin(pipeline->commands[0].argv[0])) {
            /* Execute builtin */
            int result = execute_builtin(pipeline);
            if (result == 999) {
                /* Special code from builtin_exit() */
                should_exit = 1;
            }
        }
        else {
            /* Execute external command(s) */
            pid_t pid;
            if (pipeline->num_commands > 1) {
                /* Pipeline */
                pid = execute_pipeline(pipeline);
            }
            else {
                /* Single command */
                pid = execute_command(&pipeline->commands[0]);
            }

            /* If foreground, wait for completion */
            if (pid > 0 && !pipeline->commands[pipeline->num_commands - 1].bg_flag) {
                waitpid(pid, NULL, 0);
            }
        }

        /* Clean up */
        free_pipeline(pipeline);
    }
}

int main(void)
{
    /* Initialize shell state */
    setup_signals();
    init_jobs();

    /* Run the REPL */
    repl();

    /* Cleanup on exit */
    printf("exit\n");
    return 0;
}
