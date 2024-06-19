# HTTP Server in C
Resource: https://bruinsslot.jp/post/simple-http-webserver-in-c/

My Notes: https://drive.google.com/file/d/1XxhE8DmnBPfnTTaZHJDpgnjdg8sJAMxK/view

### Webserver
A webserver, and in this case on the software side is able to satisfy client requests over HTTP, and other several related protocols. Its primary function is to store, process, deliver files to that client. At minimum this is an HTTP Server, which is a piece of software that understand URLs (Universal Resource Locator) and HTTP (Hypertext Transfer Protocol).

### HTTP
As mentioned above a webserver in our case should be able to understand the HTTP protocol. What does a protocol mean in this context? 

**Protocol:** Well, it is a set of rules for communication between two computers.

In this case it specifies how to transfer hyper text documents, meaning documents that are interconnected by hyperlinks.

The protocol is **textual** and **stateless**.

*Textual* because that all the commands are plain text, and you’ll be able to read and inspect it. *Stateless*, because that neither the client nor server remembers previous communications.

It means on the client side the application (for instance a web browser) needs to speak the same ‘language’ as the webserver in order to communicate. The ‘language’ that is used is HTTP.

>A message is constructed that is either a request or a response from either the client or the server. This message needs to be transported, and that is where TCP comes in.

### TCP
HTTP presumes an underlying transport layer protocol to establish host-to-host data transfer channels, and manage the data exchange in a client-to-server or peer-to-peer networking model. The protocol that is commonly used for HTTP server is TCP (Transmission Control Protocol), but it can also be adapted to be used with for instance UDP (User Datagram Protocol). However, because of RFC 2616 that states that the transport layer should be reliable, we will be using TCP instead of UDP.

>Why UDP is unreliable: https://chatgpt.com/share/5005a48f-ad2f-4504-856f-55a71ef6c6de

TCP maintains communications between application processes between hosts (client and server), and they use port numbers to track sessions. HTTP, and TCP protocols are part of a suite of several multiple protocols on each layer of the request/response cycle. This suite is also known as: TCP/IP.

### Internet Protocol Suite a.k.a. TCP/IP
The Internet Protocol suite is a conceptual model, and it consists out of set of protocols used in internet, and similar computer networks. It specifies how data should be packetized, addressed, transmitted, routed and received. The model is made up of 4 abstraction layers: the application, transport, internet and link layer.


We can see that HTTP is layered over TCP and uses it to transport it's message data. In turn TCP is layered over IP to make sure the data ends up in the right location.

The application layer talks to the transport layer through a port, and each port is assigned to a different protocol in the application layer. In case of HTTP, it is port 80. This helps the TCP know which protocol the data is coming from.

The steps to build a web server are:

```shell
SOCKET ----> BIND ----> LISTEN ----> ACCEPT ----> READ/WRITE
```

#### **Socket Types**:
 **SOCK_STREAM**: Used for TCP. It provides a reliable, connection-oriented, and bidirectional (full-duplex) byte stream. This means data is sent in a continuous flow of bytes, ensuring that it is not lost or duplicated.
 
 **SOCK_DGRAM**: Used for UDP. It provides an unreliable, connectionless datagram service. This means data is sent in packets, and there is no guarantee of delivery, order, or duplication.
### Sockets
a socket creates an endpoint for comms. 
we need to include the `<sys/socket.h>` header file, and that we can create a socket endpoint to communicate by using the function: `socket`.
It returns something called as the file descriptor and can take 3 inputs:
1. int domain
2. int type
3. int protocol
#### Domain
domain is an integer that specifies a comm domain, and selects the protocol family that will be used for comms. These families are defined in the `sys/socket.h`.
Since we're creating a webserver that uses TCP, we will be using AF_INET, which uses IPv4 internet protocols. 
LEARN MORE ABOUT AF_INET: https://stackoverflow.com/questions/1593946/what-is-af-inet-and-why-do-i-need-it
#### Type
The argument `type` specifies the ‘communication semantics’. So, which socket type do we need to use here?
Well, we said we wanted to create a TCP webserver, so which of the options resembles that? Let’s refer back to man pages of `tcp`, and `ip`. There we can see that the valid socket type for a TCP socket is `SOCK_STREAM`. `SOCK_STREAM` is a full-duplex byte stream, and it is characterized as a type that ensures that data is not lost or duplicated.

