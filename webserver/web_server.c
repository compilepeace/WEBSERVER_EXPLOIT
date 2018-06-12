/*
  
  

  
##########	Abhinav Thakur
##########	compilepeace@gmail.com
  
##########  Program name - web_server.c
##########	Description  - This is webserver program to demonstrate client-server architecture from scratch.
			       Present status: It responds to GET/ HEAD requests only.
			       I am still doing a research on the concepts, so its may still be full of bugs :)
						   


		     STRUCTURES USED

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Structure describing a generic socket address.
// In /usr/include/bits/socket.h  

struct sockaddr
  {
    __SOCKADDR_COMMON (sa_);	// Common data: address family and length.  
    char sa_data[14];			// Address data.  
  };

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

// Structure describing an internet socket address (IP address/ Port # / Domain-AF_INET...)
// In /usr/include/netinet/in.h

struct sockaddr_in
  {
    __SOCKADDR_COMMON (sin_);
    in_port_t sin_port;				// Port number. 
    struct in_addr sin_addr;		// Internet address. 

    // Pad to size of `struct sockaddr' (which is a generic socket address structure).  
    unsigned char sin_zero[sizeof (struct sockaddr) -
			   __SOCKADDR_COMMON_SIZE -
			   sizeof (in_port_t) -
			   sizeof (struct in_addr)];
  };

.......................................................

// Ditto, for IPv6.  
struct sockaddr_in6
  {
    __SOCKADDR_COMMON (sin6_);
    in_port_t sin6_port;		// Transport layer port # 
    uint32_t sin6_flowinfo;		// IPv6 flow information 
    struct in6_addr sin6_addr;	// IPv6 address 
    uint32_t sin6_scope_id;		// IPv6 scope-id 
  };

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

// Types of sockets in /usr/include/bits/socket_type.h

enum __socket_type
{
  SOCK_STREAM = 1,		// Sequenced, reliable, connection-based
				   		// byte streams. 
#define SOCK_STREAM SOCK_STREAM
  SOCK_DGRAM = 2,		// Connectionless, unreliable datagrams
				   		//   of fixed maximum length.  
#define SOCK_DGRAM SOCK_DGRAM
...
};

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Description of data base entry for a single host.  

struct hostent
{
  char *h_name;						// Official name of host.  
  char **h_aliases;					// Alias list.  
  int h_addrtype;					// Host address type.  
  int h_length;						// Length of address.  
  char **h_addr_list;				// List of addresses from name server.  
#ifdef __USE_MISC
# define	h_addr	h_addr_list[0]  // Address, for backward compatibility.
#endif
};
 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^




				FUNCTIONS PROTOTYPES
				
##	/usr/include/sys/socket.h

->	socket(int domain, int type, int protocol)				
->	connect(int fd, struct sockaddr *remote_host, socklen_t addr_length)
->	bind(int fd, struct sockaddr *local_addr, socklen_t addr_length)
->	listen(int fd, int backlog_queue_size)
->	accept(int fd, struct sockaddr *remote_host, socklen_t *addr_length)
->	send(int fd, void *buffer, size_t n, int flags)
->	recv(int fd, void *buffer, size_t n, int flags)

##	/usr/include/netinet/in.h

->	htonl(long value)
->	htons(short value)
->	ntohl(long value)
->	ntohs(short value)		

##	/usr/include/arpa/inet.h

->	inet_aton(char *ascii_addr, struct in_addr *network_addr)
->	inet_ntoa(struct in_addr *network_addr)								

##	/usr/include/netdb.h

->	gethostbyname(char *hostname)			// Returns a pointer to 'hostent' structure
		

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


##	HTTP HEADERS

	
->	Sample HTTP GET request

=>		GET /example HTTP/1.1
		Host: domain.com

	The request must include the 'Host:' headr


->	Sample HTTP Response

=>		HTTP/1.1 200 OK
		Date: Wed Dec 20 13:05:11 PST 2017
		Connection: close
		Content-Length: 41749
		Content-Type: text/html

		<!DOCTYPE html><html><head><title>Lambda School ...

	The end of the header on both Request and Response is marked by a blank line.
	The 'Content-type' gives you the MIME (Multipurpose Internet Mail Extensions) type 
	of content of the 'body'. This tells the web browser how to interpret the file -
	as html, GIF image or anything else.


->	If file not found, a 404 status(error) code	is generated and returned by the server

=>		HTTP/1.1 404 NOT FOUND
		Date: Wed Dec 20 13:05:11 PST 2017
		Connection: close
		Content-Length: 13
		Content-Type: text/plain

		404 Not Found

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

*/		

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "send_recieve.h"


