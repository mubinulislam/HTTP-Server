#include <stdio.h>      // For input/output functions like printf(), perror()
#include <stdlib.h>     // For functions like exit() and malloc()
#include <string.h>     // For string operations like memset(), strcpy()
#include <unistd.h>     // For close() function
#include <arpa/inet.h>  // For network functions like socket(), bind(), listen(), accept()
#include <pthread.h>    // For multi-threading

#define PORT 8080            // The port number where our server will listen
#define BUFFER_SIZE 1024      // The size of the buffer to store client requests

// Function to handle communication with a single client
void *handle_client(void *client_socket_ptr) {
    int client_socket = *(int *)client_socket_ptr;  // Get socket file descriptor
    free(client_socket_ptr);  // Free the memory allocated for the socket

    char buffer[BUFFER_SIZE];  // Buffer to store client request
    memset(buffer, 0, BUFFER_SIZE);  // Initialize buffer with zeros

    // Read data from the client (HTTP request)
    read(client_socket, buffer, BUFFER_SIZE);
    printf("Received request:\n%s\n", buffer);

    // Create a simple HTTP response
    char response[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n\r\n"
        "<html><body><h1>Hello, World!</h1></body></html>";

    // Send the response to the client
    write(client_socket, response, strlen(response));

    // Close the connection
    close(client_socket);
    printf("Connection closed.\n");

    // End the thread
    pthread_exit(NULL);
}

int main() {
    int server_fd;  // The server socket file descriptor
    struct sockaddr_in server_addr, client_addr;  // Structures to store address info
    int addrlen = sizeof(client_addr);

    // 1. Create a socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully.\n");

    // 2. Set up server address structure
    server_addr.sin_family = AF_INET;  // Use IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Accept connections from any IP
    server_addr.sin_port = htons(PORT);  // Convert port to network byte order

    // 3. Bind the socket to the address and port
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Bind successful.\n");

    // 4. Start listening for incoming connections
    if (listen(server_fd, 10) < 0) {  // Maximum 10 clients in queue
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Server is listening on port %d...\n", PORT);

    // 5. Accept and handle incoming connections in a loop
    while (1) {
        int *client_socket = malloc(sizeof(int));  // Allocate memory for client socket
        *client_socket = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen);
        
        if (*client_socket < 0) {
            perror("Accept failed");
            free(client_socket);  // Free allocated memory
            continue;  // Continue to the next iteration
        }
        printf("New connection accepted.\n");

        // 6. Create a new thread to handle the client
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, client_socket) != 0) {
            perror("Thread creation failed");
            free(client_socket);
        }
        pthread_detach(thread_id);  // Free resources automatically when the thread finishes
    }

    // 7. Close the server socket (not reached in infinite loop)
    close(server_fd);
    return 0;
}