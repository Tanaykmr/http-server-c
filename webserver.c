#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main()
{
    // creating a socket for IPv6
    int sockfd = socket(AF_INET6, SOCK_STREAM, 0); // sockfd stands for socket file descriptor
    if (sockfd == -1)
    {
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

    // Create client address
    struct sockaddr_in client_addr;
    int client_addrlen = sizeof(client_addr);

    // binding the socket to an address
    if (bind(sockfd, (struct sockaddr *)&host_addr, host_addrlen) != 0)
    {
        perror("webserver(bind)");
        close(sockfd);
        return 1;
    }
    printf("socket successfully bound to address\n");

    // listening for incoming connections
    if (listen(sockfd, SOMAXCONN) != 0)
    {
        perror("webserver(listen)");
        close(sockfd);
        return 1;
    }
    printf("server listening for connections\n");

    while (1)
    {
        // Accept incoming connections
        // struct sockaddr_in6 client_addr; //maybe this is named host_address
        // socklen_t client_addrlen = sizeof(client_addr);
        int new_socket = accept(sockfd, (struct sockaddr *)&host_addr, &host_addrlen);
        if (new_socket < 0)
        {
            perror("webserver(accept)");
            continue;
        }
        printf("New connection accepted\n");

        // Read from the socket
        char buffer[BUFFER_SIZE];
        int valread = read(new_socket, buffer, BUFFER_SIZE);
        if (valread < 0)
        {
            perror("webserver (read)");
            close(new_socket);
            continue;
        }

        // Read the request
        char method[BUFFER_SIZE], uri[BUFFER_SIZE], version[BUFFER_SIZE];
        sscanf(buffer, "%s %s %s", method, uri, version);
        printf("[%s:%u] %s %s %s\n", inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port), method, version, uri);

        // Write to the socket
        // const char *resp = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, world!";
        // use either the above or below, the only difference being that the latter response is more detailed
        char resp[] = "HTTP/1.1 200 OK\r\n"
                      "Server: webserver-c\r\n"
                      "Content-type: text/html\r\n\r\n"
                      "<html>hello, world</html>\r\n";

        int valwrite = write(new_socket, resp, strlen(resp));
        if (valwrite < 0)
        {
            perror("webserver (write)");
            close(new_socket); // maybe we will have to keep this open, let's see
            continue;
        }

        // Get client address
        int sockn = getsockname(new_socket, (struct sockaddr *)&client_addr,
                                (socklen_t *)&client_addrlen);
        printf("sockn: ", sockn, "\n");
        if (sockn < 0)
        {
            perror("webserver (getsockname)");
            continue;
        }

        // Closing the new socket after handling the connection (not shown here)
        close(new_socket);
    }

    return 0;
}
