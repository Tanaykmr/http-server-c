#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080

int main() {
    // creating a socket for IPv6
    int sockfd = socket(AF_INET6, SOCK_STREAM, 0); // sockfd stands for socket file descriptor
    if (sockfd == -1) {
        perror("webserver(socket)");
        return 1;
    }
    printf("socket created successfully\n");

    // creating the address to bind the socket to
    struct sockaddr_in6 host_addr;
    int host_addrlen = sizeof(host_addr);

    host_addr.sin6_family = AF_INET6;
    host_addr.sin6_port = htons(PORT);
    host_addr.sin6_addr = in6addr_any; // Bind to any address

    // binding the socket to an address
    if (bind(sockfd, (struct sockaddr *)&host_addr, host_addrlen) != 0) {
        perror("webserver(bind)");
        close(sockfd);
        return 1;
    }
    printf("socket successfully bound to address\n");

    // listening for incoming connections
    if (listen(sockfd, SOMAXCONN) != 0) {
        perror("webserver(listen)");
        close(sockfd);
        return 1;
    }
    printf("server listening for connections\n");

    while (1) {
        struct sockaddr_in6 client_addr;
        socklen_t client_addrlen = sizeof(client_addr);
        int new_socket = accept(sockfd, (struct sockaddr *)&client_addr, &client_addrlen);
        if (new_socket < 0) {
            perror("webserver(accept)");
            continue;
        }

        printf("New connection accepted\n");

        // Closing the new socket after handling the connection (not shown here)
        close(new_socket);
    }

    close(sockfd);
    return 0;
}
