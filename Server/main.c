#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>

#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define PORT 8888  
#define BUFFER_SIZE 8  
#define RESPONSE_SIZE 8 



int main( int argc, char *argv[] ) {

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if( server_fd == 0)   
    {   
        perror("socket failed");   
        exit(EXIT_FAILURE);   
    } 

    struct sockaddr_in address;
    address.sin_family = AF_INET;   
    address.sin_addr.s_addr = INADDR_ANY;   //htonl(INADDR_ANY)
    address.sin_port = htons( PORT ); 

    char buffer[BUFFER_SIZE];
    memset(buffer, '\0', BUFFER_SIZE);

    if (bind(server_fd, (struct sockaddr *) &address , sizeof(address)) < 0)
    {
        printf("Error! Bind has failed\n");
        exit(0);
    }
    printf("Listener on port %d \n", PORT);

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

            last_operation = clock();
            int flags = fcntl(client_fd, F_GETFL, 0);
            fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

            while (1) {
                int bytesRead = read(client_fd, buffer, BUFFER_SIZE);
                // printf("buffer= %s\n",buffer);
                //to be changed
                if (strcmp(buffer ,"close\r\n")==0) {
                    printf("Process %d: \n", getpid());
                    close(client_fd);
                    printf("Closing session with %d. Bye!\n", client_fd);
                    break;
                }
                else if (bytesRead <= 0) {
                    clock_t d = clock() - last_operation;
                    double dif = 1.0 * d / CLOCKS_PER_SEC;

                    if (dif > 5.0) {
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
                    //todo create the appropriate response
                    strcpy(response,buffer);
                    memset(buffer, '\0', BUFFER_SIZE);
                    send(client_fd, response, strlen(response), 0);
                    printf("Responded with %s. Waiting for a new query...\n", response);
                    fflush(stdout);
                    last_operation = clock();
                    
                }
            }
            
            exit(0);
        }
        else {
            close(client_fd);
        }
    }
}
