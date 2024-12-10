#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {

    char *ip = "127.0.0.1";
    int port = 5700;

    int client_1_sock;
    struct sockaddr_in client_1_addr;
    socklen_t addr_size;
    char buffer[1024];
    int sock_assign;

    client_1_sock = socket(AF_INET, SOCK_STREAM,0);
    if (client_1_sock < 0)
    {
      perror("* Socket error *");
      exit(1);
    }

    printf("* TCP server socket created. * \n");

    memset(&client_1_addr, '\0', sizeof(client_1_addr));
    client_1_addr.sin_family = AF_INET;
    client_1_addr.sin_port = port;
    client_1_addr.sin_addr.s_addr = inet_addr(ip);

    connect(client_1_sock, (struct sockaddr*)&client_1_addr, sizeof(client_1_addr));
    printf("Connected to the server\n");

    bzero(buffer, 1024);
    strcpy(buffer, "Hello 0");
    printf("Client : %s\n", buffer);
    send (client_1_sock, buffer, strlen(buffer), 0);

    bzero(buffer, 1024);
    recv(client_1_sock, buffer, sizeof(buffer), 0);
    printf("Server: %s\n", buffer);

    bzero(buffer, 1024);
    strcpy(buffer, "Hello 1");
    printf("Client : %s\n", buffer);
    send (client_1_sock, buffer, strlen(buffer), 0);

    bzero(buffer, 1024);
    recv(client_1_sock, buffer, sizeof(buffer), 0);
    printf("Server: %s\n", buffer);

    bzero(buffer, 1024);
    strcpy(buffer, "Hello 2");
    printf("Client : %s\n", buffer);
    send (client_1_sock, buffer, strlen(buffer), 0);

    bzero(buffer, 1024);
    recv(client_1_sock, buffer, sizeof(buffer), 0);
    printf("Server: %s\n", buffer);

    close(client_1_sock);
    printf("Dissconnected from server\n");

    return 0;

}
