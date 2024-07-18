#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void *receive_messages(void *sock) {
  int client_socket = *(int *)sock;
  char buffer[BUFFER_SIZE];
  int valread;

  while ((valread = read(client_socket, buffer, BUFFER_SIZE)) > 0) {
    buffer[valread] = '\0';
    printf("Server: %s\n", buffer);
  }

  return NULL;
}

int main() {
  int sock = 0;
  struct sockaddr_in serv_addr;
  char buffer[BUFFER_SIZE] = {0};

  // Creating socket file descriptor
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("\n Socket creation error \n");
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  // Convert IPv4 and IPv6 addresses from text to binary form
  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  // Connect to the server
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    printf("\nConnection Failed \n");
    return -1;
  }

  // Create a thread to receive messages from the server
  pthread_t recv_thread;
  pthread_create(&recv_thread, NULL, receive_messages, &sock);

  // Send messages to the server
  while (1) {
    printf("You: ");
    fgets(buffer, BUFFER_SIZE, stdin);
    send(sock, buffer, strlen(buffer), 0);
  }

  close(sock);
  return 0;
}
