//
//	Abhinav Thakur
//	compilepeace@gmail.com	
//
//	Description: This header file contains the send_string and recieve_line
//



// 	This function is used as a replacement for recv() since recv() does not tell the accurate
// 	number of data bytes recieved (it includes the "\r\n" - End Of Line(EOL) charecters also) 
//	So where we want to take account of accurate number of bytes recieved we will use this
//	function. This function returns number of bytes recieved (excluding "\r\n").
//
int recieve_line(int socket_fd, unsigned char *buffer)
{
	unsigned char *ptr;
	int CR_flag=0, NL_flag=0;				// Carriag Return (CR) and NewLine(NL) flags
	int recieved_length;		

	
	ptr = buffer;


		// Read byte by byte at address pointed to by 'ptr'
		while (recv(socket_fd, ptr, 1, 0) == 1)
		{
			
			// check the byte first for '\r' ODH carriage return
			if (*ptr == '\r' || CR_flag == 1)
			{	

				// set Carriage Return Flag to 1, found '\r'
				CR_flag = 1;
				

					// if newline feed '\n' is found
					if (*ptr == '\n')
					{	
					 	// Mark the place before '\n' as End of string (terminating by a NULL charecter) 
						*(ptr - 1) = '\0';
						return strlen(buffer);
					}	
				
					// If carriage return '\r' is found but newline feed '\n' is not yet found
					if (CR_flag == 1 && NL_flag == 0)
						++ptr;				
			}
	
			// increment to next index
			else
				++ptr;

		}	


	// EOL sequence not found
	return 0;
}



// This string sends each and every byte from string pointed to by ptr.
// Returns 1 on success 
int send_string(int sockfd, char *ptr)
{
	int bytes_sent=0;
	int bytes_to_send;

	bytes_to_send = strlen(ptr);

		// if there are still bytes left unsent
		while(bytes_to_send > 0)	
		{ 
			bytes_sent = send(sockfd, ptr, bytes_to_send, 0);
			
			if (bytes_sent == 0)
				return 0;			
	
			bytes_to_send -= bytes_sent;
			ptr = ptr + bytes_sent;
		}

	return 1;	
}








