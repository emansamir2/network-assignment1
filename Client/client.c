#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define BUFFER_SIZE 1000000  
#define RESPONSE_SIZE 1000000
// #define LOCALHOST "/home/eman/Documents/network-assignment1"
#define LOCALHOST "/home/maria/Documents/Networks/network-assignment1"


void extract_filename(const char *file_path, char *filename) {
    const char *last_slash = strrchr(file_path, '/');
    
    if (last_slash != NULL) {
        // Increment the pointer to move past the '/'
        last_slash++;
        strcpy(filename, last_slash);
    } else {
        // No '/' found, the entire path is the filename
        strcpy(filename, file_path);
    }
}

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
    
    char request[BUFFER_SIZE];
    snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\n", file_path);
    ssize_t sent_Bytes= send(client_fd, request, strlen(request), 0);
    strcat(request,"\r\n");
    if(sent_Bytes<0){
        perror("ERROR while sending to socket");
        return;
    }
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    printf("%s\n\n", buffer);

    // Create a file using the file path as the name
    char filename[256];
    extract_filename(file_path, filename);
    char output_file_path [256];
    snprintf(output_file_path, sizeof(output_file_path), "%s/Client/%s", LOCALHOST, filename);
    
    const char *body_start = strstr(buffer, "\r\n\r\n");
    if (body_start != NULL) {
        // Move past the "\r\n\r\n"
        body_start += 4;
    
        FILE *file = fopen(output_file_path, "wb");
        if (file == NULL) {
            perror("ERROR opening file");
            return;//exit(1);
        }
        // Write the received data to the file
        fwrite(body_start, sizeof(char), bytes_received - (body_start - buffer), file);

        // Close the file
        fclose(file);
    }
    

}

void client_post(int client_fd, char file_path[],char host_name [],char port_number []){
    printf("I am in client post\n");
    char request[BUFFER_SIZE];
    char filename[256];
    extract_filename(file_path, filename);
    char output_file_path [256];
    snprintf(output_file_path, sizeof(output_file_path), "%s/Server/%s", LOCALHOST, filename);
    // Build the POST request
    snprintf(request, sizeof(request), "POST %s HTTP/1.1\r\n", output_file_path);
    strcat(request,"\r\n");
    // body
    char buffer[BUFFER_SIZE];
    memset(buffer, '\0', BUFFER_SIZE);
    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        perror("ERROR opening file");
        return; 
    }
    // Write the received data to the file
    memset(buffer,'\0',BUFFER_SIZE);
    size_t bytesRead = fread(buffer, 1, BUFFER_SIZE, file);
    printf("bytesRead fread  %zu\n",bytesRead);
    
    printf("\n");
    // Close the file
    fclose(file);
    size_t requestLength = strlen(request);
    memcpy(request + requestLength, buffer, bytesRead);
    for (size_t i = 0; i < 1600; ++i) {
        printf("%02X ", (unsigned char)request[i]);
    }
    ssize_t sent_Bytes= send(client_fd, request, requestLength + bytesRead, 0);
    printf("sent bytes %zu\n",sent_Bytes);
    if(sent_Bytes<=0){
        perror("ERROR while sending to socket");
        return;
    }

    char response[RESPONSE_SIZE];
    ssize_t bytes_received = recv(client_fd, response, sizeof(buffer) - 1, 0);
    printf("recived bytes %zu \n",bytes_received);
    if(bytes_received<0){
        perror("ERROR while receiving to socket");
        return;
    }
    response[bytes_received] = '\0';

    printf("response = %s\n", response);

    const char *expected_pattern = "HTTP/1.1 200 OK";
    if(strncmp(response, expected_pattern, strlen(expected_pattern)) == 0){
        if (remove(file_path) == 0) {
        printf("File deleted successfully from client.\n");
        } else {
            perror("Error deleting file");
        }
    }
}


int main( int argc, char *argv[] ) {

    if (argc != 3) {
        fprintf(stderr, "Usage: %s server_ip port_number\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *server_ip = "10.0.2.15";
    char *port_n = "8888";

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

    start_connection(client_fd,server_ip,port_n);

    while (fscanf(input_file, "%s %s %s %s", command, file_path, host_name, port_number) == 4) {
        //TODO: check if the connection is open
        if (strcmp(command, "client_get") == 0) {
            // Handle GET command
            // file_path --> in the server
            client_get(client_fd ,file_path, host_name, port_number); // --> create a new file at the client
        }
        else if (strcmp(command, "client_post") == 0) {
            // Handle POST command
            client_post(client_fd ,file_path, host_name, port_number); // --> send the data to the server
        }
        else {
            fprintf(stderr, "Unknown command: %s\n", command);
        }
    }
    
    close(client_fd);

    fclose(input_file); // Close the input file

    return 0;
}