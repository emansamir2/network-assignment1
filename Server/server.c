#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define PORT 8888 //not used now 
#define BUFFER_SIZE 1000000  
#define RESPONSE_SIZE 1000000 

void handle_get_request(int client_fd, const char* request) {
    // Extract the requested file path from the request
    char file_path[256];
    // TODO: img - txt - html

    sscanf(request, "GET %s HTTP/1.1\r\n", file_path);
    printf("-------------------------------------------------------------\n");
    printf("I am in handle_get_request and the file path is %s\n", file_path);

    // Open the requested file
    FILE* file = fopen(file_path, "rb");
    if (file == NULL) {
        // If the file is not found, send a 404 Not Found response
        char response[RESPONSE_SIZE];
        snprintf(response, RESPONSE_SIZE, "HTTP/1.1 404 Not Found\r\n");
        send(client_fd, response, strlen(response), 0);
        return;
    }


    // Send the file content as the response
    char response[RESPONSE_SIZE];
    snprintf(response, RESPONSE_SIZE, "HTTP/1.1 200 OK\r\n"); // \n
       
    // Process optional headers (skip the request line and the mandatory Host header)
    const char *headers_start = strstr(request, "\r\n") + 2; /// Move to the start of headers
    char headers_copy[BUFFER_SIZE];
    strncpy(headers_copy, headers_start, sizeof(headers_copy));
    headers_copy[sizeof(headers_copy) - 1] = '\0';

    // Parse optional headers
    char *header_line = strtok(headers_copy, "\r\n");    
    while (header_line != NULL) {
        // Extract and process each header line
        char header[512];
        sscanf(header_line, "%511[^\r\n]\r\n", header);
        strcat(response, header);
        strcat(response, "\r\n");
        printf("In header while\n");
        printf("header = %s\n", header);
        printf("response = %s\n", response);
        header_line = strtok(NULL, "\r\n");
        printf("header_line = %s\n", header_line);
    }

    // End the headers
    strcat(response, "\r\n");
    printf("Response now = %s\n", response);
    size_t final_response_size = strlen(response);
    // Send the file content in chunks
    size_t LOCAL_BUFFER_SIZE = RESPONSE_SIZE - final_response_size;
    char buffer[LOCAL_BUFFER_SIZE];

    size_t bytes_read;
    if((bytes_read = fread(buffer, 1, LOCAL_BUFFER_SIZE, file)) > 0){
        size_t response_length = strlen(response);

        // Use memcpy to concatenate buffer to the response
        memcpy(response + response_length, buffer, bytes_read);

        // Update the response length
        response_length += bytes_read;

        // Send the updated response to the client
        ssize_t sent_Bytes= send(client_fd, response, response_length, 0);
        printf("sent bytes %zu\n",sent_Bytes);
        for (size_t i = 0; i < sent_Bytes; ++i) {
        printf("%02X ", (unsigned char)response[i]);
    }
    }
    // char header_response[RESPONSE_SIZE];
    // strcpy(header_response, response);
    // size_t bytes_read;
    // while ((bytes_read = fread(buffer, sizeof(char), LOCAL_BUFFER_SIZE, file)) > 0) {
    //     // send(client_fd, buffer, bytes_read, 0);
    //     // add the header files before sending the response each time
    //     strcpy(response, header_response);
    //     strcat(response, buffer);
    //     printf("In while sending response = %s\n", response);
    //     send(client_fd, response, strlen(response), 0);
    // }
    
    fclose(file);
}


void handle_post_request(int client_fd, const char* request, ssize_t bytesRead) {
    // Extract the requested file path from the request
    char file_path[256];

    // TODO: img - txt - html
    sscanf(request, "POST %s HTTP/1.1\r\n", file_path);
    printf("-------------------------------------------------------------\n");
    printf("I am in handle_post_request and the file path is %s\n", file_path);

    // Extract the data from the request body
    const char* body_start = strstr(request, "\r\n\r\n");
    if (body_start == NULL) {
        char response[RESPONSE_SIZE];
        snprintf(response, RESPONSE_SIZE, "HTTP/1.1 400 Bad Request\r\n");
        send(client_fd, response, strlen(response), 0);
        return;
    }

    const char* data = body_start + 4;  // Skip the "\r\n\r\n"
    printf("Received POST data: %s\n", data);

    FILE *file = fopen(file_path, "wb");//
    if (file == NULL) {
        // If the file is not found, send a 404 Not Found response
        char response[RESPONSE_SIZE];
        snprintf(response, RESPONSE_SIZE, "HTTP/1.1 404 Not Found\r\n");
        send(client_fd, response, strlen(response), 0);
        return;
    }

    // Append data to the file
    fwrite(data, sizeof(char), bytesRead - (data - request), file);
    // Close the file
    fclose(file);

    // Create a response
    char response[RESPONSE_SIZE];
    snprintf(response, RESPONSE_SIZE, "HTTP/1.1 200 OK\r\n");

    const char* header_start = request + strlen("POST") + strlen(file_path) + strlen("HTTP/1.1\r\n") + 2; // Move past the first \r\n after the request line
    while (strncmp(header_start, "\r\n", 2) != 0) {//not \r\n
        // Extract and process each header line
        char header[512];
        sscanf(header_start, "%511[^\r\n]\r\n", header);
        printf("Header: %s\n", header);
        strcat(response, header);
        strcat(response, "\r\n");

        // Move to the next line
        header_start = strstr(header_start, "\r\n") + 2;
    }

    strcat(response, "\r\n");
    printf("\nresponse: %s\n", response);

    // Send the response
    send(client_fd, response, strlen(response), 0);
}


