#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define MAX_CLIENTS 10

typedef struct message {
  char user[1024];
  char content[1024];
} message_t;

int main() {
  int server_socket;

  int client_sockets[MAX_CLIENTS] = {0};

  fd_set readfds;

  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == -1) {
    perror("Failed to create socket");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(PORT);
  server_address.sin_addr.s_addr = htonl(INADDR_ANY);

  int bind_result = bind(server_socket, (struct sockaddr *)&server_address,
                         sizeof(server_address));
  if (bind_result == -1) {
    perror("Failed to bind socket");
    exit(EXIT_FAILURE);
  }

  if (listen(server_socket, MAX_CLIENTS) == -1) {
    perror("Failed to listen for connections");
    exit(EXIT_FAILURE);
  }

  printf("Server listening on port %d\n", PORT);

  while (1) {
    FD_ZERO(&readfds);
    FD_SET(server_socket, &readfds);

    int max_socket = server_socket;
    for (int i = 0; i < MAX_CLIENTS; i++) {
      int client_socket = client_sockets[i];
      if (client_socket > 0) {
        FD_SET(client_socket, &readfds);
        if (client_socket > max_socket) {
          max_socket = client_socket;
        }
      }
    }

    int activity = select(max_socket + 1, &readfds, NULL, NULL, NULL);
    if (activity < 0) {
      perror("Select failed");
      exit(EXIT_FAILURE);
    }

    if (FD_ISSET(server_socket, &readfds)) {
      int client_socket = accept(server_socket, NULL, NULL);
      if (client_socket == -1) {
        perror("Failed to accept client connection");
        continue;
        // exit(EXIT_FAILURE);
      }

      for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] == 0) {
          client_sockets[i] = client_socket;
          break;
        }
      }
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
      int client_socket = client_sockets[i];
      if (FD_ISSET(client_socket, &readfds)) {
        message_t message_recv;
        int recv_result =
            recv(client_socket, &message_recv, sizeof(message_recv), 0);

        if (recv_result == -1) {
          perror("Failed to receive message");
          close(client_socket);
          client_sockets[i] = 0;
        } else if (recv_result == 0) {
          printf("Client %d disconnected\n", i);
          close(client_socket);
          client_sockets[i] = 0;
        } else {
          for (int j = 0; j < MAX_CLIENTS; j++) {
            int other_socket = client_sockets[j];
            if (other_socket > 0) {
              printf("%s: %s\n", message_recv.user, message_recv.content);
              int send_result =
                  send(other_socket, &message_recv, sizeof(message_recv), 0);

              if (send_result == -1) {
                perror("Failed to send message");
              }
            }
          }
        }
      }
    }
  }

  close(server_socket);
  return 0;
}
