#include "shell.h"

pid_t execute_command(const Command *cmd)
{
    (void)cmd; /* TODO: Implement in Phase 3 */
    fprintf(stderr, "executor: not yet implemented\n");
    return -1;
}

pid_t execute_pipeline(const Pipeline *pipeline)
{
    (void)pipeline; /* TODO: Implement in Phase 5 */
    fprintf(stderr, "pipes: not yet implemented\n");
    return -1;
}
