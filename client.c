#include "includes.h"

// argv[0] = server IP
// argv[1] = server port
// argv[2] = username
// argv[3] = password
int main(int argc, char** argv) {
    // Establish an IPV4 socket file descriptor using TCP
    if (argc != 4) {
        printf("Usage: client <server IP> <server port> <username> <password>\n");
        return -1;
    }
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

    if (connect(sockfd, &addr, sizeof(addr)) == -1) {
        printf("Connect failed! Are you running the server on port %d? Error: %s\n", port, strerror(errno));
        return -1;
    } else {
        // Authenticate with the server
        write(sockfd, argv[2], MAX_USERNAME_LEN);
        write(sockfd, argv[3], MAX_PASSWORD_LEN);
        // Expect a response of OK or INVALID
        char response[16];
        read(sockfd, response, 16);
        if (strcmp(response, "OK") != 0) {
            printf("Authentication failed! Please try a different username and password.\n");
            return -1;
        }
        printf("Welcome, %s.\n", argv[2]);

        while(1) {
            char cmd[32]; 
            fgets(cmd, 32, stdin);
            if (cmd != EOF) {
                write(sockfd, cmd, 32);
            }
            // Read the potential command output from server
            char* server_out = (char*)calloc(sizeof(char), 4096);
            if (read(sockfd, server_out, 4096) < 0) {
                printf("Error while reading from the server output buffer.\n");
            }
            // Display the server output to client console
            printf("%s", server_out);
            free(server_out);
        }
        close(sockfd);
    }
}


