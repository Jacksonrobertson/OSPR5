#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

// Error function used for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(1);
}

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address,
                        int portNumber,
                        char* hostname){

  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address));

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);

  // Get the DNS entry for this host name
  struct hostent* hostInfo = gethostbyname(hostname);
  if (hostInfo == NULL) {
    fprintf(stderr, "CLIENT: ERROR, no such host\n");
    exit(1);
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char*) &address->sin_addr.s_addr,
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

int main(int argc, char *argv[]) {
  //significant socket code sourced from socket modules in canvas
  int socketFD, charsWritten, charsRead;
  struct sockaddr_in serverAddress;
  char buffer[256];
  if (argc != 4) {
    fprintf(stderr,"USAGE: %s hostname port\n", argv[0]);
    exit(1);
  }
  //==if (argc < 4) {
    //fprintf(stderr,"USAGE: %s key error\n", argv[0]);
    //exit(1);
  //}

  FILE *input_file = fopen(argv[1], "r");
  if (input_file == NULL) {
    fprintf(stderr,"CLIENT: ERROR, Opening Input File\n");
    exit(1);
  }

  FILE *key_file = fopen(argv[2], "r");
  if (key_file == NULL) {
    fprintf(stderr,"CLIENT: ERROR, Opening Key File\n");
    fclose(input_file);
    exit(1);
  }

  fseek(input_file, 0, SEEK_END);
  int input_file_length = ftell(input_file);
  rewind(input_file);

  char input_buffer [input_file_length+1];

  for (int i = 0; i < input_file_length; i++) {
    int input_char = fgetc(input_file);
    if (input_char == EOF) {
      fprintf(stderr,"CLIENT: ERROR, KEY SIZE NOT CORRECT\n");
      exit(1);
    }
    input_buffer[i] = input_char;
  }
  input_buffer[input_file_length] = '\0';

  fseek(key_file, 0, SEEK_END);
  int key_length = ftell(key_file);
  rewind(key_file);

  char key_buffer[key_length+1];

  for (int i = 0; i < key_length; i++) {
    int key_char = fgetc(key_file);
    if (key_char == EOF) {
      fprintf(stderr,"ERROR\n");
      exit(1);
    }
    key_buffer[i] = key_char;
  }
  key_buffer[key_length] = '\0';

  if (key_length < input_file_length) {
    fprintf(stderr, "Error: key length\n");
    exit(1);
  }

  if (input_buffer[input_file_length - 1] == '\n') {
    input_file_length = input_file_length - 1;
    input_buffer[input_file_length] = '\0';
    }

  if (key_buffer[key_length - 1] == '\n') {
    key_length = key_length - 1;
    key_buffer[key_length] = '\0';
  }

  for (int i = 0; i < input_file_length; i++) {
    if ((input_buffer[i] < 'A' || input_buffer[i] > 'Z')) {
      if (input_buffer[i] != ' ') {
        fprintf(stderr, "Error: input file not A-Z\n");
        exit(1);
      }
    }
  }
    for (int j = 0; j < key_length; j++) {
      if ((key_buffer[j] < 'A' || key_buffer[j] > 'Z')) {
        if (key_buffer[j] != ' ') {
          fprintf(stderr, "Error: key not A-Z\n");
          exit(1);
        }
      }
    }
  fclose(input_file);
  fclose(key_file);

  // Create a socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0);
  if (socketFD < 0){
    error("CLIENT: ERROR opening socket");
  }

   // Set up the server address struct
  setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");
  // Connect to server
  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    error("CLIENT: ERROR connecting");
  }

  char server_buffer_length[32];
  int length = snprintf(server_buffer_length, sizeof(server_buffer_length), "%d\n", input_file_length);
  int sent = 0;
  while (sent < length) {
    int send_data = send(socketFD, server_buffer_length + sent, length - sent, 0);
    sent = sent + send_data;
    }


  int i = 0;
  while (i < input_file_length) {
    int n = send(socketFD,input_buffer + i , input_file_length - i,0);
    if (n == -1) {
      error("Error: sending input file\n");
    }
    i += n;
  }

  int j = 0;
  while (j < key_length) {
    int n = send(socketFD, key_buffer + j, key_length - j, 0);
    if (n == -1) {
      error("Error: sending key\n");
    }
    j += n;
  }

  char *received_code = malloc(input_file_length + 1);
    int received = 0;
    while (received < input_file_length) {
      int n = recv(socketFD,received_code + received, input_file_length - received, 0);
      if (n <= 0) {
        error("Error: receiving data\n");
      }
      received += n;
    }
  received_code[input_file_length] = '\0';
  printf("%s\n", received_code);
  free(received_code);
  close(socketFD);
  return 0;
}
