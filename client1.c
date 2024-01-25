// Client Code

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_MESSAGE_LEN 1024

void *receive_messages(void *arg) {
    int client_socket = *((int *)arg);
    char message[MAX_MESSAGE_LEN];

    while (1) {
        int receive_size = recv(client_socket, message, MAX_MESSAGE_LEN, 0);
        if (receive_size <= 0) {
            perror("Server disconnected");
            close(client_socket);
            exit(EXIT_FAILURE);
        }

        message[receive_size] = '\0';
        printf("Received: %s", message);
        fflush(stdout);
    }

    pthread_exit(NULL);
}

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    pthread_t receive_thread;

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Create a thread to receive messages
    pthread_create(&receive_thread, NULL, receive_messages, &client_socket);

    char message[MAX_MESSAGE_LEN];

    // Send and receive messages
    while (1) {
        printf("Enter message: ");
        fgets(message, MAX_MESSAGE_LEN, stdin);

        if (send(client_socket, message, strlen(message), 0) < 0) {
            perror("Failed to send message");
            break;
        }
    }

    // Close the client socket
    close(client_socket);

    return 0;
}

