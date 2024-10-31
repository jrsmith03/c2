#include "includes.h"

#define AES256 1

// argv[1] = server IP
// argv[2] = server port
// argv[3] = username
// argv[4] = password
// argv[5] = location of AES key file

int main(int argc, char** argv) {
    // Establish an IPV4 socket file descriptor using TCP
    if (argc < 4) {
        printf("Usage: client <server IP> <server port> <username> <password> <aes>\n");
        return -1;
    }
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
    
    unsigned char key[32] = "B8BF20A598E86D623CB26D2D81DD0761227632BC533635C9C85FF2C1518A7B99";
    unsigned char iv[16] = "C3C006666DF105DD08EB05EE788C5711";


    if (connect(sockfd, &addr, sizeof(addr)) == -1) {
        printf("Connect failed! Are you running the server on port %d? Error: %s\n", port, strerror(errno));
        return -1;
    } else {
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
        char response[16];
        read(sockfd, response, 16);

        if (strcmp(response, "OK") != 0) {
            printf("Authentication failed! Please try a different username and password.\n");
            return -1;
        }
        printf("Welcome, %s.\n", argv[3]);

        while(1) {
            char cmd[READ_LENGTH]; 
            fgets(cmd, READ_LENGTH, stdin);
            if (cmd != EOF) {
                AES_init_ctx_iv(&ctx, key, iv);
                AES_CTR_xcrypt_buffer (&ctx, cmd, READ_LENGTH);
                write(sockfd, cmd, READ_LENGTH);
            }
            // Read the potential command output from server
            char* server_out = (char*)calloc(sizeof(char), OUTPUT_BUFFER);
            
            if (read(sockfd, server_out, OUTPUT_BUFFER) < 0) {
                printf("Error while reading from the server output buffer.\n");
            }
            AES_init_ctx_iv(&ctx, key, iv);
            AES_CTR_xcrypt_buffer (&ctx, server_out, sizeof(server_out));

            // Display the server output to client console
            printf("%s", server_out);
            free(server_out);
        }
        close(sockfd);
    }
}


