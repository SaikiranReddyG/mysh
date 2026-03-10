#include "shell.h"

/* Helper: Skip leading whitespace */
static const char *skip_whitespace(const char *s)
{
    while (*s && (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r'))
        s++;
    return s;
}

/* Helper: Skip trailing whitespace */
static void strip_trailing_whitespace(char *s)
{
    int len = strlen(s);
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\t' || 
                       s[len-1] == '\n' || s[len-1] == '\r')) {
        s[--len] = '\0';
    }
}

/* Helper: Tokenize a single command string into argv */
static int tokenize_command(char *cmd_str, char **argv, int max_args)
{
    int argc = 0;
    char *token;
    char *saveptr;

    /* Make a temporary copy for tokenization */
    char *tmp = strdup(cmd_str);
    if (!tmp) {
        perror("strdup");
        return 0;
    }

    token = strtok_r(tmp, " \t", &saveptr);
    while (token && argc < max_args) {
        argv[argc] = strdup(token);  /* Copy token to persist after strtok buffer is freed */
        if (!argv[argc]) {
            perror("strdup");
            free(tmp);
            return argc;
        }
        argc++;
        token = strtok_r(NULL, " \t", &saveptr);
    }

    free(tmp);
    argv[argc] = NULL;
    return argc;
}

/* Helper: Parse redirects from a command string and remove them from argv */
static Redirect *parse_redirects(char **argv, int *argc, int *num_redirects)
{
    Redirect *redirects = NULL;
    int count = 0;
    int i = 0;

    *num_redirects = 0;

    while (i < *argc) {
        char *arg = argv[i];

        if (strcmp(arg, ">") == 0) {
            /* Output redirect: > file */
            if (i + 1 >= *argc) {
                fprintf(stderr, "Error: > requires filename\n");
                if (redirects) free(redirects);
                return NULL;
            }
            redirects = realloc(redirects, (count + 1) * sizeof(Redirect));
            redirects[count].type = REDIR_STDOUT;
            redirects[count].filename = strdup(argv[i + 1]);
            count++;

            /* Remove both > and filename from argv */
            for (int j = i; j < *argc - 2; j++) {
                argv[j] = argv[j + 2];
            }
            *argc -= 2;
        }
        else if (strcmp(arg, ">>") == 0) {
            /* Append redirect: >> file */
            if (i + 1 >= *argc) {
                fprintf(stderr, "Error: >> requires filename\n");
                if (redirects) free(redirects);
                return NULL;
            }
            redirects = realloc(redirects, (count + 1) * sizeof(Redirect));
            redirects[count].type = REDIR_STDOUT_APPEND;
            redirects[count].filename = strdup(argv[i + 1]);
            count++;

            for (int j = i; j < *argc - 2; j++) {
                argv[j] = argv[j + 2];
            }
            *argc -= 2;
        }
        else if (strcmp(arg, "<") == 0) {
            /* Input redirect: < file */
            if (i + 1 >= *argc) {
                fprintf(stderr, "Error: < requires filename\n");
                if (redirects) free(redirects);
                return NULL;
            }
            redirects = realloc(redirects, (count + 1) * sizeof(Redirect));
            redirects[count].type = REDIR_STDIN;
            redirects[count].filename = strdup(argv[i + 1]);
            count++;

            for (int j = i; j < *argc - 2; j++) {
                argv[j] = argv[j + 2];
            }
            *argc -= 2;
        }
        else if (strcmp(arg, "2>") == 0) {
            /* Stderr redirect: 2> file */
            if (i + 1 >= *argc) {
                fprintf(stderr, "Error: 2> requires filename\n");
                if (redirects) free(redirects);
                return NULL;
            }
            redirects = realloc(redirects, (count + 1) * sizeof(Redirect));
            redirects[count].type = REDIR_STDERR;
            redirects[count].filename = strdup(argv[i + 1]);
            count++;

            for (int j = i; j < *argc - 2; j++) {
                argv[j] = argv[j + 2];
            }
            *argc -= 2;
        }
        else {
            i++;
        }
    }

    *num_redirects = count;
    return redirects;
}

/* Helper: Parse a single command (between pipes) */
static Command *parse_single_command(char *cmd_str)
{
    Command *cmd = malloc(sizeof(Command));
    if (!cmd) {
        perror("malloc");
        return NULL;
    }

    /* Make a writable copy for tokenization */
    char *cmd_copy = strdup(cmd_str);
    strip_trailing_whitespace(cmd_copy);

    /* Tokenize */
    cmd->argc = tokenize_command(cmd_copy, cmd->argv, MAX_ARGS);
    free(cmd_copy);

    if (cmd->argc == 0) {
        free(cmd);
        return NULL;
    }

    /* Check for background flag (&) */
    cmd->bg_flag = 0;
    if (cmd->argc > 0 && strcmp(cmd->argv[cmd->argc - 1], "&") == 0) {
        cmd->bg_flag = 1;
        cmd->argc--;
        cmd->argv[cmd->argc] = NULL;
    }

    /* Parse redirects */
    cmd->redirects = parse_redirects(cmd->argv, &cmd->argc, &cmd->num_redirects);

    return cmd;
}

/**
 * parse_input() - Parse raw input string into a pipeline structure
 * 
 * Breaks input on pipes (|), creates Command for each part,
 * parses redirects and background flag within each command.
 */
Pipeline *parse_input(const char *input)
{
    if (!input || input[0] == '\0') {
        return NULL;
    }

    Pipeline *pipeline = malloc(sizeof(Pipeline));
    if (!pipeline) {
        perror("malloc");
        return NULL;
    }

    /* Make writable copy */
    char *input_copy = strdup(input);
    char *saveptr;
    char *cmd_part;
    int cmd_count = 0;
    Command **cmds = NULL;

    /* Split on pipes */
    cmd_part = strtok_r(input_copy, "|", &saveptr);
    while (cmd_part) {
        cmd_part = (char *)skip_whitespace(cmd_part);
        
        Command *cmd = parse_single_command(cmd_part);
        if (cmd) {
            cmds = realloc(cmds, (cmd_count + 1) * sizeof(Command *));
            cmds[cmd_count++] = cmd;
        }

        cmd_part = strtok_r(NULL, "|", &saveptr);
    }

    free(input_copy);

    if (cmd_count == 0) {
        free(pipeline);
        free(cmds);
        return NULL;
    }

    /* Convert array of Command* to array of Command */
    pipeline->commands = malloc(cmd_count * sizeof(Command));
    for (int i = 0; i < cmd_count; i++) {
        pipeline->commands[i] = *cmds[i];
        free(cmds[i]);
    }
    free(cmds);

    pipeline->num_commands = cmd_count;
    return pipeline;
}

/**
 * free_pipeline() - Free all memory associated with a pipeline
 */
void free_pipeline(Pipeline *p)
{
    if (!p) return;

    for (int i = 0; i < p->num_commands; i++) {
        Command *cmd = &p->commands[i];

        /* Free argv strings */
        for (int j = 0; cmd->argv[j] != NULL; j++) {
            free(cmd->argv[j]);
        }

        /* Free redirects */
        if (cmd->redirects) {
            for (int j = 0; j < cmd->num_redirects; j++) {
                if (cmd->redirects[j].filename) {
                    free(cmd->redirects[j].filename);
                }
            }
            free(cmd->redirects);
        }
    }

    free(p->commands);
    free(p);
}
