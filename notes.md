unix shell written in C 
8 source files structured around REPL (read - eval - print loop)

here is how the repl() loop works per iteration:

print mysh> = read a line (EOF = ctrl + D = break)

strip trailing \n skip empty lines

histroy_add() - store command

branch = if single command & is_builtin() = execute_builtin()
        else if num_commands > 1 -execute_pipelien()
        else - execute_command() (single external)

background(&) - add_job() + print pid 
foreground - waitpid()

free pipeline 