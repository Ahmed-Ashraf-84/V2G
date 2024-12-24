#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>

// Function to get our own link-local IPv6 address
char* get_own_linklocal_addr() {
    struct ifaddrs *ifaddr, *ifa;
    static char addr_str[INET6_ADDRSTRLEN];
    
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs failed");
        return NULL;
    }
    
    // Look through all interfaces for a link-local address
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET6) {
            struct sockaddr_in6 *addr = (struct sockaddr_in6 *)ifa->ifa_addr;
            // Check if this is a link-local address
            if (IN6_IS_ADDR_LINKLOCAL(&addr->sin6_addr)) {
                inet_ntop(AF_INET6, &addr->sin6_addr, addr_str, INET6_ADDRSTRLEN);
                break;
            }
        }
    }
    
    freeifaddrs(ifaddr);
    return addr_str;
}

int main() {
    // Create a UDP socket for receiving discovery messages
    int discovery_sock = socket(AF_INET6, SOCK_DGRAM, 0);
    if (discovery_sock < 0) {
        perror("Discovery socket creation failed");
        return 1;
    }

    // Allow multiple sockets to use the same port
    int reuse = 1;
    if (setsockopt(discovery_sock, SOL_SOCKET, SO_REUSEADDR, 
                   &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt SO_REUSEADDR failed");
        return 1;
    }

    // Bind to the discovery port (15118)
    struct sockaddr_in6 recv_addr;
    memset(&recv_addr, 0, sizeof(recv_addr));
    recv_addr.sin6_family = AF_INET6;
    recv_addr.sin6_port = htons(15118);
    recv_addr.sin6_addr = in6addr_any;

    if (bind(discovery_sock, (struct sockaddr*)&recv_addr, sizeof(recv_addr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    // Join the multicast group
    struct ipv6_mreq mreq;
    inet_pton(AF_INET6, "ff02::1", &mreq.ipv6mr_multiaddr);
    mreq.ipv6mr_interface = 0;  // Use default interface

    if (setsockopt(discovery_sock, IPPROTO_IPV6, IPV6_JOIN_GROUP, 
                   &mreq, sizeof(mreq)) < 0) {
        perror("setsockopt IPV6_JOIN_GROUP failed");
        return 1;
    }

    printf("Receiver listening for discovery messages on port 15118...\n");

    while (1) {
        char buffer[1024];
        struct sockaddr_in6 sender_addr;
        socklen_t sender_addr_len = sizeof(sender_addr);

        // Receive discovery message
        ssize_t received = recvfrom(discovery_sock, buffer, sizeof(buffer) - 1, 0,
                                  (struct sockaddr*)&sender_addr, &sender_addr_len);
        if (received < 0) {
            perror("recvfrom failed");
            continue;
        }

        buffer[received] = '\0';
        
        // Get sender's address as string
        char sender_str[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &sender_addr.sin6_addr, sender_str, INET6_ADDRSTRLEN);
        
        printf("Received discovery message from %s: %s\n", sender_str, buffer);

        // Get our own link-local address
        char* our_addr = get_own_linklocal_addr();
        if (our_addr == NULL) {
            fprintf(stderr, "Failed to get own address\n");
            continue;
        }

        // Prepare response message with our address and port
        char response[256];
        snprintf(response, sizeof(response), "Receiver at %s:15118", our_addr);

        // Send response to sender's port 65500
        sender_addr.sin6_port = htons(65500);
        
        if (sendto(discovery_sock, response, strlen(response), 0,
                  (struct sockaddr*)&sender_addr, sender_addr_len) < 0) {
            perror("Failed to send response");
            continue;
        }

        printf("Sent response to sender at port 65500\n");
    }

    close(discovery_sock);
    return 0;
}