#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

int client_sockets[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *arg) {
  int client_socket = *(int *)arg;
  char buffer[BUFFER_SIZE];
  int valread;

  while ((valread = read(client_socket, buffer, BUFFER_SIZE)) > 0) {
    buffer[valread] = '\0';
    printf("Client %d: %s\n", client_socket, buffer);

    // Broadcast message to all other clients
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
      if (client_sockets[i] != 0 && client_sockets[i] != client_socket) {
        send(client_sockets[i], buffer, strlen(buffer), 0);
      }
    }
    pthread_mutex_unlock(&clients_mutex);
  }

  close(client_socket);
  pthread_mutex_lock(&clients_mutex);
  for (int i = 0; i < MAX_CLIENTS; ++i) {
    if (client_sockets[i] == client_socket) {
      client_sockets[i] = 0;
      break;
    }
  }
  pthread_mutex_unlock(&clients_mutex);
  free(arg);
  return NULL;
}

int main() {
  int server_fd, new_socket;
  struct sockaddr_in address;
  int addrlen = sizeof(address);

  // Initialize client sockets
  memset(client_sockets, 0, sizeof(client_sockets));

  // Creating socket file descriptor
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // Bind the socket to the network address and port
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  // Listen for incoming connections
  if (listen(server_fd, 3) < 0) {
    perror("listen");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  printf("Server listening on port %d\n", PORT);

  while (1) {
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             (socklen_t *)&addrlen)) < 0) {
      perror("accept");
      close(server_fd);
      exit(EXIT_FAILURE);
    }

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
      if (client_sockets[i] == 0) {
        client_sockets[i] = new_socket;
        pthread_t tid;
        int *pclient = malloc(sizeof(int));
        *pclient = new_socket;
        pthread_create(&tid, NULL, handle_client, pclient);
        break;
      }
    }
    pthread_mutex_unlock(&clients_mutex);
  }

  close(server_fd);
  return 0;
}