> what is a full duplex byte stream?!
#### Protocol
`protocol`, according to the man page, is the particular protocol to be used with the socket.
It is common that there exists only one protocol that will support a specific socket type. In our case where we are choosing `SOCK_STREAM` as the `type`, and as stated by the `man 7 ip` man page, `protocol` is the IP protocol in the IP header to be received or sent. And in this case of creating a TCP socket were the valid value is `0` for TCP sockets.
#### Summarising
* Domain = AF_INET6 ----> since we are using IPv6
* Type = SOCK_STREAM ----> man pages dictate that the socket type = SOCK_STREAM for TCP
* Protocol = 0 ----> man pages dictate that the IP protocol in the IP header should be 0 for TCP protocols
#### Return value
The `int socket(int domain, int type, int protocol)` function returns an integer which is an file descriptor for the socket.
The file descriptor is an unique number that identifies an open file, in this case this is our socket and just as a regular file we will be able to read and write to it.
When an error occurred it will return the value `-1`, it will also set an `errno` which we can use to properly handle errors.
#### errno
errno is an integer variable that can be set to signify what exactly has gone wrong.
And in order to inspect what kind of error was raised, we can use `perror` to print the error, it will translate the error code that has been set in the variable `errno` to a human-readable form.
```c
if(sockfd == -1){

perror("webserver(socket)");

return 1;

}
```

We can use `perror` and set the argument `s` with a string, and it then will be appended with an error message that corresponds with the current value of `errno`.



We have now created the socket, now we need to bind it to an address.

### Binding the socket to an address
The socket is created and exists in a namespace (an address family, the `AF` in `AF_INET` stands for address family), and we need to bind the socket to a local address, in order for the socket to receive connections. We need to be using the `bind` function for this.

**Bind**:
When a socket is created with socket, it exists in a name space (address family) but has no address assigned to it. `bind()` assigns the address specified by `addr` to the socket referred to by the file descriptor `sockfd`. `addrlen` specifies the size, in bytes, of the address structure pointed to by `addr`.
Traditionally, this operation is called “assigning a name to a socket”. It is normally necessary to assign a local address using `bind()` before a SOCK_STREAM socket may receive connections.
`bind()` is included in the header file, `<sys/socket.h>`. And, on success it will return zero. It accepts as arguments: `sockfd`, `*addr`, and `addrln`. Let’s go over the arguments, and make sense of what we need to do in order to implement it:
* sockfd: this is the file descriptor we created in the last section
* addr: Defines the address structure to which we want to bind the socket to, and that depends on the address family we are using.
Our address family is `AF_INET6`. So, according to the man pages, the address structure looks like:
```c
struct sockaddr_in {
	sa_family_t sin_family; /* address family: AF_INET */
	in_port_t sin_port; /* port in network byte order */
	struct in_addr sin_addr; /* internet address */ };
	
	/* Internet address. */
	struct in_addr {
		uint32_t s_addr; /* address in network byte order */
	};
```

`sin_family` is always set to`AF_IFNET6`, `sin_port `contains the port in network byte order.

Network byte order represents how bytes are arranged when sending data over a network. This order must be chosen to make sure that on both ends, the machines interpret the numbers the same way independent of the cpu architecture.

Example: an integer value of 1 represented as 4 bytes would be represented on ‘big endian’ machines as `0 0 0 1`, on a ‘little endian’ machines this would be `1 0 0 0`. The value of `0 0 0 1` of the ‘big endian’ machine would then be interpreted by the ‘little endian’ machine as the value `16777216`, and vice versa.
- **Big-endian**: The most significant byte (MSB) is stored first (at the lowest address).
- **Little-endian**: The least significant byte (LSB) is stored first (at the lowest address).

Hence, the man page dictate that we need to call `htons` on the number that is assigned to the port. Like so: `htons(8080)`. It converts the host byte order to network byte order.
`sin_addr` contains the host interface address in network byte order, and it is a member of the `struct` named `in_addr`.
The man page states that it should be one of the `INADDR_*` values.
These are defined as symbolic constants in the header file `<netinet/in.h>`, or can set it by using one of the `inet_aton`, `inet_addr`, or `inet_makeaddr` library functions, to specify a specific address.
We will make use of the symbolic constant `INADDR_ANY`, and it means ‘any address’, which translates to the `0.0.0.0`. `INADDRY_ANY` is already in network byte order, so we don’t really have to convert it.
The man page advices us to convert, so lets just implement it. We do this by calling `htonl` on the address.

