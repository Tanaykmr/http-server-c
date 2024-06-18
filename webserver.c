#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>

#define PORT 8080

int main()
{
	// creating a socket
	int sockfd = socket(AF_INET6, SOCK_STREAM, 0); // sockfd stands for socket file descriptor
	if (sockfd == -1)
	{
		perror("webserver(socket)");
		return 1;
	}
	printf("socket created successfully\n");
	return 0;

	// we create the address to bind the socket to
	struct sockaddr_in host_addr;
	int host_addrlen = sizeof(host_addr);

	host_addr.sin_family = AF_INET;
	host_addr.sin_port = htons(PORT);
	host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// we bind the socket to an address
	if (bind(sockfd, (struct sockaddr *)&host_addr, host_addrlen) != 0)
	{
		perror("webserver (bind)");
		return 1;
	}
	printf("socket successfully bound to address\n");

	return 0;
}
