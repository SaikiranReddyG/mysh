#include "shell.h"

/**
 * apply_redirects() - Apply all file redirections to the current process
 * 
 * For each redirect in the command, opens the target file and uses dup2()
 * to replace stdin/stdout/stderr with the file descriptor.
 * 
 * Must be called in child process before execvp().
 * 
 * Returns: 0 on success, -1 on error
 */
int apply_redirects(const Command *cmd)
{
    if (!cmd) return 0;

    for (int i = 0; i < cmd->num_redirects; i++) {
        Redirect *r = &cmd->redirects[i];
        int fd;

        switch (r->type) {
        case REDIR_STDOUT:
            /* Output redirect: > file (truncate) */
            fd = open(r->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1) {
                perror(r->filename);
                return -1;
            }
            if (dup2(fd, STDOUT_FILENO) == -1) {
                perror("dup2");
                close(fd);
                return -1;
            }
            close(fd);
            break;

        case REDIR_STDOUT_APPEND:
            /* Append redirect: >> file (append) */
            fd = open(r->filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd == -1) {
                perror(r->filename);
                return -1;
            }
            if (dup2(fd, STDOUT_FILENO) == -1) {
                perror("dup2");
                close(fd);
                return -1;
            }
            close(fd);
            break;

        case REDIR_STDIN:
            /* Input redirect: < file (read) */
            fd = open(r->filename, O_RDONLY);
            if (fd == -1) {
                perror(r->filename);
                return -1;
            }
            if (dup2(fd, STDIN_FILENO) == -1) {
                perror("dup2");
                close(fd);
                return -1;
            }
            close(fd);
            break;

        case REDIR_STDERR:
            /* Stderr redirect: 2> file */
            fd = open(r->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1) {
                perror(r->filename);
                return -1;
            }
            if (dup2(fd, STDERR_FILENO) == -1) {
                perror("dup2");
                close(fd);
                return -1;
            }
            close(fd);
            break;

        case REDIR_NONE:
            /* No redirect */
            break;
        }
    }

    return 0;
}