But why are we using `0.0.0.0`, here?
This is just your machine’s IP address. Your machine will have one IP address for each network interface. When your machine has for example Wi-Fi, and an ethernet connection, then that machine will have two addresses, one for each interface. When we don’t care what interface is going to be used we use the special address for this, `0.0.0.0` which is defined in the symbolic constant `INADDR_ANY` translates to this address.

* addrlen: The `addrlen` argument specifies the size of the address structure `addr` in bytes. To get this we can use the `sizeof()` operator (it looks like a function, but it is an operator like `&&`, `||`, etc.). The argument is of type `socklen_t` which is an integer type.'
#### return value
On success the return value for `bind` will be zero, when an error occurred it will return `-1`, and `errno` will also be set.

```c
// we bind the socket to an address

if (bind(sockfd, (struct sockaddr *)&host_addr, host_addrlen) != 0)

{

perror("webserver (bind)");

return 1;

}

printf("socket successfully bound to address\n");
```
Note that we are typecasting `addr` to the `struct` pointer `struct sockaddr *` in the argument of the `bind` function. Since `addr` is of the type `struct sockaddr_in` we need to cast it to `struct sockaddr *`. From the man page `bind` we can read: “The only purpose of this structure (`sockaddr`) is to cast the structure pointer passed in `addr` in order to avoid compiler warnings.” In essence what we are doing here is: whatever `addr` is pointing to, act like a `sockaddr`.


We have now bound our socket to a specific address.


### Listen
We make the socket listen to incoming connections using the `listen` function.
The `listen` function will put the socket into ‘passive’ mode. Stream sockets are often ‘active’ or ‘passive’.
Stream sockets provide a reliable, two-way, connection-based byte stream. The most common type of stream socket is a TCP socket.
1. **Active Socket**:
    - **Client Side**: The client performs an active socket open. This involves creating a socket and initiating a connection to a server.
    - **Steps**: The client creates a socket, specifies the server's address and port, and calls the `connect()` function to establish a connection.
2. **Passive Socket**:
    - **Server Side**: The server performs a passive socket open. This involves creating a socket and preparing it to accept incoming connections from clients.
    - **Steps**: The server creates a socket, binds it to a local address and port, calls `listen()` to mark it as a passive socket that will listen for incoming connection requests, and then waits for connections using the `accept()`function.
#### sockfd
Again, this is the file descriptor of the socket, and thus we will use the `sockfd` that we’ve created in section 3.
#### backlog
This integer will define how many pending connections will be queued up for `sockfd` socket, before it will be refused. For now, we will set this to `128`.
Further connection requests block until a pending connection is accepted. So, it defines the number of connections that are accepted, but not yet handled by the application, until `accept` gets it off the queue.

The symbolic constant `SOMAXCONN` in `<sys/socket.h>` is defined by our system (`128` in the case of Linux), and we can use it to set the `backlog` argument (`man sys_socket.h`).
#### return value
On success, zero will be returned, on failure `-1` will be returned, and as before `errno` will also set, so we can check and handle it accordingly.

Since we can now listen to incoming connections, we must accept them now.


### Accept
We are now ready to accept the incoming connections, and we shall use the `accept` function to do it.

The `accept` function will get the first connection from the queue of the listening socket `sockfd`. Then it will create a _new_ connected socket, and the file descriptor that points to that socket will be returned. The newly created socket is however _not in a listening state_, and thus the original socket is not affected by this call and can be used to accept other connections. When there are no pending connections when the `accept` function is called, the call blocks until a new connection arrives.

