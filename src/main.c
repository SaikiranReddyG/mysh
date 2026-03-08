#include <stdio.h>

int main(void)
{
    char buffer[1024];

    while (1) {
        printf("mysh> ");
        fflush(stdout);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break; // Ctrl+D / EOF
        }

        printf("%s", buffer);
    }

    return 0;
}
