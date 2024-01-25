// Server Code

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_CLIENTS 10
#define MAX_MESSAGE_LEN 1024

int clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void send_to_all_clients(char *message, int current_client) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i] != current_client) {
            if (send(clients[i], message, strlen(message), 0) < 0) {
                perror("Failed to send message");
                continue;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *handle_client(void *arg) {
    int client_socket = *((int *)arg);
    char message[MAX_MESSAGE_LEN];

    while (1) {
        int receive_size = recv(client_socket, message, MAX_MESSAGE_LEN, 0);
        if (receive_size <= 0) {
            close(client_socket);

            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < client_count; i++) {
                if (clients[i] == client_socket) {
                    for (int j = i; j < client_count - 1; j++) {
                        clients[j] = clients[j + 1];
                    }
                    break;
                }
            }
            client_count--;
            pthread_mutex_unlock(&clients_mutex);

            break;
        }
        message[receive_size] = '\0';
        printf("%s\n",message);
        send_to_all_clients(message, client_socket);
    }

    pthread_exit(NULL);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    pthread_t client_threads[MAX_CLIENTS];

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Socket bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("Socket listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port 8080...\n");

    while (1) {
        // Accept a connection
        int client_addr_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket == -1) {
            perror("Error accepting connection");
            continue;
        }

        // Add the client to the clients array
        pthread_mutex_lock(&clients_mutex);
        clients[client_count++] = client_socket;
        pthread_mutex_unlock(&clients_mutex);

        // Create a thread to handle the client
        pthread_create(&client_threads[client_count - 1], NULL, handle_client, &clients[client_count - 1]);
    }

    // Close the server socket
    close(server_socket);

    return 0;
}