Again, let’s look at the arguments that we need to provide to accept connections.
#### sockfd
Like before, we will use the original socket that was created in and here `sockfd` is the file descriptor of the socket. 
#### addr
The argument `addr` is a pointer that refers to a `sockaddr` struct, this needs to be the address of the original socket that we’ve created, and we need the pointer to that struct here.
### addrlen
The `addrlen` is a value result argument, it points to the size of the buffer pointed to by the argument `addr`. Because `accept()` can accept multiple protocol families we need to provide the size of the address that we are using. A pointer is used because: “the caller must initialize it to contain the size (in bytes) of the structure pointed to by `addr`; on return it will contain the actual size of the peer address.” The kernel then knows how much space is available to return the socket address. Upon return from the `accept` function, the value of `addrlen` is set to indicate the number of bytes of data actually stored by the kernel in the socket address structure.

When binding our socket, we’ve already created our `addrlen` variable with the size of the `sockaddr` struct, so we can just pass it to the `accept` function. However, the original variable was an `int`, so we need to typecast it to `socklen_t *` to make it work.
#### return value
It will return a non-negative integer that is a file descriptor for the accepted socket. On error, it will return `-1`, and `errno` will be set.



We have now implemented accepting, now we shall allow our socket to read incoming requests

### Read
We use the `read` and `write` functions.
The function `read` will read upto `count` bytes from the file descriptor `fd` into the buffer `*buf`. 

On success, it returns the number of bytes read and the file position advanced by this number.

On error, we get -1.
#### fd
This argument `fd` needs to be the file descriptor, this is the new socket that was returned by the `accept` function.
#### buf
The `buf` argument needs to be a pointer to the address of the memory buffer that you want the contents of the file descriptor `fd` to be read into as a **temporary storage**.
This buffer must be at least `count` bytes long. In our case we will be creating a buffer that will be an array of the type `char`.
And because the array name is converted to pointer, we can use the variable name of the buffer as the argument.
#### count
We need to provide how many bytes we want to ready from the file descriptor `fd` into the buffer. This depends on how large of a buffer you’re creating. In this example we’ll create an array of 2048 characters.
Here, size_t ----> unsigned integer

#### return value
Here, return value = number of bytes read into the buffer(0 if EOF has been reached)
in case of an error, return value = -1
ssize_t ---> signed integer
 
### Write
Now, we shall add the ability to write a request.
Because we are implementing a webserver, we  will return a simple web page.
Since we need to adhere to the HTTP protocols, we will need to structure our response to these rules.

We will be using the same socket the `accept` function that we’ve just read from. Because this socket is a file descriptor we will, just as with `read`, be able to write to this socket using the `write` function.

The `write` function wil write bytes upto count from the buffer pointed to by `buf`, to the file referenced by the file descriptor `fd`.
On success, it returns the number of bytes written.
#### fd
`fd` is the file descriptor that references the socket we’ve created by calling the `accept` function. This is the also the same file descriptor from which we read the request with the function `read`.

#### buf
The `buf` arguments need to be a pointer to **what we want to write to as a response.** For now we will use a pre-defined string that we will add to the server. But for now we will add the following:

```c
char resp[] = "HTTP/1.0 200 OK\r\n"
"Server: webserver-c\r\n"
"Content-type: text/html\r\n\r\n"
"<html>hello, world</html>\r\n";
```

The above string is formatted with the HTTP protocol.
We start with the request line, followed by headers and end with body.
The escape code `\r\n` is used to separate the different sections of the request.
`\r` stands for carriage return and will set the cursor at the beginning of the line, and `\n` for new line and will move the cursor to a new line.
#### count
The argument `count` is the number of bytes we need write to the file `fd` from buffer `buf`. Because we want to write the complete contents we need to know how many bytes there are in the buffer. We do that by using `strlen()`.
The strlen() function calculates the length of the string pointed to by s, excluding the terminating null byte ('\0').
So, we can provide the `s` argument and we will get the number of characters we provided in the string point to by `s`.
#### return value
The return value of the `write` function will be the number of bytes written to the file and its type is `ssize_t`.
From before we’ve noted that `size_t` was used to represent sizes and counts, this version is the signed version of `size_t`, which means that it can hold values less than zero.
So, in this case a value of less than zero, `-1` is means an error occurred and `errno` will be set appropriately.
When the return value is zero, it indicates that nothing was written. It will not mean that an error occurred when this number is smaller than the number of bytes that were requested.


Our server is now complete!

PS: We also added functionality to read the request headers using `sscanf` and get the client address(`sockn`).

