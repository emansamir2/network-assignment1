#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <curl/curl.h>

int main() {
    FILE *input_file = fopen("commands.txt", "r"); // Open the input file

    if (!input_file) {
        fprintf(stderr, "Error opening input file.\n");
        return -1;
    }

    char command[256];
    char file_path[256];
    char host_name[256];
    char port_number[6]; // Maximum port number length is 5 digits

    while (fscanf(input_file, "%s %s %s %s", command, file_path, host_name, port_number) == 4) {
        if (strcmp(command, "client_get") == 0) {
            // Handle GET command
            // file_path --> in the server
            client_get(file_path, host_name, port_number); // --> create a new file at the client
        } else if (strcmp(command, "client_post") == 0) {
            // Handle POST command
            client_post(file_path, host_name, port_number); // --> send the data to the server
        } else {
            fprintf(stderr, "Unknown command: %s\n", command);
        }
    }

    fclose(input_file); // Close the input file

    return 0;
}