int main( int argc, char *argv[] ) {

    // if (argc != 2) {
    //     fprintf(stderr, "Usage: %s port_number\n", argv[0]);
    //     return EXIT_FAILURE;
    // }

    char *port_number ="8888";

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if( server_fd == 0)   
    {   
        perror("socket failed");   
        exit(EXIT_FAILURE);   
    } 

    struct sockaddr_in address;
    address.sin_family = AF_INET;   
    address.sin_addr.s_addr = INADDR_ANY;   //htonl(INADDR_ANY)
    address.sin_port = htons(atoi(port_number)); 

    char buffer[BUFFER_SIZE];
    memset(buffer, '\0', BUFFER_SIZE);

    if (bind(server_fd, (struct sockaddr *) &address , sizeof(address)) < 0)
    {
        printf("Error! Bind has failed\n");
        exit(0);
    }
    printf("Listener on port %d \n", atoi(port_number));

    if (listen(server_fd, 3) < 0)
    {
        printf("Error! Can't listen\n");
        exit(0);
    }

    int addrlen = sizeof(address);   
    puts("Waiting for connections ..."); 

    char response[RESPONSE_SIZE];
    time_t last_operation;
    __pid_t pid = -1;

    while (1) {
        int client_fd = accept(server_fd, (struct sockaddr *) &address, (socklen_t*)&addrlen);

        pid = fork();

        if (pid == 0) {
            close(server_fd); //to be able to accept other connections

            if (client_fd < 0) {
                printf("Error! Can't accept\n");
                exit(0);
            }

            printf("Connection with %d has been established and delegated to the process %d.\nWaiting for a query...\n", client_fd, getpid());

            // to make the client_fd not blocking
            last_operation = clock();
            int flags = fcntl(client_fd, F_GETFL, 0);
            fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

            while (1) {
                ssize_t bytesRead = recv(client_fd, buffer, BUFFER_SIZE,0);
                
                // printf("buffer= %s\n",buffer);
                //to be changed
                if (strcmp(buffer ,"close\r\n") == 0) {
                    printf("Process %d: \n", getpid());
                    close(client_fd);
                    printf("Closing session with %d. Bye!\n", client_fd);
                    break;
                }
                else if (bytesRead <= 0) {
                    clock_t d = clock() - last_operation;
                    double dif = 1.0 * d / CLOCKS_PER_SEC;

                    if (dif > 30.0) {
                        printf("Process %d: \n", getpid());
                        close(client_fd);
                        printf("Connection timed out after %.3lf seconds. \n", dif);
                        printf("Closing session with %d. Bye!\n", client_fd);
                        break;
                    }

                }
                else {
                    //buffer[bytesRead] = '\0';  

                    printf("Process %d: \n", getpid());
                    printf("Received %s. Processing... \n", buffer);
                    fflush(stdout);
                    memset(response, '\0', RESPONSE_SIZE);
                    // Check if it's a GET or POST request
                    if (strncmp(buffer, "GET", 3) == 0) {
                        handle_get_request(client_fd, buffer);
                    }
                    else if (strncmp(buffer, "POST", 4) == 0) {
                        printf("hereeeeeeeee bytesRead %zu\n\n",bytesRead);
                        handle_post_request(client_fd, buffer,bytesRead);
                    }
                    else {
                        // Invalid request
                        char response[RESPONSE_SIZE];
                        snprintf(response, RESPONSE_SIZE, "HTTP/1.1 400 Bad Request\r\n"); // \r\nInvalid request
                        send(client_fd, response, strlen(response), 0);
                    }

                    memset(buffer, '\0', BUFFER_SIZE);
                    last_operation = clock();
                }
            }
            close(client_fd);
            exit(0);
        }
        else {
            close(client_fd);
        }
    }
    close(server_fd); // Close the server socket in the parent process
    return 0;
}