#define PORT 8080


void connection_handler(int, struct sockaddr_in *);
int file_size(int );
void send_get_response(int, char *);
void send_head_response(int );
void file_not_found(int );


int main()
{
	struct sockaddr_in host_address, client_address;	
	int socket_fd, new_socket_fd;
	int sockopt_true=1, test=0;
	socklen_t sockaddr_size;



		// Creating a socket through which the clients will connect
		socket_fd = socket(PF_INET, SOCK_STREAM, 0);
		if (socket_fd == -1)
		{
			perror("socket(): ");
			exit(1);
		}


		
			// Set socket option is set so as to reuse the port even if the port is already in use
			test = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &sockopt_true, sizeof(int));
			if (test == -1)
			{
				perror("setsockopt(): ");
				exit(1);
			}	
			


		// information of host
		host_address.sin_family = AF_INET;				// Address family (host byte order)
		host_address.sin_port = htons(PORT);			// 16-bit short port # (network byte order)
		host_address.sin_addr.s_addr = 0;				// Automatically fills up host 32-bit IP
		memset(&(host_address.sin_zero), '\0', 8);		// i.e. 16-(2-2-4) = 8 Bytes (see struct sockaddr_in)



		// Bind the socket with information of the host
		test = bind(socket_fd, (struct sockaddr *)&host_address, sizeof(struct sockaddr));
		if (test == -1)
		{
			perror("bind(): ");
			exit(-1);
		}

		


			// Set socket to listening state
			test = listen(socket_fd, 10);
			if (test == -1)
			{
				perror("listen(): \n");
				exit(1);
			}
	
			printf("\nListening on Port: %d\n\n", PORT);
		


			// Accept loop
			while(1)
			{
				// Accept client connection and returning client information to new_socket_fd
				sockaddr_size = sizeof(struct sockaddr);
				new_socket_fd = accept(socket_fd, (struct sockaddr *)&client_address, &sockaddr_size);
				if (new_socket_fd == -1)
				{
					perror("accept(): \n");
					printf("Terminating...\n");
					exit(1);
				}

					// Printing client information
					printf("\n");
					printf("Client connected : %s: %d \n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port) ); 	


				// transfer control to 'connection_handler' function which handles client connection
				connection_handler(new_socket_fd, &client_address);
			}

return 0;
}




// This function is handles a particular client by validating the web request made by the
// client (whose file descriptor and structure address is passed to it). After the request
// is validated, it is then identified as GET / HEAD and actions are taken accordingly.
//
void connection_handler(int client_socket_fd, struct sockaddr_in *client_address_ptr)
{
	char request[1000], resource[2000];
	char *dynamic_buffer;
	char *request_token[3];
	int fd, bytes_read;
	int GET_flag = 0, HEAD_flag = 0;



		// welcome message can't be send until headers are recieved by client
		// send_string(client_socket_fd, "\n\n -=-=-=-=-=- Welcome to the d3@d -=-=-=-=-=-\n\r\n");

		
				// User starts making requests
				// Client starts recieving request from the client
				bytes_read = recieve_line(client_socket_fd, request);	
				printf(" Request recieved: %s \n", request);
				printf(" Bytes recieved  : %d \n", bytes_read);



				// If user does not want to make any requests
				if (strcmp(request, "bye") == 0 || strcmp(request, "BYE") == 0)
				{
					printf("Terminating connection..\n");
					printf("Bye.\n");
					exit(1);	
				}


				// This function breaks a 'request' string into tokens via delimiters
				// strtok(char *string, const char *delimiter)
				// see 'strtok.c' in ./../Meanwhile directory.
				request_token[0] = strtok(request, " ");				// request type (eg: GET/HEAD/POST...)
				request_token[1] = strtok(NULL, " ");					// resource 	(eg: /file.txt )
				request_token[2] = strtok(NULL, " ");					// Protocol version (eg: HTTP/1.1 )



				// Identifying the request and set their corresponding flag variables
				if ( strcmp(request_token[0], "GET") == 0 )
					GET_flag = 1;
				else if ( strcmp(request_token[0], "HEAD") == 0 )
					HEAD_flag = 1;



						// if 'GET_flag' is set to 1, it is a "GET" request
						if ( GET_flag == 1 )	
							// passed the client_socket and resource requested from server	
							send_get_response(client_socket_fd, request_token[1]);
				
			
						// if 'HEAD_flag' is set to 1, it is a "HEAD" request
						else if ( HEAD_flag == 1 )
							// passed the client_socket file as argument
							send_head_response(client_socket_fd);
							

}





