#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

// Error function used for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(1);
}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address,
                        int portNumber){

  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address));

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char *argv[]){
  signal(SIGCHLD, SIG_IGN);
  int connectionSocket;
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);

  // Check usage & args
  if (argc < 2) {
    fprintf(stderr,"USAGE: %s port\n", argv[0]);
    exit(1);
  }

  // Create the socket that will listen for connections
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) {
    error("ERROR opening socket");
  }

  // Set up the address struct for the server socket
  setupAddressStruct(&serverAddress, atoi(argv[1]));

  // Associate the socket to the port
  if (bind(listenSocket,
          (struct sockaddr *)&serverAddress,
          sizeof(serverAddress)) < 0){
    error("ERROR on binding");
  }

  // Start listening for connections. Allow up to 5 connections to queue up
  listen(listenSocket, 5);

  // Accept a connection, blocking if one is not available until one connects
  while(1){
    // Accept the connection request which creates a connection socket
    connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);

    if (connectionSocket < 0){
      error("ERROR on accept");
      continue;
    }
    pid_t pid = fork();
    if (pid < 0) {
      perror("ERROR on fork");
      close(connectionSocket);
    }
    if (pid == 0) {
      close(listenSocket);

      char input_line[32];
      int length_tracker = 0;
      char input_character;
      while (length_tracker < (int)sizeof(input_line) - 1) {
        recv(connectionSocket, &input_character, 1, 0);
        if (input_character == '\n') {
          break;
        }
        input_line[length_tracker++] = input_character;
      }
      input_line[length_tracker] = '\0';
      int input_text_length = atoi(input_line);
      char *input_text = malloc(input_text_length);
      char *encoded_text = malloc(input_text_length);
      char *key = malloc(input_text_length);

      int data_received = 0;
      while (data_received < input_text_length) {
        ssize_t data = recv(connectionSocket,input_text + data_received,input_text_length - data_received,0);
        data_received = data_received + data;
      }
      data_received = 0;
      while (data_received < input_text_length) {
        ssize_t data = recv(connectionSocket,key + data_received,input_text_length - data_received,0);
        data_received = data_received + data;
      }

      for (int i = 0; i < input_text_length; i++) {
        char input_char = input_text[i];
        int input_char_value;
        if (input_char == ' ') {
          input_char_value = 26;
        } else {
          input_char_value = input_char - 'A';
        }
        char key_char = key[i];
        int key_char_value;
        if (key_char == ' ') {
          key_char_value = 26;
        } else {
          key_char_value = key_char - 'A';
        }
        int encoded_value = (input_char_value + key_char_value) % 27;
        if (encoded_value == 26) {
          encoded_text[i] = ' ';
        } else {
          encoded_text[i] = 'A' + encoded_value;
        }
      }

      size_t sent_counter = 0;
      while (sent_counter < input_text_length) {
        ssize_t data_out = send(connectionSocket,encoded_text + sent_counter,input_text_length - sent_counter,0);
        sent_counter = sent_counter + data_out;
      }
      close(connectionSocket);
      free(input_text);
      free(encoded_text);
      free(key);
      exit(0);
    }
    close(connectionSocket);

  }
}
