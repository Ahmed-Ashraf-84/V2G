#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <sys/types.h>
#include <net/if.h>

#define DISCOVERY_PORT 15118       // Port to listen for discovery requests
#define RESPONSE_PORT 65500        // Port to send responses
#define MULTICAST_ADDR "ff02::1"   // IPv6 multicast address for local network

// Function to get the first link-local IPv6 address and its associated interface name
void get_valid_link_local_address(char *address_buffer, size_t buffer_size, char *interface_name) {
    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs failed");
        exit(EXIT_FAILURE);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET6) {
            struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)ifa->ifa_addr;

            // Include only link-local addresses
            if (IN6_IS_ADDR_LINKLOCAL(&sin6->sin6_addr)) {
                if (inet_ntop(AF_INET6, &sin6->sin6_addr, address_buffer, buffer_size) != NULL) {
                    strncpy(interface_name, ifa->ifa_name, IFNAMSIZ);
                    freeifaddrs(ifaddr);
                    return;
                }
            }
        }
    }

    // If no link-local address is found, exit with an error
    fprintf(stderr, "No link-local IPv6 address found\n");
    freeifaddrs(ifaddr);
    exit(EXIT_FAILURE);
}

int main() {
    int sockfd;
    char buffer[1024];
    struct sockaddr_in6 server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char link_local_ip[INET6_ADDRSTRLEN]; // Buffer for link-local IP address
    char interface_name[IFNAMSIZ];       // Buffer for interface name

    // 1. Get the link-local IPv6 address and interface name
    get_valid_link_local_address(link_local_ip, sizeof(link_local_ip), interface_name);
    printf("Using link-local IP: %s%%%s\n", link_local_ip, interface_name);

    // 2. Create an IPv6 UDP socket
    sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 3. Allow the socket to reuse the address
    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("Setting SO_REUSEADDR failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 4. Bind the socket to the link-local address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    inet_pton(AF_INET6, link_local_ip, &server_addr.sin6_addr);
    server_addr.sin6_port = htons(DISCOVERY_PORT);
    server_addr.sin6_scope_id = if_nametoindex(interface_name); // Add the scope ID

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("Server is listening for discovery requests on %s%%%s (port %d)\n", link_local_ip, interface_name, DISCOVERY_PORT);

    // 5. Join the multicast group
    struct ipv6_mreq multicast_req;
    inet_pton(AF_INET6, MULTICAST_ADDR, &multicast_req.ipv6mr_multiaddr);
    multicast_req.ipv6mr_interface = if_nametoindex(interface_name);

    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_JOIN_GROUP, &multicast_req, sizeof(multicast_req)) < 0) {
        perror("Joining multicast group failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 6. Main loop: Handle discovery requests
    while (1) {
        // Receive discovery request from a client
        int n = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (n < 0) {
            perror("Receive failed");
            continue;
        }
        buffer[n] = '\0'; // Null-terminate the received message
        printf("Received message: %s\n", buffer);

        // Prepare the response message
        char response[256];
        snprintf(response, sizeof(response), "SDP_RESPONSE: SE_IP=[%s%%%s]; SE_PORT=%d", link_local_ip, interface_name, RESPONSE_PORT);

        // Send the response back to the client
        if (sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)&client_addr, addr_len) < 0) {
            perror("Send response failed");
        } else {
            printf("Sent response: %s\n", response);
        }
    }

    // 7. Clean up
    close(sockfd);
    return 0;
}
