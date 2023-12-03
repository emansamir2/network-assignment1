#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
// #include <curl/curl.h>

void start_connection(int client_fd,char host_name [],char port_number []){

    struct hostent *server = gethostbyname(host_name);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    struct in_addr** addr_list = (struct in_addr**)server->h_addr_list;
    if (addr_list[0] != NULL) {
        server_address.sin_addr = *addr_list[0];
    } else {
        fprintf(stderr, "No address found\n");
        return ;
    }
    server_address.sin_port = htons(atoi(port_number));

    if (connect(client_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("ERROR while connecting");
        exit(1);
    }
}

void client_get(int client_fd, char file_path[],char host_name [],char port_number []){
    
}


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

    int client_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (client_fd == -1) {
            perror("socket");
            exit(EXIT_FAILURE);
        }

    while (fscanf(input_file, "%s %s %s %s", command, file_path, host_name, port_number) == 4) {
        if (strcmp(command, "client_get") == 0) {
            // Handle GET command
            // file_path --> in the server
            client_get(client_fd ,file_path, host_name, port_number); // --> create a new file at the client
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