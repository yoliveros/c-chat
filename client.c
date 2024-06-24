#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <wchar.h>

#define SERVER_ADDRESS "127.0.0.1"
#define PORT 8080

typedef struct message {
  char user[1024];
  char content[1024];
  wchar_t stream_content;
} message_t;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Missing argument username\n");
    exit(EXIT_FAILURE);
  }

  int client_socket;
  client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (client_socket == -1) {
    perror("Failed to create socket");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(PORT);
  server_address.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);

  int connect_result =
      connect(client_socket, (struct sockaddr *)&server_address,
              sizeof(server_address));

  if (connect_result == -1) {
    perror("Failed to connect to server");
    exit(EXIT_FAILURE);
  }

  printf("Connected to server\n");

  message_t message_send;
  strcpy(message_send.user, argv[1]);

  while (1) {
    memset(message_send.content, 0, sizeof(message_send.content));

    if (strcmp(message_send.content, "!quit") == 0) {
      printf("exiting...\n");
      break;
    }

    fgetws(&message_send.stream_content, sizeof(message_send.content), stdin);
    message_send.content[strcspn(message_send.content, "\n")] = '\0';

    // Send message to server with username
    int send_result =
        send(client_socket, &message_send, sizeof(message_send), 0);
    if (send_result == -1) {
      perror("Failed to send message");
      exit(EXIT_FAILURE);
    }

    printf("You: %s\n", message_send.content);

    // Receive message from server
    message_t message_recv;
    int recv_result =
        recv(client_socket, &message_recv, sizeof(message_recv), 0);
    if (recv_result == -1) {
      perror("Failed to receive message");
      exit(EXIT_FAILURE);
    } else if (recv_result == 0) {
      printf("Server disconnected\n");
      break;
    }

    printf("%s: %s\n", message_recv.user, message_recv.content);
  }

  close(client_socket);
  return 0;
}
