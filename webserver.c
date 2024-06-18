#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
int main()
{
	// creating a socket
	int sockfd = socket(AF_INET6, SOCK_STREAM, 0); // sockfd stands for socket file descriptor
	if(sockfd == -1){
		perror("webserver(socket)");
		return 1;
	}
	printf("socket created successfully\n");
	return 0;
}
