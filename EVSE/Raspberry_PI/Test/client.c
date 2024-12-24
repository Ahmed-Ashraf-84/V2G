#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <net/if.h>

// Function to get our own link-local IPv6 address
char* get_own_linklocal_addr() {
    struct ifaddrs *ifaddr, *ifa;
    static char addr_str[INET6_ADDRSTRLEN];
    
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs failed");
        return NULL;
    }
    
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET6) {
            struct sockaddr_in6 *addr = (struct sockaddr_in6 *)ifa->ifa_addr;
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
    // Create socket for sending discovery messages and receiving responses
    int sock = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Bind to sender port (65500)
    struct sockaddr_in6 sender_addr;
    memset(&sender_addr, 0, sizeof(sender_addr));
    sender_addr.sin6_family = AF_INET6;
    sender_addr.sin6_port = htons(65500);
    sender_addr.sin6_addr = in6addr_any;

    if (bind(sock, (struct sockaddr*)&sender_addr, sizeof(sender_addr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    //index to get the number of tap0 : qemu vertual interface : the number changed each time you run qemu
    const char *interface_name = "tap0";
    unsigned int index;

    // Get the index of the interface
    index = if_nametoindex(interface_name);
    if (index == 0) {
        perror("if_nametoindex failed");
        
    }

    // Enable multicast on the socket
    unsigned int outgoing_interface = index;
    if (setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_IF, 
                   &outgoing_interface, sizeof(outgoing_interface)) < 0) {
        perror("setsockopt IPV6_MULTICAST_IF failed");
        return 1;
    }

    // Get our own link-local address for the discovery message
    char* our_addr = get_own_linklocal_addr();
    if (our_addr == NULL) {
        fprintf(stderr, "Failed to get own address\n");
        return 1;
    }

    printf("IP:%s \n",our_addr);

    // Prepare multicast destination address
    struct sockaddr_in6 multicast_addr;
    memset(&multicast_addr, 0, sizeof(multicast_addr));
    multicast_addr.sin6_family = AF_INET6;
    multicast_addr.sin6_port = htons(15118);  // Receiver's port
    inet_pton(AF_INET6, "ff02::1", &multicast_addr.sin6_addr);

    // Prepare discovery message
    char discovery_msg[256];
    snprintf(discovery_msg, sizeof(discovery_msg), "discovery message from IP: %s", our_addr);

    printf("Sending discovery message...\n");

    // Send discovery message

         
    if (sendto(sock, discovery_msg, strlen(discovery_msg), 0,
           (struct sockaddr*)&multicast_addr, sizeof(multicast_addr)) < 0) {
        perror("Failed to send discovery message");
        return 1;
    }   



    
    

    printf("Waiting for response...\n");

    // Wait for response
    char buffer[1024];
    struct sockaddr_in6 resp_addr;
    socklen_t resp_addr_len = sizeof(resp_addr);

    // Set receive timeout to 5 seconds
    struct timeval tv;
    tv.tv_sec = 15;
    tv.tv_usec = 0;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("setsockopt SO_RCVTIMEO failed");
        return 1;
    }

    ssize_t received = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                               (struct sockaddr*)&resp_addr, &resp_addr_len);
    if (received < 0) {
        perror("No response received");
        return 1;
    }

    buffer[received] = '\0';

    // Get responder's address as string
    char resp_str[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &resp_addr.sin6_addr, resp_str, INET6_ADDRSTRLEN);

    printf("Received response from %s: %s\n", resp_str, buffer);
    printf("Receiver found at IP: %s, Port: 15118\n", resp_str);

    close(sock);
    return 0;
}
