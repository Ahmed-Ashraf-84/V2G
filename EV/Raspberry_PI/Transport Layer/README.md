# **UDP Communication: Client-Server Example**

This README explains each function used in the UDP connection code between a client and a server. It includes detailed descriptions, function prototypes, all options available, and how the UDP connection works step-by-step.

---

## **Table of Contents**
1. [Overview](#overview)
2. [UDP Client Code](#udp-client-code)
3. [UDP Server Code](#udp-server-code)
4. [Functions Used](#functions-used)
    - [socket()](#socket)
    - [bind()](#bind)
    - [sendto()](#sendto)
    - [recvfrom()](#recvfrom)
    - [close()](#close)
5. [How UDP Communication Works](#how-udp-communication-works)
6. [Testing the UDP Connection](#testing-the-udp-connection)
7. [Conclusion](#conclusion)

---

## **Overview**
This project demonstrates the communication between a UDP server and a UDP client. The server listens for incoming messages, while the client sends a set of messages to the server. This README explains how the UDP connection is established and the functions used to set up the communication.

---

## **UDP Client Code**
The client initiates the connection to the server, sends messages, and waits for the server's response.

### **Steps in the Client Code:**
1. **socket()**: Create a UDP socket.
2. **sendto()**: Send messages to the server.
3. **recvfrom()**: Receive responses from the server.
4. **close()**: Close the socket.

---

## **UDP Server Code**
The server listens on a port for incoming messages, processes them, and responds back to the client.

### **Steps in the Server Code:**
1. **socket()**: Create a UDP socket.
2. **bind()**: Bind the socket to a specific port to listen for incoming messages.
3. **recvfrom()**: Receive messages from the client.
4. **sendto()**: Send a response back to the client.
5. **close()**: Close the socket.

---

## **Functions Used**

### **1. socket()**
**Prototype:**
```c
int socket(int domain, int type, int protocol);
```

**Description:** The `socket()` function is used to create a socket, which serves as the endpoint for communication. It establishes a communication channel between the application and the network.

**Parameters and Options:**
- `domain`:
  - `AF_INET`: IPv4 Internet protocol.
  - `AF_INET6`: IPv6 Internet protocol.
  - `AF_UNIX`: Local communication (inter-process).
- `type`:
  - `SOCK_DGRAM`: Datagram socket for UDP.
  - `SOCK_STREAM`: Stream socket for TCP.
- `protocol`:
  - `IPPROTO_UDP`: UDP protocol.
  - `IPPROTO_TCP`: TCP protocol.

**Example:**
```c
int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
```

---

### **2. bind()**
**Prototype:**
```c
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

**Description:** The `bind()` function is used to associate a socket with a specific IP address and port number on the local machine.

**Parameters:**
- `sockfd`: The socket file descriptor returned by `socket()`.
- `addr`: A pointer to a `struct sockaddr_in` structure that contains the IP address and port number.
- `addrlen`: The size of the `addr` structure.

**Example:**
```c
bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
```

---

### **3. sendto()**
**Prototype:**
```c
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
```

**Description:** The `sendto()` function sends a message to a specified address, which in this case is the server's address.

**Parameters:**
- `sockfd`: The socket file descriptor.
- `buf`: A pointer to the buffer containing the data to be sent.
- `len`: The length of the data to send.
- `flags`: Flags for the send operation (e.g., `0` for default behavior).
- `dest_addr`: A pointer to the destination address (server's address).
- `addrlen`: The size of the destination address structure.

**Options:**
- `flags`:
  - `MSG_DONTWAIT`: This flag makes the function non-blocking. It will return immediately, even if the send operation would block.
  - `MSG_CONFIRM`: This flag ensures that the kernel sends an ARP request to resolve the destination's MAC address if it is not already cached. It effectively checks the reachability of the destination in terms of network-level address resolution, but `it does not guarantee` the delivery of the actual data.
    - `Explanation` :

        -`Without MSG_CONFIRM`: The kernel may not send a fresh ARP request if the destination’s MAC address is already in the cache.
        -`With MSG_CONFIRM`: It forces the kernel to confirm or revalidate the destination address by sending an ARP request if necessary. This is typically useful in scenarios like:

        Detecting changes in the network (e.g., a new MAC address for the same IP).
        Ensuring the destination is still reachable before attempting to send critical data.
  - `MSG_NOSIGNAL`: This flag prevents the system from sending a `SIGPIPE` signal if the destination socket is closed.
    - `Explanation` :

        When a program tries to send data on a socket that has been closed by the receiver (like when the client disconnects or crashes), the operating system sends a special signal called `SIGPIPE` to the sending program.
        
        By default, this signal causes the program to terminate abruptly, which may not be desirable in most cases.

**Example:**
```c
sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
```

---

### **4. recvfrom()**
**Prototype:**
```c
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
```

**Description:** The `recvfrom()` function is used to receive messages from the network. It retrieves the data sent by the client or server.

**Parameters:**
- `sockfd`: The socket file descriptor.
- `buf`: A pointer to the buffer where the received data will be stored.
- `len`: The size of the buffer.
- `flags`: Flags for the receive operation (e.g., `0` for default behavior).
- `src_addr`: A pointer to the source address (client's address).
- `addrlen`: A pointer to the size of the address structure.

**Options:**
- `flags`:
  - `MSG_DONTWAIT`: This flag makes the function non-blocking. It returns immediately, even if no data is available, instead of waiting for data.
  - `MSG_PEEK`: This flag allows you to look at the data in the socket buffer without removing it. The next call to `recvfrom` will still retrieve the same data.
  - `MSG_WAITALL`: This flag forces the function to block until the entire requested amount of data `len` is received.

**Example:**
```c
recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &client_len);
```

---

### **5. close()**
**Prototype:**
```c
int close(int fd);
```

**Description:** The `close()` function is used to close a socket, releasing the resources used by the socket.

**Parameters:**
- `fd`: The file descriptor of the socket to be closed.

**Example:**
```c
close(sockfd);
```

---

## **How UDP Communication Works**
### **UDP Communication:**
- **Unreliable**: UDP does not guarantee the delivery of messages.
- **Connectionless**: UDP does not establish a connection before sending data.
- **No Acknowledgment**: UDP does not wait for an acknowledgment from the receiver.

### **Steps in UDP Communication:**
1. The server creates a socket and binds it to a port to listen for incoming messages.
2. The client creates a socket and sends a message to the server’s IP address and port using `sendto()`.
3. The server receives the message using `recvfrom()`, processes it, and responds to the client using `sendto()`.
4. The client receives the response using `recvfrom()`.
5. Both client and server close the sockets using `close()` when the communication is finished.

---

## **Testing the UDP Connection**

To test the UDP connection between the client and server:

1. Compile the client and server programs.
2. Run the server on one terminal:
   ```sh
   ./server
   ```
3. Run the client on another terminal:
   ```sh
   ./client
   ```
4. Monitor the messages exchanged between the client and server using `Wireshark`.

---

## **Conclusion**
In this README, we covered the key functions used in the UDP client-server communication, their descriptions, and their parameters. We also outlined how UDP works, and how to test the connection. By using these functions (`socket()`, `bind()`, `sendto()`, `recvfrom()`, and `close()`), you can set up and manage UDP connections for communication between devices in a network.
