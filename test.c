// test.c
#define MAX_ARGS 5
#define MAX_CMD_LEN 32
#include "server.h"

// void get_command_arg_array(char** args, char* token, int* num_args);

int main(int argc, char** argv) {
    char cmd[20] = "cd .. ; pwd\0";
    char cmd_cpy[20];
    char* check_tok;
    strncpy(cmd_cpy, cmd, sizeof(cmd));
    check_tok = strtok(cmd, " ");
    printf("%s\n", check_tok);

    if (check_tok && strncmp(check_tok, "cd", 2) == 0) {
        check_tok = strtok(NULL, " ");
        printf("%s\n", check_tok);
        printf("Changing the working directory...");
        chdir(check_tok);
    } 
    system(cmd_cpy);
    
    // get_command_arg_array(args, token, num_args);
    // printf("command name: %s\n", args[0]);
    // args[*num_args + 1] = "\0";
    // for (int i = 1; i < *num_args + 1; i++) {
    //     printf("arg %d: %s\n", i, args[i]);
    // }

}

// void get_command_arg_array(char** args, char* token, int* num_args) {

//     strtok(token, " ");
//     int i = 0;

//     while (token) {
//         args[i] = (char*)malloc(sizeof(token));
//         args[i] = token;
//         printf("arg %d is %s\n", i, args[i]); 
//         token = strtok(NULL, " ");
//         i++;
//     }
//     *num_args = i;
// }




    // pid_t shell_process = fork();
    // printf("   Fork new process %d to execute command %s\n", shell_process, argv[1]);

    // if (shell_process == -1) {
    //     printf("Unable to spawn child process.");
    //     return -1;
    // } else if (shell_process > 0) {
    //     // Parent code
    //     int status = 0;
    //     waitpid(shell_process, &status, NULL);     // Wait for execution to finish before requesting new commands.
    //     printf("Child %d finished with exit code %d\n", shell_process, status);
    // } else {
    //     // Child code
    //     // printf("Child %d attempting to exec system program %s", shell_process, cmd);
    //     execvp(args[0], args);

    // }