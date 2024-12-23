#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <sys/types.h>

#define DISCOVERY_PORT 15118       // Port to listen for discovery requests
#define RESPONSE_PORT 65500         // Service port to respond with
#define MULTICAST_ADDR "ff02::1"   // IPv6 multicast address

// Function to get the first non-link-local IPv6 address
void get_valid_ipv6_address(char *address_buffer, size_t buffer_size) {
    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs failed");
        exit(EXIT_FAILURE);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET6) {
            struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)ifa->ifa_addr;

            // Skip link-local addresses (starting with fe80::)
            if (IN6_IS_ADDR_LINKLOCAL(&sin6->sin6_addr)) {
                continue;
            }

            // Convert address to string and store it
            if (inet_ntop(AF_INET6, &sin6->sin6_addr, address_buffer, buffer_size) != NULL) {
                freeifaddrs(ifaddr);
                return;
            }
        }
    }

    // If no valid IPv6 address is found, fall back to "::"
    strncpy(address_buffer, "::", buffer_size);
    freeifaddrs(ifaddr);
}

int main() {
    int sockfd;
    char buffer[1024];
    struct sockaddr_in6 server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char server_ip[INET6_ADDRSTRLEN]; // Buffer for server IP address

    // Create an IPv6 UDP socket
    sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Allow the socket to reuse the address
    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("Setting SO_REUSEADDR failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Get the first valid non-link-local IPv6 address
    get_valid_ipv6_address(server_ip, sizeof(server_ip));

    // Bind the socket to the first valid non-loopback IPv6 interface
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;

    // We bind to the first valid IPv6 address here (not the loopback address)
    if (inet_pton(AF_INET6, server_ip, &server_addr.sin6_addr) <= 0) {
        perror("Invalid address for binding");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    server_addr.sin6_port = htons(DISCOVERY_PORT);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("Server is listening for discovery requests on port %d...\n", DISCOVERY_PORT);
    printf("Server IP: %s\n", server_ip);

    // Join the multicast group
    struct ipv6_mreq multicast_req;
    inet_pton(AF_INET6, MULTICAST_ADDR, &multicast_req.ipv6mr_multiaddr);
    multicast_req.ipv6mr_interface = 0; // Default interface

    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_JOIN_GROUP, &multicast_req, sizeof(multicast_req)) < 0) {
        perror("Joining multicast group failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Receive discovery request
        int n = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (n < 0) {
            perror("Receive failed");
            continue;
        }
        buffer[n] = '\0'; // Null-terminate the received message
        printf("Received message: %s\n", buffer);

        // Prepare the response message
        char response[256];
        snprintf(response, sizeof(response), "SDP_RESPONSE: SE_IP=[%s]; SE_PORT=%d", server_ip, RESPONSE_PORT);

        // Send the response to the client
        if (sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)&client_addr, addr_len) < 0) {
            perror("Send response failed");
        } else {
            printf("Sent response: %s\n", response);
        }
    }

    close(sockfd);
    return 0;
}
