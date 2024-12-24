#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>

void print_available_interfaces() {
    struct ifaddrs *ifaddr, *ifa;
    char addr_str[INET6_ADDRSTRLEN];
    
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs failed");
        return;
    }
    
    printf("Available interfaces with link-local addresses:\n");
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET6) {
            struct sockaddr_in6 *addr = (struct sockaddr_in6 *)ifa->ifa_addr;
            if (IN6_IS_ADDR_LINKLOCAL(&addr->sin6_addr)) {
                inet_ntop(AF_INET6, &addr->sin6_addr, addr_str, INET6_ADDRSTRLEN);
                printf("Interface: %s, Address: %s\n", ifa->ifa_name, addr_str);
            }
        }
    }
    
    freeifaddrs(ifaddr);
}


int main()
{
    print_available_interfaces();
}