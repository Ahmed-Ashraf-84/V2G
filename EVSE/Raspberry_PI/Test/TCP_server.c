#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {

    char *ip = "127.0.0.1";
    int port = 5700;

    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    char buffer[1024];
    int sock_assign;

    server_sock = socket(AF_INET, SOCK_STREAM,0);
    if (server_sock < 0)
    {
      perror("* Socket error *");
      exit(1);
    }

    printf("* TCP server socket created. * \n");

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    sock_assign = bind (server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (sock_assign < 0)
    {
      perror ("* Bind error *");
      exit(1);
    }

    printf("Bind to port number : %d\n",port);

    listen(server_sock,2);
    printf("Listen.......\n");

    while (1) {
      addr_size = sizeof(client_addr);
      client_sock = accept(server_sock, (struct sockaddr*)&client_sock, &addr_size);
      printf("* Client connected *\n");

      bzero(buffer, 1024);
      recv(client_sock, buffer, sizeof(buffer), 0);
      printf("Client: %s\n", buffer);

      bzero(buffer, 1024);
      strcpy(buffer, "Hello 0");
      printf("Server: %s\n", buffer);
      send (client_sock, buffer, strlen(buffer), 0);

      bzero(buffer, 1024);
      recv(client_sock, buffer, sizeof(buffer), 0);
      printf("Client: %s\n", buffer);

      bzero(buffer, 1024);
      strcpy(buffer, "Hello 1");
      printf("Server: %s\n", buffer);
      send (client_sock, buffer, strlen(buffer), 0);


      bzero(buffer, 1024);
      recv(client_sock, buffer, sizeof(buffer), 0);
      printf("Client: %s\n", buffer);

      bzero(buffer, 1024);
      strcpy(buffer, "Hello 2");
      printf("Server: %s\n", buffer);
      send (client_sock, buffer, strlen(buffer), 0);


      close(client_sock);
      printf("* Client disconnected *\n");
      return 0;

    }

}