// This function accepts a file descriptor and returns the size of file in bytes
// 
int file_size(int file_descriptor)
{
	struct stat stat_var;
	
		
			if (fstat(file_descriptor, &stat_var) == -1)
			{
				perror("fstat(): ");
				exit(1);
			}

			else
				return (int)stat_var.st_size;

}





// This function handles the GET request and sends a response to client socket
//
void send_get_response(int client_sockfd, char *resource)
{
	char *dynamic_buffer;
	int fd;
	char path_resource[] = ".";



	// Path of resource should start with "." i.e. current directory from which
	// the webserver program is running, therefore we preceed the resource with
	// a ".". Fro instance, request mentioned "/index.html" as resource, the to
	// consider it relative path we add "." to resouce path --> "./index.html"
	if (strcmp(resource, "/") == 0)
		strncat(resource, "index.html", 10);

	strncat(path_resource, resource, strlen(resource));



			// sending HTTP Headers prior to sending resource files	
			send_string(client_sockfd, "HTTP/1.1 200 OK\r\n");
            send_string(client_sockfd, "Content-Length: 177\r\n");
            send_string(client_sockfd, "Content-type: text/html\r\n");
            send_string(client_sockfd, "Server: d3@d\r\n\r\n");



			// sending resource file
			// opening resource file
			fd = open(path_resource, O_RDONLY, 0);
			if (fd == -1)
			{
				perror("open(): ");
				file_not_found(client_sockfd);
			}
			printf("\n\nOpening file --> %s : %d bytes \n\n", path_resource, file_size(fd));

			
			

			
			// allocating memory to dynamic buffer
			dynamic_buffer = (char *)malloc(file_size(fd));	
			if (dynamic_buffer == NULL)
			{
				perror("send_get_response(): ");
				exit(1);
			}



			// reading the file contents to dynamic buffer
			read(fd, dynamic_buffer, file_size(fd));


			// appending EOL string "\r\n" to the dynamic buffer
			// append 3 charecters such that the last charecter i.e. " "
			// is automatically overwritten with a '\0' terminating the string
			strncat(dynamic_buffer, "\r\n ", 3);


			// Sending dynamic_buffer (file contents) to the client socket
			send_string(client_sockfd, dynamic_buffer);

	
	free(dynamic_buffer);
	close(fd);	

}





// This function handles the HEAD request and sends HEADERS as response to client socket
//
void send_head_response(int client_sockfd)
{
	 // sending HTTP headers 
            send_string(client_sockfd, "HTTP/1.1 200 OK\r\n");
            send_string(client_sockfd, "Content-Length: 177\r\n");
            send_string(client_sockfd, "Content-type: text/html\r\n");
            send_string(client_sockfd, "Server: d3@d\r\n\r\n");

}




// This function sends a 'status_code_404.html' file by default to the client
// if the resource in demand was not identified / found.
//
void file_not_found(int client_socket_fd)
{
	int fd, size_of_file;
	char *dynamic_buffer;



			fd = open("./status_code_404.html", O_RDONLY, 0);		// open file in READ ONLY MODE
			size_of_file = file_size(fd);							// keep size of file				


				// send HTTP Header before sending content. The content and the HEADERS
				// should be separated by a blank line, i.e. why we use 2 EOL strings "\r\n\r\n" 
				send_string(client_socket_fd, "HTTP/1.1 404 NOT FOUND\r\n");
				send_string(client_socket_fd, "Content-Length: 174\r\n");
				send_string(client_socket_fd, "Content-type: text/html\r\n\r\n");
				send_string(client_socket_fd, "Server: d3@d\r\n\r\n");

				
				
				
				//send_string(client_socket_fd, "<!DOCTYPE html><head><title>Error 404</title></head>");
				//send_string(client_socket_fd, "<body><h1>URL NOT FOUND</h1></body></html>\r\n");	
				// Now sending error 404 .html file 
				


					dynamic_buffer = (char *)malloc(size_of_file);		// allocate memory for file to be read	
						if (dynamic_buffer == NULL)						// should be no error allocating memory
						{
							perror("malloc(): ");
							exit(1);
						}
						
					read(fd, dynamic_buffer, size_of_file);			// read file into memory allocated
					strncat(dynamic_buffer, "\r\n ", 3);				// append EOL string "\r\n" before sending
					send_string(client_socket_fd, dynamic_buffer);		// send file to the socket
				

				free(dynamic_buffer);
				close(fd);	

}

