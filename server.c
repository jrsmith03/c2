#include "includes.h"

int main(int argc, char** argv) {
    
    // Establish an IPV4 socket file descriptor using TCP
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // First argument is the address for which to bind the socket to.
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = atoi(argv[1]);
    struct in_addr sin_addr;
    sin_addr.s_addr = INADDR_ANY; 
    addr.sin_addr = sin_addr; 

    struct AES_ctx ctx;

    unsigned char key[32] = "B8BF20A598E86D623CB26D2D81DD0761227632BC533635C9C85FF2C1518A7B99";
    unsigned char iv[16] = "C3C006666DF105DD08EB05EE788C5711";

    // TODO: obtain these from the config file
    char auth_usr[MAX_USERNAME_LEN] = "jrsmith";
    char auth_pwd[MAX_PASSWORD_LEN] = "password";


    if (bind(sockfd, &addr, sizeof(addr)) == -1) {
        printf("Binding failed!\n");
    }
    
    // Now we want to make the sockfd a passive socket
    if (listen(sockfd, 255) == -1) {
        printf("Listen failed!\n");
    }
    printf("Server successfully established! Waiting for incoming connections...\n");

    // Loop indefinitely until the server recieves SIGINT
    int client_fd;
    while (1) {
        socklen_t addr_len = sizeof(addr);
        int client_fd = accept(sockfd, &addr, &addr_len);
        if (client_fd == -1) {
            printf("Error: unable to accept the client.");
            return -1;
        }
        // Authentication
        char attempt_usr[MAX_USERNAME_LEN];
        char attempt_pwd[MAX_PASSWORD_LEN];

        read(client_fd, attempt_usr, MAX_USERNAME_LEN);
        read(client_fd, attempt_pwd, MAX_PASSWORD_LEN);

        // Decrypt the auth fields
        AES_init_ctx_iv(&ctx, key, iv);
        AES_CTR_xcrypt_buffer (&ctx, attempt_usr, sizeof(attempt_usr));  // decrypt client username
        AES_init_ctx_iv(&ctx, key, iv);
        AES_CTR_xcrypt_buffer (&ctx, attempt_pwd, sizeof(attempt_pwd));  // decrypt client password
        printf("  auth: provided user %s\n  auth: provided password %s\n", attempt_usr, attempt_pwd);
        if (strcmp(attempt_usr, auth_usr) == 0 && strcmp(attempt_pwd, auth_pwd) == 0) {
            printf("Client is successfully authenticated.\n");
            write(client_fd, "OK", 16);
        } else {
            write(client_fd, "INVALID", 16);
            printf("Error authenticating with the client, closing connection.\n");
            close(client_fd);
            continue;
        }

        printf("Client connected on fd %d.\n", client_fd);
        // Read 32 bytes from the client, presumably to execute a command.
        char client_command[READ_LENGTH];
        read(client_fd, client_command, READ_LENGTH);
        AES_init_ctx_iv(&ctx, key, iv);
        AES_CTR_xcrypt_buffer (&ctx, client_command, sizeof(client_command)); // decrypt client command
        while (client_command != NULL) {
            // Spawn a new child process. system command allows for args to be passed in seemlessly
            // Specifically, it spawns a child process of /bin/sh that runs any command.
            char* output = (char*)calloc(sizeof(char), 4096);

            // We need to manually run the chdir command due to the nature of the system() function.
            // NOTE/todo: Required to have an extra space after the command name.
            char cmd_cpy[READ_LENGTH];
            char* check_tok;
            strncpy(cmd_cpy, client_command, sizeof(client_command));
            check_tok = strtok(cmd_cpy, " ");
            printf("%s\n", check_tok);

            if (check_tok && strncmp(check_tok, "cd", 2) == 0) {
                check_tok = strtok(NULL, " ");
                printf("%s\n", check_tok);
                printf("Changing the working directory to %s\n", check_tok);
                chdir(check_tok);
            } 
            int pipe_fd[2];
            pipe(pipe_fd);
            printf("Execute: %s\n", client_command);
            int stdout_cpy = dup(STDOUT_FILENO);
            dup2(pipe_fd[1], STDOUT_FILENO);
            int status = system(client_command);   
            close(pipe_fd[1]);
            dup2(stdout_cpy, STDOUT_FILENO);
            // Clear out the array
            if (read(pipe_fd[0], output, 4096) < 0){
                printf("Error while reading from stdout buffer.\n");
            }
            
            // Write the output buffer to the client
            AES_init_ctx_iv(&ctx, key, iv);
            AES_CTR_xcrypt_buffer (&ctx, output, sizeof(output)); // encrypt terminal output
            if (write(client_fd, output, 4096) < 0) {
                printf("Error while writing to client. %s\n",strerror(errno));
            }
            free(output);
            close(pipe_fd[0]);
            // Obtain a new command from client, or NULL if client is disconnected
            read(client_fd, client_command, READ_LENGTH);
            AES_init_ctx_iv(&ctx, key, iv);
            AES_CTR_xcrypt_buffer (&ctx, client_command, sizeof(client_command)); // decrypt client command
        }
    }
}

