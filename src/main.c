#include "shell.h"

/* Main REPL loop that read the input,  evalaute the input and print the output and start the loop again */
void repl(void)
{
    char buffer[MAX_INPUT];
    Pipeline *pipeline;
    int should_exit = 0;

    while (!should_exit) {

        printf("mysh> ");
        fflush(stdout);


        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {

            printf("\n");
            break;
        }


        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }


        if (strlen(buffer) == 0) {
            continue;
        }


        pipeline = parse_input(buffer);
        if (!pipeline) {

            continue;
        }


        history_add(buffer);


        if (pipeline->num_commands == 1 && 
            is_builtin(pipeline->commands[0].argv[0])) {

            int result = execute_builtin(pipeline);
            if (result == 999) {

                should_exit = 1;
            }
        }
        else {

            pid_t pid;
            if (pipeline->num_commands > 1) {

                pid = execute_pipeline(pipeline);
            }
            else {

                pid = execute_command(&pipeline->commands[0]);
            }


            int is_background = pipeline->commands[pipeline->num_commands - 1].bg_flag;

            if (pid > 0) {
                if (is_background) {

                    add_job(pid, JOB_RUNNING, buffer);
                    printf("[%d] %d\n", get_job_count(), pid);
                }
                else {

                    waitpid(pid, NULL, 0);
                }
            }
        }


        free_pipeline(pipeline);
    }
}

/*the main entry point */
int main(void)
{
    /* */
    setup_signals();
    init_jobs();


    repl();


    printf("exit\n");
    return 0;
}
