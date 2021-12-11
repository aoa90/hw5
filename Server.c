#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include "client.h"

//im thinking at the first instance of a client we set the grid and everything 

//client struct to hold all the clients, maybe their File descriptors so that u can send message to all
 int clientfds[5]; //maybe this can hold client fds;
 
void* processClient(void *client_sock){

  int curr_sock = *(int *) client_sock; 
  free(client_sock); //they said to free it here since we dont need it 

// this is where server listens 
// the server needs to recieve a string like client with fd ____ moved player position up 
  char buffer[1024];
  bzero(buffer, 1024); 
  recv(curr_sock, buffer, sizeof(buffer), 0);
  printf("Client: %s\n", buffer);

//this is where the server sends to client (thats what goes in the buffer)
// since we recieved a client moved up, we send client ____ position->y++
  bzero(buffer, 1024);
  strcpy(buffer, "HI, THIS IS SERVER. HAVE A NICE DAY!!!");
  printf("Server: %s\n", buffer);
  for(int i = 0; i < 5; i++){ // send a message to all;
    if(clientfds[i] != 0){
      send(clientfds[i], buffer, strlen(buffer), 0);
    }
  }
  close(curr_sock);
  printf("[+]Client disconnected.\n\n");

}

int main(){

  char *ip = "127.0.0.1";
  int port = 40261;

 
  int server_sock, client_sock;
  struct sockaddr_in server_addr, client_addr;
  socklen_t addr_size;
  char buffer[1024];
  int n;

  server_sock = socket(AF_INET, SOCK_STREAM, 0);

  if (server_sock < 0){
    perror("[-]Socket error");
    exit(1);
  }

  printf("[+]TCP server socket created.\n");

  memset(&server_addr, '\0', sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = port;
  server_addr.sin_addr.s_addr = inet_addr(ip);

  n = bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));

  if (n < 0){
    perror("[-]Bind error");
    exit(1);
  }

  printf("[+]Bind to the port number: %d\n", port);

  
  listen(server_sock, 5);
  printf("Listening...\n");
  
  while(1){

    addr_size = sizeof(client_addr);
    client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size); // this returns diff file destriptors for each client 
    printf("[+]Client connected.\n");

    pthread_t t;
    int *pclient = malloc(sizeof(int));
    pclient = client_sock;

    pthread_create(&t, NULL, processClient, pclient); // create new thread

  }

  return 0;
}